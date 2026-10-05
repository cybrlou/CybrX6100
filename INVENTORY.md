# CybrX6100 Custom Firmware — Change Inventory

**Inventory date:** 2026-10-05 (America/New_York)  
**Build host:** Kali `192.168.60.189` (`kali` / historically `kali`)  
**Trees:** `/home/kali/x6100/AetherX6100Buildroot` + `/home/kali/x6100/x6100_gui`  
**Upstream tips at inventory:** BR `07a8d25` (master), GUI `02f5240` (main)  
**Status:** All Cybr changes are **uncommitted local modifications** (and untracked files) on top of gdyuldin forks. No GitHub push / repo create was done for this inventory.

---

## Upstream baselines

| Component | Upstream | License (on disk / GitHub) | Local path |
|-----------|----------|----------------------------|------------|
| Buildroot image | [gdyuldin/AetherX6100Buildroot](https://github.com/gdyuldin/AetherX6100Buildroot) | **GPL-2.0** (`LICENSE`) | `/home/kali/x6100/AetherX6100Buildroot` |
| LVGL GUI | [gdyuldin/x6100_gui](https://github.com/gdyuldin/x6100_gui) | **LGPL-2.1** (`LICENSE`; sources use `LGPL-2.1-or-later`) | `/home/kali/x6100/x6100_gui` |
| GUI override for builds | Buildroot `local.mk` | — | `/home/kali/x6100/AetherX6100Buildroot/build/local.mk` → `X6100_GUI_OVERRIDE_SRCDIR = /home/kali/x6100/x6100_gui` |

---

## Feature map (known features → sources)

### 1. Custom boot splash (CybrX)

| Path | Role |
|------|------|
| `AetherX6100Buildroot/br2_external/board/X6100/linux/logo.png` | **Modified** Linux boot logo. PNG 800×480 RGBA (~128 KB). Replaces stock logo (was ~47 KB). Used as kernel framebuffer / boot splash. |

### 2. Newer GUI with Wi‑Fi + Bluetooth UI

| Path | Role |
|------|------|
| `br2_external/package/x6100-gui/x6100_gui.mk` | **Modified:** `X6100_GUI_VERSION` bumped from `v0.23.0-rc.3` → commit `02f524068f8ec19031331bab38f06eb212efd47b` (newer gdyuldin tip with Wi‑Fi dialog / BT-related upstream work). |
| `build/local.mk` | **Local-only:** forces Buildroot to build GUI from `/home/kali/x6100/x6100_gui` instead of fetching that git version alone. |
| Upstream GUI files (unchanged by Cybr status) | `src/dialog_wifi.c`, `src/wifi.cpp`, BT scripts such as overlay `usr/bin/bt_start.sh` — Wi‑Fi/BT UI largely comes with the newer GUI pin; Cybr adds SSH toggle beside Wi‑Fi (below). |

### 3. Cybr REST API (`cybr_http.c`) — port 58080, auth `X-Cybr-Token`

| Path | Role |
|------|------|
| `x6100_gui/src/cybr_http.c` | **Untracked (~557 lines).** Tiny HTTP server: bind port **58080**, require header **`X-Cybr-Token: CYBRX6100`** (hardcoded default — sanitize for OSS). Routes for health, apps, actions. Does not touch CAT freq/mode/PTT. |
| `x6100_gui/src/cybr_http.h` | **Untracked.** `cybr_http_init` / `cybr_http_destruct`. |
| `x6100_gui/src/CMakeLists.txt` | **Modified:** adds `cybr_http.c` (and `ssh.cpp`) to build. |
| `x6100_gui/src/main.c` | **Modified:** calls `cybr_http_init()` after CAT LAN; `cybr_http_destruct()` on exit. |
| `x6100_gui/src/main_screen.c` / `.h` | **Modified:** `main_screen_close_app()`, `main_screen_current_app()` for REST app query/close. |

**REST surface (from source):**

- `GET /` — health / version  
- `GET /apps` — current app name  
- `POST /apps/close` — close dialog  
- `POST /apps/{rtty,ft8,swr,gps,recorder,settings,callsign,qth,wifi}`  
- `POST /actions/{mute,nr,nb,step_up,step_down,voice_mode,battery,screenshot}`  

### 4. `wifi.txt` on DATA (`/mnt`) — `ssid=` / `psk=`

| Path | Role |
|------|------|
| `…/rootfs-overlay/usr/bin/cybr_wifi_from_file.sh` | **Untracked.** Parses `/mnt/wifi.txt`, writes NM keyfile `cybr-from-file.nmconnection`, reloads NM via dbus. Never logs PSK. Idempotent via MD5 fingerprint. |
| `…/rootfs-overlay/etc/init.d/S46cybr_wifi` | **Untracked.** Boot hook after NM; runs `cybr_wifi_from_file.sh`. |
| `…/rootfs-overlay/etc/NetworkManager/dispatcher.d/98-cybr-wifi.sh` | **Untracked.** Re-applies wifi.txt when `wlan0` goes up (e.g. after GUI Wi‑Fi power-on). |
| `…/rootfs-overlay/usr/share/cybr/wifi.txt.example` | **Untracked.** Template (`ssid=YourNetworkName` / `psk=YourWifiPassword`). |
| `S01create_data` | Seeds `/mnt/wifi.txt.example` when DATA mounts. |

### 5. BOOT enlarged to 512 MB FAT32

| Path | Role |
|------|------|
| `br2_external/board/X6100/genimage.cfg` | **Modified:** `boot.vfat` `size = 32M` → **`512M`**. Confirmed built image: `images/boot.vfat` is 536870912 bytes. |

### 6. Eva / Cybrtch web branding (WEBMODERN)

| Path | Role |
|------|------|
| `br2_external/package/x6100-webserver/cybrtch.css` | **Untracked.** Dark theme tokens (`#0B100E`, `#39FF14`, …). Hides `.subtitle` (CYBRTCH subtitle removed). Overrides `simple.min.css` header/nav button styling. |
| `…/base.html`, `index.html`, `bands.html`, `digital_modes.html`, `files.html`, `time.html` | **Untracked** package templates; brand **“CybrX 6100”**, link `cybrtch.css`, nav tabs + active-state JS. |
| `x6100_webserver.mk` | **Modified:** `X6100_WEBSERVER_INSTALL_CYBRTCH` post-install hook copies CSS/HTML into site-packages. |
| Overlay copies under `…/rootfs-overlay/usr/lib/python3.11/site-packages/x6100_webserver/` | Same assets for rootfs overlay path. |
| `/home/kali/x6100/eva-pack/` | Staging pack: `apply_on_kali.py`, `rebuild-webmodern.sh`, CSS/HTML copies. |

### 7. DATA create bugfix (`S01create_data`)

| Path | Role |
|------|------|
| `…/rootfs-overlay/etc/init.d/S01create_data` | **Modified.** Old logic treated any FAT32 as “DATA exists” and could skip creating DATA when only BOOT was FAT32. Fix: only treat partition as DATA if `blkid -L DATA` or `mmcblk0p3` **LABEL=DATA**. Also seeds wifi example + `/mnt/ssh.enabled`. |
| `S01create_data.bak.20261005101003` | Backup of pre-fix script (do **not** ship in OSS). |

### 8. SSH OpenSSH default ON + APP toggle

| Path | Role |
|------|------|
| `…/rootfs-overlay/etc/init.d/S50sshd` | **Untracked.** Preference-aware sshd start: `/mnt/ssh.enabled` missing/1 → start; 0/off → skip. Documents default **root / 123**. |
| `…/rootfs-overlay/etc/ssh/sshd_config` | **Modified:** comments; `PermitEmptyPasswords no` (was yes). |
| `…/rootfs-overlay/usr/share/cybr/SSH.txt` | **Untracked.** End-user note on DATA about SSH defaults and toggle. |
| `x6100_gui/src/ssh.cpp`, `ssh.h` | **Untracked.** APP-page toggle; syncs `params.db` `ssh_enabled` ↔ `/mnt/ssh.enabled`; start/stop `S50sshd`. |
| `settings_manager.h` / `cfg_api.*` / `buttons.cpp` | **Modified:** `p_ssh_enabled` (default **true**); SSH button on APP page 3 next to Wi‑Fi. |
| `S01create_data` | Seeds `/mnt/ssh.enabled=1` if missing. |

### 9. MCP stdio wrapper

| Path | Role |
|------|------|
| Box: `/workspace/cybr-mcp/server.py` (copied into proposed `mcp/`) | Python stdio MCP server. Env: `CYBR_RADIO_URL` (default `http://192.168.60.117:58080`), `CYBR_RADIO_TOKEN` (default `CYBRX6100`). Auth header must be **`X-Cybr-Token`** (not Bearer). |
| Tools | `radio_health`, `radio_current_app`, `radio_open_app`, `radio_close_app`, `radio_action` |

---

## Build helper scripts (Kali `/home/kali/x6100/`)

| Script | Purpose |
|--------|---------|
| `rebuild-cybr-rest.sh` | Docker Jammy rebuild: dirclean GUI, verify `cybr_http` in override tree, full `make`. |
| `rebuild-cybrx-batch.sh` | Batch rebuild: webserver + full image (512M BOOT, overlays). |
| `rebuild-ssh.sh` | SSH-focused rebuild. |
| `rebuild-wifi-bt.sh` | Wi‑Fi/BT rebuild helper. |
| `rebuild-webmodern` / `eva-pack/rebuild-webmodern.sh` | Web theme apply + rebuild. |
| `container-build.sh`, `Dockerfile.jammy` | Ubuntu Jammy Docker build environment (cmake pin). |
| `SSH_BUILD_REPORT.txt` | Notes from SSH feature build. |

Built artifact present: `AetherX6100Buildroot/build/images/sdcard.img` (~1.0 GB, mtime 2026-10-05 18:44).

---

## Git status summary (Kali)

### AetherX6100Buildroot (modified)

- `br2_external/board/X6100/genimage.cfg`
- `br2_external/board/X6100/linux/logo.png`
- `br2_external/board/X6100/linux/rootfs-overlay/etc/init.d/S01create_data`
- `br2_external/board/X6100/linux/rootfs-overlay/etc/ssh/sshd_config`
- `br2_external/package/x6100-gui/x6100_gui.mk`
- `br2_external/package/x6100-webserver/x6100_webserver.mk`

### AetherX6100Buildroot (untracked Cybr)

- Overlay: `S46cybr_wifi`, `S50sshd`, `98-cybr-wifi.sh`, `cybr_wifi_from_file.sh`, `usr/share/cybr/*`, webserver static under overlay `usr/lib/...`
- Package: `cybrtch.css`, HTML templates under `x6100-webserver/`
- Backups `*.bak.*` — exclude from public release

### x6100_gui (modified)

- `src/CMakeLists.txt`, `buttons.cpp`, `cfg/cfg_api.cpp`, `cfg/cfg_api.h`, `cfg/settings_manager.h`, `main.c`, `main_screen.c`, `main_screen.h`

### x6100_gui (untracked)

- `src/cybr_http.c`, `src/cybr_http.h`, `src/ssh.cpp`, `src/ssh.h`

---

## Sanitize before public release

| Item | Where | Action |
|------|-------|--------|
| REST token default `CYBRX6100` | `cybr_http.c`, MCP `server.py` | Change to env/config or obvious placeholder; document as **CHANGE ME** |
| SSH `root` / `123` | `S50sshd`, `SSH.txt`, docs | Keep documented as **dev default — CHANGE ME**; prefer forced first-boot password note |
| Private Wi‑Fi SSIDs/PSKs | Any real `wifi.txt` on radios / SD cards | **Do not** commit; only ship `wifi.txt.example` placeholders |
| Lou / operator callsign | `params.db` on device DATA (not in these source trees) | Do not include device `params.db` or logs in release |
| Lab IPs (`192.168.60.x`), Kali password | MCP defaults, docs | Use placeholders in public docs |
| Backup files `*.bak.*` | overlay / eva-pack | Omit |
| Branding | CybrX / Cybrtch / Eva | Lou decides: keep CybrX vs generic “community X6100 extras” |

**Secrets scan note:** No private SSIDs found in Cybr source trees; `wifi.txt.example` uses placeholders. Callsign hits are upstream GUI APIs/tests only (e.g. test values `R1XXX`), not Lou’s callsign.
