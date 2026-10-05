/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * CybrX6100 V.01 HTTP control.
 *
 * POSIX sockets only. UI work is posted with scheduler_put_noargs(), which
 * the main loop runs from scheduler_work() on the LVGL thread, next to
 * event_obj_check() / lv_timer_handler(). Handlers call main_screen_start_app(),
 * main_screen_action(), or main_screen_close_app() only from that thread.
 *
 * Frequency, mode, filter, power, PTT, and meters stay on the existing CAT path.
 */
#include "cybr_http.h"

#include "main_screen.h"
#include "scheduler.h"

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define CYBR_VERSION     "CybrX6100 V.01"
#define CYBR_TOKEN       "CYBRX6100"
#define CYBR_HTTP_PORT   58080
#define CYBR_REQ_MAX     8192
#define CYBR_QUEUE_MAX   16

typedef enum {
    CYBR_JOB_ACTION = 1,
    CYBR_JOB_APP,
    CYBR_JOB_CLOSE,
    CYBR_JOB_QUERY
} cybr_job_kind_t;

typedef enum {
    CYBR_ST_QUEUED = 0,
    CYBR_ST_RUNNING,
    CYBR_ST_DONE
} cybr_job_state_t;

typedef struct cybr_job {
    struct cybr_job   *next;
    cybr_job_kind_t    kind;
    press_action_t     action;
    cybr_job_state_t   state;
    int                abandoned;
    char               app_name[24];
    pthread_mutex_t    mu;
    pthread_cond_t     cv;
} cybr_job_t;

static int              listen_fd = -1;
static pthread_t        server_thread;
static int              thread_started = 0;
static volatile int     keep_running = 0;
static char             bind_desc[64] = "0.0.0.0";

static pthread_mutex_t  qmu = PTHREAD_MUTEX_INITIALIZER;
static cybr_job_t      *qhead = NULL;
static cybr_job_t      *qtail = NULL;
static int              qlen = 0;

static void cybr_drain(void *unused);

static void job_enqueue(cybr_job_t *job) {
    job->next = NULL;
    job->state = CYBR_ST_QUEUED;
    if (qtail) {
        qtail->next = job;
    } else {
        qhead = job;
    }
    qtail = job;
    qlen++;
}

static void job_unlink(cybr_job_t *job) {
    cybr_job_t **pp = &qhead;
    cybr_job_t *prev = NULL;
    while (*pp) {
        if (*pp == job) {
            *pp = job->next;
            if (qtail == job) {
                qtail = prev;
            }
            qlen--;
            job->next = NULL;
            return;
        }
        prev = *pp;
        pp = &(*pp)->next;
    }
}

static void job_free(cybr_job_t *job) {
    pthread_cond_destroy(&job->cv);
    pthread_mutex_destroy(&job->mu);
    free(job);
}

static cybr_job_t *job_new(cybr_job_kind_t kind, press_action_t action) {
    cybr_job_t *job = calloc(1, sizeof(*job));
    if (!job) {
        return NULL;
    }
    job->kind = kind;
    job->action = action;
    pthread_mutex_init(&job->mu, NULL);
    pthread_cond_init(&job->cv, NULL);
    snprintf(job->app_name, sizeof(job->app_name), "unknown");
    return job;
}

/* Runs on the LVGL thread via scheduler_work(). */
static void cybr_drain(void *unused) {
    (void)unused;
    for (;;) {
        pthread_mutex_lock(&qmu);
        cybr_job_t *job = qhead;
        if (job) {
            qhead = job->next;
            if (!qhead) {
                qtail = NULL;
            }
            qlen--;
            job->next = NULL;
            pthread_mutex_lock(&job->mu);
            job->state = CYBR_ST_RUNNING;
            pthread_mutex_unlock(&job->mu);
        }
        pthread_mutex_unlock(&qmu);
        if (!job) {
            return;
        }

        if (job->kind == CYBR_JOB_APP) {
            main_screen_start_app(job->action);
        } else if (job->kind == CYBR_JOB_ACTION) {
            main_screen_action(job->action);
        } else if (job->kind == CYBR_JOB_CLOSE) {
            main_screen_close_app();
        } else if (job->kind == CYBR_JOB_QUERY) {
            const char *name = main_screen_current_app();
            if (!name) {
                name = "unknown";
            }
            snprintf(job->app_name, sizeof(job->app_name), "%s", name);
        }

        pthread_mutex_lock(&job->mu);
        job->state = CYBR_ST_DONE;
        int abandoned = job->abandoned;
        pthread_cond_signal(&job->cv);
        pthread_mutex_unlock(&job->mu);
        if (abandoned) {
            job_free(job);
        }
    }
}

/*
 * Block the HTTP thread until the LVGL thread finishes the job.
 * Never waits while holding qmu, otherwise cybr_drain cannot pop the job.
 * Returns 0 and copies app_name on success. Frees job unless drain still
 * owns a timed-out RUNNING job (that path sets abandoned and drain frees it).
 */
static int cybr_submit(cybr_job_t *job, char *app_out, size_t app_out_len) {
    pthread_mutex_lock(&qmu);
    if (qlen >= CYBR_QUEUE_MAX) {
        pthread_mutex_unlock(&qmu);
        job_free(job);
        return -1;
    }
    job_enqueue(job);
    pthread_mutex_unlock(&qmu);

    scheduler_put_noargs(cybr_drain);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;

    pthread_mutex_lock(&job->mu);
    while (job->state != CYBR_ST_DONE) {
        int rc = pthread_cond_timedwait(&job->cv, &job->mu, &ts);
        if (rc == ETIMEDOUT && job->state != CYBR_ST_DONE) {
            break;
        }
    }
    if (job->state == CYBR_ST_DONE) {
        if (app_out && app_out_len) {
            snprintf(app_out, app_out_len, "%s", job->app_name);
        }
        pthread_mutex_unlock(&job->mu);
        job_free(job);
        return 0;
    }
    pthread_mutex_unlock(&job->mu);

    pthread_mutex_lock(&qmu);
    pthread_mutex_lock(&job->mu);
    if (job->state == CYBR_ST_DONE) {
        if (app_out && app_out_len) {
            snprintf(app_out, app_out_len, "%s", job->app_name);
        }
        pthread_mutex_unlock(&job->mu);
        pthread_mutex_unlock(&qmu);
        job_free(job);
        return 0;
    }
    if (job->state == CYBR_ST_QUEUED) {
        job_unlink(job);
        pthread_mutex_unlock(&job->mu);
        pthread_mutex_unlock(&qmu);
        job_free(job);
        return -1;
    }
    job->abandoned = 1;
    pthread_mutex_unlock(&job->mu);
    pthread_mutex_unlock(&qmu);
    return -1;
}

static int send_all(int fd, const char *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, buf + off, len - off, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

static void respond(int fd, int code, const char *reason, const char *json) {
    char hdr[256];
    size_t blen = json ? strlen(json) : 0;
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.0 %d %s\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     code, reason, blen);
    if (n < 0 || (size_t)n >= sizeof(hdr)) {
        return;
    }
    if (send_all(fd, hdr, (size_t)n) != 0) {
        return;
    }
    if (blen) {
        send_all(fd, json, blen);
    }
}

static int header_token_ok(const char *req) {
    const char *p = strstr(req, "\r\n");
    if (!p) {
        return 0;
    }
    p += 2;
    const char *end = strstr(p, "\r\n\r\n");
    if (!end) {
        return 0;
    }
    while (p < end) {
        const char *nl = strstr(p, "\r\n");
        if (!nl || nl > end) {
            break;
        }
        size_t len = (size_t)(nl - p);
        if (len > 12 && p[12] == ':' && strncasecmp(p, "X-Cybr-Token", 12) == 0) {
            const char *v = p + 13;
            while (v < nl && (*v == ' ' || *v == '\t')) {
                v++;
            }
            size_t vlen = (size_t)(nl - v);
            while (vlen > 0 && (v[vlen - 1] == ' ' || v[vlen - 1] == '\t')) {
                vlen--;
            }
            if (vlen == strlen(CYBR_TOKEN) && memcmp(v, CYBR_TOKEN, vlen) == 0) {
                return 1;
            }
            return 0;
        }
        p = nl + 2;
    }
    return 0;
}

static int read_request(int fd, char *buf, size_t buflen) {
    size_t used = 0;
    buf[0] = '\0';
    while (used + 1 < buflen) {
        ssize_t n = recv(fd, buf + used, buflen - 1 - used, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            break;
        }
        used += (size_t)n;
        buf[used] = '\0';
        if (strstr(buf, "\r\n\r\n")) {
            return (int)used;
        }
    }
    return strstr(buf, "\r\n\r\n") ? (int)used : -1;
}

static int app_name_safe(const char *s) {
    if (!s || !s[0] || strlen(s) > 16) {
        return 0;
    }
    for (const char *p = s; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_')) {
            return 0;
        }
    }
    return 1;
}

static void handle_client(int fd) {
    char buf[CYBR_REQ_MAX];
    if (read_request(fd, buf, sizeof(buf)) < 0) {
        respond(fd, 400, "Bad Request", "{\"error\":\"bad_request\"}\n");
        return;
    }

    char method[8];
    char path[128];
    if (sscanf(buf, "%7s %127s", method, path) != 2) {
        respond(fd, 400, "Bad Request", "{\"error\":\"bad_request\"}\n");
        return;
    }
    char *qmark = strchr(path, '?');
    if (qmark) {
        *qmark = '\0';
    }

    if (!header_token_ok(buf)) {
        respond(fd, 401, "Unauthorized", "{\"error\":\"unauthorized\"}\n");
        return;
    }

    struct {
        const char     *method;
        const char     *path;
        cybr_job_kind_t kind;
        press_action_t  action;
    } table[] = {
        {"GET",  "/apps",              CYBR_JOB_QUERY,  ACTION_NONE},
        {"POST", "/apps/close",        CYBR_JOB_CLOSE,  ACTION_NONE},
        {"POST", "/apps/rtty",         CYBR_JOB_APP,    ACTION_APP_RTTY},
        {"POST", "/apps/ft8",          CYBR_JOB_APP,    ACTION_APP_FT8},
        {"POST", "/apps/swr",          CYBR_JOB_APP,    ACTION_APP_SWRSCAN},
        {"POST", "/apps/gps",          CYBR_JOB_APP,    ACTION_APP_GPS},
        {"POST", "/apps/recorder",     CYBR_JOB_APP,    ACTION_APP_RECORDER},
        {"POST", "/apps/settings",     CYBR_JOB_APP,    ACTION_APP_SETTINGS},
        {"POST", "/apps/callsign",     CYBR_JOB_APP,    ACTION_APP_CALLSIGN},
        {"POST", "/apps/qth",          CYBR_JOB_APP,    ACTION_APP_QTH},
        {"POST", "/apps/wifi",         CYBR_JOB_APP,    ACTION_APP_WIFI},
        {"POST", "/actions/mute",      CYBR_JOB_ACTION, ACTION_MUTE},
        {"POST", "/actions/nr",        CYBR_JOB_ACTION, ACTION_NR_TOGGLE},
        {"POST", "/actions/nb",        CYBR_JOB_ACTION, ACTION_NB_TOGGLE},
        {"POST", "/actions/step_up",   CYBR_JOB_ACTION, ACTION_STEP_UP},
        {"POST", "/actions/step_down", CYBR_JOB_ACTION, ACTION_STEP_DOWN},
        {"POST", "/actions/voice_mode", CYBR_JOB_ACTION, ACTION_VOICE_MODE},
        {"POST", "/actions/battery",   CYBR_JOB_ACTION, ACTION_BAT_INFO},
        {"POST", "/actions/screenshot", CYBR_JOB_ACTION, ACTION_SCREENSHOT},
    };

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        respond(fd, 200, "OK", "{\"version\":\"" CYBR_VERSION "\"}\n");
        return;
    }

    cybr_job_kind_t kind = CYBR_JOB_ACTION;
    press_action_t action = ACTION_NONE;
    int found = 0;
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (strcmp(method, table[i].method) == 0 && strcmp(path, table[i].path) == 0) {
            kind = table[i].kind;
            action = table[i].action;
            found = 1;
            break;
        }
    }
    if (!found) {
        respond(fd, 404, "Not Found", "{\"error\":\"not_found\"}\n");
        return;
    }

    cybr_job_t *job = job_new(kind, action);
    if (!job) {
        respond(fd, 500, "Internal Server Error", "{\"error\":\"nomem\"}\n");
        return;
    }

    char app_name[24];
    app_name[0] = '\0';
    if (cybr_submit(job, app_name, sizeof(app_name)) != 0) {
        respond(fd, 503, "Service Unavailable",
                "{\"ok\":false,\"error\":\"lvgl_queue_timeout\"}\n");
        return;
    }

    if (kind == CYBR_JOB_QUERY) {
        if (!app_name_safe(app_name)) {
            snprintf(app_name, sizeof(app_name), "unknown");
        }
        char body[64];
        snprintf(body, sizeof(body), "{\"app\":\"%s\"}\n", app_name);
        respond(fd, 200, "OK", body);
        return;
    }
    respond(fd, 200, "OK", "{\"ok\":true}\n");
}

static void *server_main(void *arg) {
    (void)arg;
    while (keep_running) {
        struct pollfd pfd;
        pfd.fd = listen_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        int pr = poll(&pfd, 1, 500);
        if (!keep_running) {
            break;
        }
        if (pr <= 0) {
            continue;
        }
        int cfd = accept(listen_fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            break;
        }
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(cfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        handle_client(cfd);
        close(cfd);
    }
    return NULL;
}

static int open_listener(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CYBR_HTTP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    snprintf(bind_desc, sizeof(bind_desc), "0.0.0.0");

    struct ifaddrs *ifs = NULL;
    if (getifaddrs(&ifs) == 0) {
        for (struct ifaddrs *ifa = ifs; ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) {
                continue;
            }
            if (strcmp(ifa->ifa_name, "wlan0") != 0) {
                continue;
            }
            struct sockaddr_in *in = (struct sockaddr_in *)ifa->ifa_addr;
            if (in->sin_addr.s_addr == htonl(INADDR_ANY) ||
                in->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
                continue;
            }
            addr.sin_addr = in->sin_addr;
            inet_ntop(AF_INET, &in->sin_addr, bind_desc, sizeof(bind_desc));
            break;
        }
        freeifaddrs(ifs);
    }

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        snprintf(bind_desc, sizeof(bind_desc), "0.0.0.0");
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            close(fd);
            return -1;
        }
    }
    if (listen(fd, 2) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int cybr_http_init(void) {
    if (listen_fd >= 0) {
        return 0;
    }
    listen_fd = open_listener();
    if (listen_fd < 0) {
        LV_LOG_ERROR("CybrX6100 HTTP listen failed");
        return -1;
    }
    keep_running = 1;
    if (pthread_create(&server_thread, NULL, server_main, NULL) != 0) {
        close(listen_fd);
        listen_fd = -1;
        keep_running = 0;
        LV_LOG_ERROR("CybrX6100 HTTP thread failed");
        return -1;
    }
    thread_started = 1;
    LV_LOG_USER("CybrX6100 V.01 HTTP on %s:%d", bind_desc, CYBR_HTTP_PORT);
    return 0;
}

void cybr_http_destruct(void) {
    keep_running = 0;
    if (listen_fd >= 0) {
        shutdown(listen_fd, SHUT_RDWR);
        close(listen_fd);
        listen_fd = -1;
    }
    if (thread_started) {
        pthread_join(server_thread, NULL);
        thread_started = 0;
    }
}
