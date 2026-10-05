# Building CybrX6100 firmware

## Prerequisites

- Linux host (or Docker). Lab builds used **Ubuntu Jammy** in Docker on Kali because host GCC was too new for the Buildroot snapshot.
- ~20+ GB disk, several GB RAM.
- Git, make, Docker (recommended), and Buildroot host deps ([upstream docs](https://github.com/gdyuldin/AetherX6100Buildroot)).

## Clone upstream

```bash
mkdir -p ~/x6100 && cd ~/x6100
git clone https://github.com/gdyuldin/AetherX6100Buildroot
git clone https://github.com/gdyuldin/x6100_gui
cd AetherX6100Buildroot && git submodule update --init --recursive
cd ../x6100_gui && git submodule update --init --recursive
```

Pin GUI to the commit Cybr was developed against (or newer, then re-test):

```bash
cd ~/x6100/x6100_gui
git checkout 02f524068f8ec19031331bab38f06eb212efd47b
```

## Apply Cybr overlay / patches

Upstream pins: **AetherX6100Buildroot `07a8d25`**, **x6100_gui `02f5240`**.

```bash
cd ~/x6100/AetherX6100Buildroot && git checkout 07a8d25 && ./br_config.sh   # creates build/
git clone <your private CybrX6100 remote> ~/x6100/CybrX6100
cd ~/x6100/CybrX6100
# Buildroot overlay (copy full files) + GUI patch series (git am) + build/local.mk:
./scripts/apply-overlay.sh --br ~/x6100/AetherX6100Buildroot --gui ~/x6100/x6100_gui
# or review-friendly: --mode patch  (git apply patches/buildroot/*.patch, then add Cybr-new files)
# preview only:       --dry-run
```

`apply-overlay.sh --gui` writes `build/local.mk` (see `scripts/local.mk.example`):

```make
X6100_GUI_OVERRIDE_SRCDIR = /home/YOU/x6100/x6100_gui
```

The overlay's `package/x6100-gui/x6100_gui.mk` already pins `X6100_GUI_VERSION = 02f524068f8ec19031331bab38f06eb212efd47b`, but the Cybr GUI code only comes from the patched OVERRIDE tree.

## Configure and build

```bash
cd ~/x6100/AetherX6100Buildroot
./br_config.sh
cd build
# optional: make menuconfig
make BR2_JLEVEL=$(nproc)
```

Output image:

```text
build/images/sdcard.img
```

### Docker Jammy pattern (lab)

Lab used an image `x6100-jammy` from `Dockerfile.jammy`, mounting `~/x6100`, working directory `AetherX6100Buildroot/build`, with `BR2_DL_DIR` pointing at `buildroot/dl`. Inside the container, cmake **3.28.x** was installed via pip because older host cmake broke the GUI build. A sanitized version of the lab helpers is in `scripts/rebuild-docker.sh.example` + `scripts/Dockerfile.jammy`.

### Incremental rebuilds

| Change | Typical rebuild |
|--------|-----------------|
| GUI / REST / SSH C sources | `make x6100-gui-dirclean && make x6100-gui-reconfigure && make` |
| Web theme | `make x6100-webserver-dirclean && make x6100-webserver && make` |
| Overlay init scripts / genimage | `make` (rootfs + genimage) |

## Verify before flash

```bash
strings build/target/usr/sbin/x6100_gui | grep -E 'CybrX6100|58080'
test -f build/target/usr/bin/cybr_wifi_from_file.sh
test -f build/target/etc/init.d/S46cybr_wifi
test -f build/target/etc/init.d/S50sshd
ls -la build/images/boot.vfat   # expect ~512 MiB
```
