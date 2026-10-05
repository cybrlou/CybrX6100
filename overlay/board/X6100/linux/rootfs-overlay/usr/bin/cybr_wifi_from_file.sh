#!/bin/sh
# CybrX6100: apply Wi-Fi from DATA partition text file (/mnt/wifi.txt).
# Soft-fail if missing/invalid. Never log the PSK. Idempotent via fingerprint.

WIFI_FILE="${CYBR_WIFI_FILE:-/mnt/wifi.txt}"
CONN_DIR="/etc/NetworkManager/system-connections"
CONN_FILE="$CONN_DIR/cybr-from-file.nmconnection"
FP_FILE="/var/lib/cybr-wifi.fp"
IFACE="wlan0"
LOGTAG="cybr-wifi"

log() { echo "$LOGTAG: $*" ; }

# Soft-fail helpers
[ -r "$WIFI_FILE" ] || { log "no $WIFI_FILE (skip)"; exit 0; }

# Parse ssid= / psk= (ignore blanks and # comments). Do not echo psk.
SSID=""
PSK=""
while IFS= read -r line || [ -n "$line" ]; do
    line=$(printf '%s' "$line" | tr -d '\r')
    case "$line" in
        ""|\#*) continue ;;
        ssid=*|SSID=*) SSID=${line#*=} ;;
        psk=*|PSK=*|password=*|PASSWORD=*) PSK=${line#*=} ;;
    esac
done < "$WIFI_FILE"

# trim spaces
SSID=$(printf '%s' "$SSID" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
PSK=$(printf '%s' "$PSK" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

if [ -z "$SSID" ]; then
    log "ssid empty in wifi.txt (skip)"
    exit 0
fi

# Fingerprint without writing psk to logs
FP=$(printf '%s\n%s' "$SSID" "$PSK" | md5sum | awk '{print $1}')
if [ -f "$FP_FILE" ] && [ "$(cat "$FP_FILE" 2>/dev/null)" = "$FP" ] && [ -f "$CONN_FILE" ]; then
    log "already applied for ssid='$SSID' (skip)"
    exit 0
fi

mkdir -p "$CONN_DIR" /var/lib 2>/dev/null || true

# Stable UUID from ssid (not secret)
HEX=$(printf '%s' "$SSID" | md5sum | awk '{print $1}')
UUID=$(printf '%s' "$HEX" | sed 's/^\(........\)\(....\)\(....\)\(....\)\(............\).*/\1-\2-\3-\4-\5/')

# Escape not needed for NM keyfile if we avoid newlines; write carefully
umask 077
{
    printf '%s\n' "[connection]"
    printf '%s\n' "id=$SSID"
    printf '%s\n' "uuid=$UUID"
    printf '%s\n' "type=wifi"
    printf '%s\n' "interface-name=$IFACE"
    printf '%s\n' "autoconnect=true"
    printf '%s\n' "autoconnect-retries=3"
    printf '%s\n' ""
    printf '%s\n' "[wifi]"
    printf '%s\n' "mode=infrastructure"
    printf '%s\n' "ssid=$SSID"
    printf '%s\n' ""
    if [ -n "$PSK" ]; then
        printf '%s\n' "[wifi-security]"
        printf '%s\n' "auth-alg=open"
        printf '%s\n' "key-mgmt=wpa-psk"
        printf '%s\n' "psk=$PSK"
        printf '%s\n' ""
    fi
    printf '%s\n' "[ipv4]"
    printf '%s\n' "method=auto"
    printf '%s\n' ""
    printf '%s\n' "[ipv6]"
    printf '%s\n' "method=ignore"
} > "$CONN_FILE"
chmod 600 "$CONN_FILE"

# Ask NetworkManager to reload connection files (no nmcli on this image)
if pidof NetworkManager >/dev/null 2>&1; then
    if [ -x /usr/bin/dbus-send ]; then
        dbus-send --system --type=method_call \
            --dest=org.freedesktop.NetworkManager \
            /org/freedesktop/NetworkManager \
            org.freedesktop.NetworkManager.ReloadConnections \
            >/dev/null 2>&1 || log "ReloadConnections dbus call soft-failed"
    else
        kill -HUP "$(pidof NetworkManager)" 2>/dev/null || true
    fi
else
    log "NetworkManager not running yet; keyfile written for later"
fi

printf '%s\n' "$FP" > "$FP_FILE"
log "applied ssid='$SSID' -> $CONN_FILE"
exit 0
