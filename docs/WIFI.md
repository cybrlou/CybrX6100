# Wi‑Fi via DATA `wifi.txt`

## File location

On the radio, DATA is mounted at **`/mnt`**. Create:

```text
/mnt/wifi.txt
```

From a PC, mount the SD **DATA** partition and place `wifi.txt` in its root. An example is seeded as `/mnt/wifi.txt.example` (from `/usr/share/cybr/wifi.txt.example`).

## Format

```text
# Comments start with #
ssid=YourNetworkName
psk=YourWifiPassword
```

Keys are case-insensitive (`SSID`/`PSK`/`password` accepted for the secret). Blank lines ignored.

## How it is applied

1. Boot: `S01create_data` mounts DATA; `S46cybr_wifi` runs `cybr_wifi_from_file.sh`.
2. Script writes NetworkManager keyfile `/etc/NetworkManager/system-connections/cybr-from-file.nmconnection` and asks NM to reload.
3. Dispatcher `98-cybr-wifi.sh` re-runs the script when `wlan0` comes up (useful after GUI Wi‑Fi power-on).

PSK is **never** written to syslog. Application is idempotent (fingerprint file `/var/lib/cybr-wifi.fp`).

## GUI

Newer gdyuldin GUI includes a Wi‑Fi dialog (APP pages). File-based config remains useful for headless first connect.

## Security

Do not commit real `wifi.txt` files. FAT DATA is readable on any PC with the SD card.
