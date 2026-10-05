# SD card partitions

Cybr image layout (genimage + first-boot DATA):

| Partition | Label | FS | Size / notes |
|-----------|-------|----|--------------|
| p1 | **BOOT** | FAT32 (VFAT) | **512 MB** (Cybr change; upstream was 32 MB). Kernel, DTB, boot.scr, etc. |
| p2 | (rootfs) | ext4 | Root filesystem from Buildroot |
| p3 | **DATA** | FAT32 | Created on first boot from free space if missing; mounted at **`/mnt`** |

## DATA create fix

`S01create_data` must **not** treat the BOOT FAT32 volume as “DATA already exists”. Cybr logic:

1. Prefer `blkid -L DATA`.
2. Else, if `/dev/mmcblk0p3` exists **and** its LABEL is `DATA`, use it.
3. Otherwise create a new primary FAT32 partition in free space, `mkfs.vfat -n DATA`, mount on `/mnt`.

Then seed:

- `/mnt/wifi.txt.example` (if absent)
- `/mnt/ssh.enabled` → `1` (if absent)

## Operator files on DATA

| File | Purpose |
|------|---------|
| `wifi.txt` | Optional NM Wi‑Fi config (see WIFI.md) |
| `wifi.txt.example` | Template |
| `ssh.enabled` | `1`/`0` SSH preference |
| Upstream ADIF / logs | e.g. `incoming_log.adi`, `ft_log.adi` per gdyuldin GUI docs |

## Image artifacts

After `make`, under `build/images/`:

- `sdcard.img` — whole-card image to flash
- `boot.vfat` — should be 512 MiB when Cybr genimage patch applied
- `rootfs.ext2` / `rootfs.ext4`
