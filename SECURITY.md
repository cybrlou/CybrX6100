# Security Policy — CybrX6100

## Defaults you MUST change before exposing the radio on a network

CybrX6100 development images ship with convenient lab defaults. Treat them as **insecure**.

| Default | Value (dev) | Risk | Mitigate |
|---------|-------------|------|----------|
| REST API token | `CYBRX6100` | Anyone on LAN can open apps / toggle mute / screenshot | Set a strong token; rebuild or make token file-based before OSS release |
| REST bind | TCP **58080** (all interfaces as implemented) | LAN control plane | Firewall; token; consider bind-to-interface in a future change |
| SSH user/password | `root` / `123` | Full root shell | Run `passwd root` immediately; prefer SSH keys in `/root/.ssh/authorized_keys` |
| SSH default state | **ON** at boot (`/mnt/ssh.enabled` seeded to `1`) | Attack surface if ETH/Wi‑Fi reachable | APP toggle **SSH Off**, or write `0` to `/mnt/ssh.enabled` and reboot |
| Empty SSH passwords | Disabled (`PermitEmptyPasswords no`) | — | Keep disabled |
| Wi‑Fi credentials | `/mnt/wifi.txt` on DATA | PSK on FAT partition readable from PC | Protect the SD card; do not publish `wifi.txt` |

## Reporting

This is a **private** repository. Report issues privately to the owner (`cybrlou`). Do not open public issues that include working tokens, PSKs, or callsign/QTH private data.

## Auth header note (MCP / REST)

Clients must send:

```http
X-Cybr-Token: <your-token>
```

`Authorization: Bearer …` is **not** accepted by `cybr_http.c`.

## Scope of REST

The Cybr HTTP server is intentionally limited: GUI app open/close and a small set of UI actions. It does **not** implement CAT frequency/mode/PTT. Do not assume it is a full security boundary for RF control.

## Where the defaults live (CHANGE ME)

| Default | File in this repo |
|---------|-------------------|
| REST token `CYBRX6100` | `gui-src/cybr_http.c` / `patches/gui/0002-*.patch` (`#define CYBR_TOKEN`); `mcp/server.py` (`CYBR_RADIO_TOKEN` fallback) |
| SSH `root` / `123` | Buildroot `BR2_TARGET_GENERIC_ROOT_PASSWD` (upstream defconfig); documented in `overlay/.../etc/init.d/S50sshd`, `usr/share/cybr/SSH.txt` |
| SSH default ON | `overlay/.../etc/init.d/S01create_data` (seeds `/mnt/ssh.enabled=1`); GUI `p_ssh_enabled` default `true` (`patches/gui/0003-*.patch`) |

Never commit a real `wifi.txt`, device `params.db`, SD-card images, or build logs (see `.gitignore`).
