#!/bin/sh
# Re-apply wifi.txt when wlan0 comes online (e.g. after GUI wifi_power_on).
interface=$1
status=$2
case "$interface:$status" in
  wlan0:up|wlan0:pre-up|wlan0:dhcp4-change)
    [ -x /usr/bin/cybr_wifi_from_file.sh ] && /usr/bin/cybr_wifi_from_file.sh || true
    ;;
esac
exit 0
