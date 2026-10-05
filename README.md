# CybrX6100

Private overlay + patch set (**CybrX** branding) for the Xiegu X6100, on top of
[gdyuldin/AetherX6100Buildroot](https://github.com/gdyuldin/AetherX6100Buildroot) @ `07a8d25` and
[gdyuldin/x6100_gui](https://github.com/gdyuldin/x6100_gui) @ `02f5240`.

This repo does **not** fork the upstreams; it carries only the CybrX deltas:
a Buildroot `br2_external` overlay, Buildroot diffs, a 3-patch GUI series, and an MCP client.

> Private repo `cybrlou/CybrX6100`. Lab-default credentials are documented in [SECURITY.md](SECURITY.md) — **CHANGE ME** before network exposure.

## Features

1. CybrX boot splash (`overlay/board/X6100/linux/logo.png`, 800×480)
2. Newer GUI pin (Wi‑Fi / BT UI from upstream `02f5240`) + CybrX SSH toggle on the APP page
3. REST API on `:58080`, auth header **`X-Cybr-Token`** ([docs/REST_API.md](docs/REST_API.md))
4. `wifi.txt` on the DATA partition → NetworkManager ([docs/WIFI.md](docs/WIFI.md))
5. BOOT partition enlarged to 512 MB FAT32 ([docs/PARTITIONS.md](docs/PARTITIONS.md))
6. CybrX web theme for the x6100 webserver (`cybrtch.css`) ([docs/WEB_UI.md](docs/WEB_UI.md))
7. DATA-partition creation fix (`S01create_data`)
8. OpenSSH default ON + APP toggle (`/mnt/ssh.enabled`) ([docs/SSH.md](docs/SSH.md))
9. MCP stdio wrapper for the REST API ([mcp/](mcp/), [docs/MCP.md](docs/MCP.md))

## Quick start

```bash
mkdir -p ~/x6100 && cd ~/x6100
git clone https://github.com/gdyuldin/AetherX6100Buildroot && git -C AetherX6100Buildroot checkout 07a8d25
git clone https://github.com/gdyuldin/x6100_gui && git -C x6100_gui checkout 02f5240
(cd AetherX6100Buildroot && git submodule update --init --recursive && ./br_config.sh)
(cd x6100_gui && git submodule update --init --recursive)
git clone git@github.com:cybrlou/CybrX6100.git
./CybrX6100/scripts/apply-overlay.sh --br AetherX6100Buildroot --gui x6100_gui
cp CybrX6100/scripts/rebuild-docker.sh.example CybrX6100/scripts/rebuild-docker.sh   # edit WORK, then run
```

Details: [docs/BUILD.md](docs/BUILD.md) · flash a **spare** SD card only: [docs/FLASH.md](docs/FLASH.md).

## Layout

| Path | What |
|------|------|
| `overlay/` | Full files, mirrors `br2_external/` (board init scripts, sshd_config, genimage.cfg, logo, webserver theme, package `.mk`s) |
| `patches/buildroot/` | `git apply` diffs vs AetherX6100Buildroot `07a8d25` for every *modified* upstream file |
| `patches/gui/` | `git am` series vs x6100_gui `02f5240` (main_screen helpers → cybr_http → ssh toggle) |
| `gui-src/` | Convenience copies of the new GUI files (`cybr_http.c/h`, `ssh.cpp/h`) |
| `mcp/` | MCP stdio server (`X-Cybr-Token`), MIT |
| `scripts/` | `apply-overlay.sh`, `local.mk.example`, `rebuild-docker.sh.example`, `Dockerfile.jammy` |
| `examples/` | `wifi.txt.example` |
| `docs/` | Build, flash, partitions, Wi‑Fi, SSH, REST, MCP, web UI, changelog |

See also [INVENTORY.md](INVENTORY.md) (maintainer path map) and [REPO_LAYOUT.md](REPO_LAYOUT.md).

## License

Multi-license — see [NOTICE](NOTICE).

- Buildroot overlay, init scripts, genimage/package changes, scripts: **GPL-2.0** ([LICENSE](LICENSE)), matching AetherX6100Buildroot
- GUI patches / `gui-src/`: **LGPL-2.1-or-later** ([LICENSES/LGPL-2.1.txt](LICENSES/LGPL-2.1.txt)), matching x6100_gui
- `mcp/`: **MIT** ([mcp/LICENSE](mcp/LICENSE))


## Premade image (non-technical)

1. Download **CybrX6100-sdcard-v0.1.0.zip** from [Releases](https://github.com/cybrlou/CybrX6100/releases).
2. Unzip to get the .img file.
3. Use [Balena Etcher](https://etcher.balena.io/) (or Raspberry Pi Imager) to write the image to a **spare** microSD card � do not overwrite the card currently in your radio until you have a backup.
4. Insert the card, power on, connect Wi-Fi (or Ethernet), then open http://<radio-ip>/ in a browser.
5. **Immediately change** the SSH password (oot / 123) and treat the REST token CYBRX6100 as a lab default � see [SECURITY.md](SECURITY.md).

Full flash notes: [docs/FLASH.md](docs/FLASH.md).
