# READY — CybrX6100 staging tree

Prepared 2026-10-05 (ET) for private repo **`cybrlou/CybrX6100`**. **Not** git-initialized, **not** pushed, nothing flashed.

- **Files:** 70 (this READY.md included), ~0.9 MB
- **Sources:** Kali build host trees — AetherX6100Buildroot @ `07a8d25` + uncommitted Cybr changes; x6100_gui @ `02f5240` + uncommitted Cybr changes. Exported read-only (source repos not modified).

## Verified

| Check | Result |
|-------|--------|
| `patches/gui/*.patch` `git am` onto fresh upstream gdyuldin/x6100_gui @ 02f5240 | ✅ 3/3 apply; resulting `src/` byte-identical to the Kali working tree |
| `patches/buildroot/*.patch` `git apply` onto fresh gdyuldin/AetherX6100Buildroot @ 07a8d25 | ✅ all apply; results byte-identical to `overlay/` (incl. binary logo.png) |
| `scripts/apply-overlay.sh --mode copy` vs `--mode patch --gui` | ✅ identical `br2_external/`; writes `build/local.mk` with `X6100_GUI_OVERRIDE_SRCDIR` |
| `mcp/server.py` initialize / tools/list | ✅ 5 tools; mock server confirmed header `X-Cybr-Token` sent, no `Authorization` |
| `git init && git add -A` dry run (temp copy) | ✅ 70 files staged, nothing ignored unexpectedly |
| Secret scan | ✅ no real SSID/PSK, no Kali password, no `*.bak.*`, no params.db/images/logs |

## Push (when Lou approves)

```bash
cd /workspace/CybrX6100
git init -b main && git add -A && git commit -m "CybrX6100 overlay + patches (BR 07a8d25, GUI 02f5240)"
gh repo create cybrlou/CybrX6100 --private --source . --push   # or add remote + git push -u origin main
```

## Gaps / decisions left

1. **REST token is still hardcoded** `#define CYBR_TOKEN "CYBRX6100"` in `cybr_http.c` (patch 0002). Documented as CHANGE ME in SECURITY.md; making it file/env-based is a future GUI change.
2. **SSH `root`/`123`** comes from upstream defconfig `BR2_TARGET_GENERIC_ROOT_PASSWD` — not overridden here (no defconfig patch). Documented as CHANGE ME.
3. `gui-src/ssh.cpp` / `ssh.h` have **no SPDX header** (left unmodified so they match patch 0003); NOTICE covers them as LGPL-2.1-or-later. Add headers in a follow-up commit if desired.
4. **GUI patches were split by me** from uncommitted changes into 3 commits (author "CybrX <cybrx@users.noreply.github.com>"). 0002 builds only on top of 0001; apply as a series. Change author with `git am --committer-date-is-author-date` / rebase if you want your own identity.
5. Layout deviation from the original sketch: Buildroot `.patch` files live in `patches/buildroot/` (not inside `overlay/`); `overlay/` is full files only. Noted at top of REPO_LAYOUT.md.
6. `scripts/rebuild-docker.sh.example` is a sanitized rewrite of the Kali helpers (no sudo password, no lab paths) — **not test-run** in this session.
7. The overlay `usr/lib/python3.11/site-packages/x6100_webserver/static/css/base.css` came along as an untracked Cybr file; confirm it's intended (it duplicates/overrides upstream's webserver CSS path).
8. Not included on purpose: `eva-pack/apply_on_kali.py`, Kali rebuild logs, `SSH_BUILD_REPORT.txt`, `sdcard.img`, `*.bak.*`.
9. Still open for you: whether to keep the INVENTORY lab IP/host references (`192.168.60.x`, Kali paths) — fine for a private repo, scrub if it ever goes public.
