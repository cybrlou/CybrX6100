# SSH (OpenSSH)

## Defaults (CHANGE ME)

| Item | Value |
|------|-------|
| User | `root` |
| Password | `123` (Buildroot `BR2_TARGET_GENERIC_ROOT_PASSWD`) |
| Boot default | **Enabled** |
| Preference file | `/mnt/ssh.enabled` (`1` = on, `0` = off; missing treated as on) |
| GUI param | `ssh_enabled` in `params.db` (default true) |
| Help text on image | `/usr/share/cybr/SSH.txt` (also seeded conceptually for operators) |

`sshd_config`: password auth yes; **empty passwords disabled**.

## Toggle

- **APP page** (page with Wi‑Fi): button **SSH: On/Off**. Writes `/mnt/ssh.enabled` and starts/stops `/etc/init.d/S50sshd`.
- Or edit `/mnt/ssh.enabled` on DATA from a PC and reboot.

## First-login hardening

```bash
ssh root@<radio-ip>
passwd root
# optional:
mkdir -p /root/.ssh
# install your pubkey into authorized_keys
```

If the radio is on Wi‑Fi/Ethernet and you do not need shell access, turn SSH **Off** in the APP page.

## Init script

`S50sshd` reads `/mnt/ssh.enabled` before starting `sshd`. `S01create_data` seeds the file to `1` once DATA is mounted if missing.
