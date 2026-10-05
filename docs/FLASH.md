# Flashing a spare microSD (recommended)

**Do not** flash the radio’s internal storage from these docs. Use a **spare microSD**, so you can remove the card and restore stock/vendor behavior.

## What you need

- Built or released `sdcard.img`
- microSD large enough for the image (BOOT is **512 MB** VFAT + rootfs ext4 + free space for DATA). Prefer **16 GB+**.
- balenaEtcher, Raspberry Pi Imager, Rufus (DD mode), or `dd` on Linux.

## Flash (Linux example)

```bash
# Identify the SD device carefully — wrong disk destroys data
lsblk
sudo dd if=sdcard.img of=/dev/sdX bs=4M status=progress conv=fsync
sync
```

## First boot

1. Power off the X6100.
2. Insert the card.
3. Power on; CybrX splash should appear, then the LVGL GUI.
4. On first boot with free space, `S01create_data` creates a FAT32 partition labeled **DATA** and mounts it at `/mnt`.
5. Immediately change SSH password if the radio will be on a network (`passwd root`). See [SSH.md](SSH.md) and [SECURITY.md](../SECURITY.md).

## Optional: Wi‑Fi without GUI

On a PC, mount the DATA partition and create `wifi.txt` (see [WIFI.md](WIFI.md)).

## Rollback

Remove the microSD and boot from internal storage / previous card.
