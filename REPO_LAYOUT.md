# Repository layout — CybrX6100 (private: `cybrlou/CybrX6100`)

Repo: **`cybrlou/CybrX6100`** (private). Strategy 1 below (overlay + patches) was chosen.

> **As built (differs slightly from the original sketch below):** `overlay/` holds **full files only** (mirror of `br2_external/`, rsync-able). All diffs vs upstream `07a8d25` live in `patches/buildroot/` (`genimage.cfg`, `logo.png` binary, `S01create_data`, `sshd_config`, `x6100_gui.mk`, `x6100_webserver.mk`). GUI patch series is ordered `0001-main_screen…`, `0002-cybr_http…`, `0003-ssh…` (helpers first so each step is self-consistent). Extra: `LICENSES/`, `scripts/Dockerfile.jammy`, overlay copies of the web theme under `rootfs-overlay/usr/lib/python3.11/site-packages/x6100_webserver/`.

This layout packages **Cybr deltas** as a Buildroot external overlay + GUI patches + MCP client, rather than republishing full upstream trees.

```
CybrX6100/                          # repo root
├── README.md                       # quick start, feature list, links to upstream
├── LICENSE                         # see license recommendation below
├── NOTICE                          # upstream attribution (gdyuldin, AetherRadio, R1CBU/strijar)
├── SECURITY.md                     # change default token/password before exposing network
├── CHANGELOG.md                    # end-user feature history (symlink or copy of docs/CHANGELOG.md)
├── INVENTORY.md                    # maintainer-oriented path map (from this inventory)
├── REPO_LAYOUT.md                  # this file
│
├── docs/
│   ├── BUILD.md                    # clone upstream + apply overlay + Docker/Jammy notes
│   ├── FLASH.md                    # spare SD card flash (balenaEtcher/dd); no eMMC wipe guidance
│   ├── WIFI.md                     # wifi.txt on DATA
│   ├── REST_API.md                 # port 58080, X-Cybr-Token, routes
│   ├── MCP.md                      # stdio wrapper env + tools
│   ├── SSH.md                      # OpenSSH default ON, APP toggle, CHANGE ME password
│   ├── PARTITIONS.md               # BOOT 512M / rootfs / DATA
│   ├── WEB_UI.md                   # CybrX Eva theme / cybrtch.css
│   └── CHANGELOG.md
│
├── overlay/                        # dropped into AetherX6100Buildroot br2_external (or BR2_EXTERNAL)
│   ├── board/X6100/
│   │   ├── genimage.cfg.patch      # or full file: BOOT 512M
│   │   ├── linux/logo.png          # CybrX splash (if branding kept)
│   │   └── linux/rootfs-overlay/
│   │       ├── etc/init.d/S01create_data
│   │       ├── etc/init.d/S46cybr_wifi
│   │       ├── etc/init.d/S50sshd
│   │       ├── etc/NetworkManager/dispatcher.d/98-cybr-wifi.sh
│   │       ├── etc/ssh/sshd_config
│   │       ├── usr/bin/cybr_wifi_from_file.sh
│   │       └── usr/share/cybr/
│   │           ├── wifi.txt.example
│   │           └── SSH.txt
│   └── package/
│       ├── x6100-gui/x6100_gui.mk.patch   # pin GUI commit / document OVERRIDE
│       └── x6100-webserver/
│           ├── x6100_webserver.mk.patch   # INSTALL_CYBRTCH hook
│           ├── cybrtch.css
│           ├── base.html
│           ├── index.html
│           ├── bands.html
│           ├── digital_modes.html
│           ├── files.html
│           └── time.html
│
├── patches/gui/                    # quilt/git-format patches against gdyuldin/x6100_gui @ 02f5240
│   ├── 0001-cybr-http-rest-server.patch
│   ├── 0002-ssh-app-toggle.patch
│   └── 0003-main-screen-app-helpers.patch
│   # OR ship a fork branch; patches preferred for small reviewable OSS drop
│
├── gui-src/                        # optional: full cybr_http.c/h + ssh.cpp/h for easy apply
│   ├── cybr_http.c
│   ├── cybr_http.h
│   ├── ssh.cpp
│   └── ssh.h
│
├── mcp/
│   ├── README.md                   # point to docs/MCP.md
│   ├── requirements.txt            # optional httpx
│   └── server.py                   # stdio MCP (sanitize defaults)
│
├── scripts/
│   ├── apply-overlay.sh            # copy overlay into a local AetherX6100Buildroot checkout
│   ├── local.mk.example            # X6100_GUI_OVERRIDE_SRCDIR = …
│   └── rebuild-docker.sh.example   # Jammy container pattern from Kali helpers
│
└── examples/
    └── wifi.txt.example
```

## Distribution strategies (Lou chooses)

1. **Overlay + patches repo (recommended for first public drop)**  
   Smallest surface; clear what Cybr owns; consumers clone gdyuldin trees and apply.

2. **Public forks of both upstreams with Cybr commits**  
   Easier for end users (`git clone` + build) but more maintenance / sync work.

3. **Hybrid:** fork GUI for `cybr_http`/`ssh`, keep Buildroot changes as overlay in this meta-repo.

## License recommendation

| Layer | Match upstream | Recommendation |
|-------|----------------|----------------|
| Buildroot overlay / init / genimage / webserver package hook | AetherX6100Buildroot **GPL-2.0** | **GPL-2.0** (or GPL-2.0-or-later if Lou prefers “or later”) |
| GUI C/C++ additions (`cybr_http`, `ssh`, button/cfg hooks) | x6100_gui **LGPL-2.1-or-later** | **LGPL-2.1-or-later** on those files |
| MCP Python wrapper | New code | **Apache-2.0** or **MIT** (compatible; keep separate notice) |
| Docs / examples | — | CC0 or same as repo |

Root `LICENSE` should state multi-license clearly in `NOTICE`. Do **not** relicense upstream code.

## What not to put in the public repo

- Device `params.db`, real `wifi.txt`, SD card dumps, `sdcard.img` with private DATA  
- Kali rebuild logs that may contain lab hostnames  
- `*.bak.*` files  
- Hardcoded lab IPs as if they were product defaults (document as examples only)
