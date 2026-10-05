/*
 * SSH_START
 * CybrX6100 SSH preference + OpenSSH control.
 * Persists to params.db (ssh_enabled) and /mnt/ssh.enabled for boot init.
 * SSH_END
 */
#include "ssh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cfg/cfg_api.h"

extern "C" {
#include "msg.h"
}

#define SSH_PREF_PATH "/mnt/ssh.enabled"
#define SSH_INIT_SCRIPT "/etc/init.d/S50sshd"

static void write_pref_file(bool enabled) {
    FILE *f = fopen(SSH_PREF_PATH, "w");
    if (!f) {
        return;
    }
    fputs(enabled ? "1\n" : "0\n", f);
    fclose(f);
}

static void apply_service(bool enabled) {
    if (access(SSH_INIT_SCRIPT, X_OK) != 0) {
        return;
    }
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "%s %s >/dev/null 2>&1", SSH_INIT_SCRIPT,
             enabled ? "start" : "stop");
    system(cmd);
}

bool ssh_is_enabled(void) {
    return param_i_get(cfg.network.ssh_enabled()) != 0;
}

void ssh_setup(void) {
    bool on = ssh_is_enabled();
    write_pref_file(on);
    apply_service(on);
}

void ssh_toggle_cb(button_data_t *btn_data) {
    (void)btn_data;
    bool on = !ssh_is_enabled();
    param_i_set(cfg.network.ssh_enabled(), on ? 1 : 0);
    write_pref_file(on);
    apply_service(on);
    msg_update_text_fmt("SSH %s", on ? "On" : "Off");
}

const char *ssh_label_getter(void) {
    static char buf[24];
    snprintf(buf, sizeof(buf), "SSH:\n%s", ssh_is_enabled() ? "On" : "Off");
    return buf;
}
