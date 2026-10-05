# Cybr REST API

Implemented in `x6100_gui` as `cybr_http.c` (started next to LAN CAT).

## Connection

| Item | Dev default | Notes |
|------|-------------|-------|
| URL | `http://<radio-ip>:58080` | Example lab IP was `192.168.60.117` |
| Auth header | `X-Cybr-Token: <token>` | **Not** Bearer |
| Default token | `CYBRX6100` | **CHANGE ME** before public/network use |

## Endpoints

### Health

```http
GET /
X-Cybr-Token: CYBRX6100
```

### Current app

```http
GET /apps
```

Returns current GUI app name (`none`, `ft8`, `rtty`, …).

### Open app

```http
POST /apps/{app}
```

Allowed apps: `rtty`, `ft8`, `swr`, `gps`, `recorder`, `settings`, `callsign`, `qth`, `wifi`.

### Close app

```http
POST /apps/close
```

### Actions

```http
POST /actions/{name}
```

Allowed: `mute`, `nr`, `nb`, `step_up`, `step_down`, `voice_mode`, `battery`, `screenshot`.

## curl examples

```bash
export R=http://192.0.2.10:58080
export T=CHANGE_ME_TOKEN

curl -s -H "X-Cybr-Token: $T" "$R/"
curl -s -H "X-Cybr-Token: $T" "$R/apps"
curl -s -X POST -H "X-Cybr-Token: $T" "$R/apps/ft8"
curl -s -X POST -H "X-Cybr-Token: $T" "$R/apps/close"
curl -s -X POST -H "X-Cybr-Token: $T" "$R/actions/mute"
```

## Scope / non-goals

Does **not** change CAT frequency, mode, filter, power, PTT, or meters. For RF control use existing CAT/LAN paths from upstream GUI.
