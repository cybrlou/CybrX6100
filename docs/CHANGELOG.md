# Changelog (end-user draft)

## CybrX6100 V.01 (unreleased / lab)

### Added

- Custom **CybrX** Linux boot splash (`logo.png`, 800×480).
- **REST control API** on TCP **58080** with `X-Cybr-Token` auth: health, open/close GUI apps, common UI actions.
- **MCP stdio tools**: `radio_health`, `radio_current_app`, `radio_open_app`, `radio_close_app`, `radio_action`.
- **Wi‑Fi from file**: put `ssid=` / `psk=` in `/mnt/wifi.txt` on the DATA partition; applied via NetworkManager at boot and when wlan0 comes up.
- **SSH**: OpenSSH enabled by default; APP-page **SSH** toggle next to Wi‑Fi; preference stored in `params.db` and `/mnt/ssh.enabled`.
- **Web UI** dark CybrX theme (`cybrtch.css`) with fixed nav vs simple.css; CYBRTCH subtitle removed.
- Build pin to newer **gdyuldin/x6100_gui** (Wi‑Fi UI and related upstream features).

### Changed

- **BOOT** partition enlarged from 32 MB to **512 MB** FAT32.
- `sshd_config`: disallow empty passwords.

### Fixed

- **DATA partition creation**: no longer skips creating DATA just because BOOT is already FAT32; only LABEL=`DATA` / p3 counts.

### Security notes for operators

- Default REST token and SSH password are lab values — change before any shared network use (see SECURITY.md).
