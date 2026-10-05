# MCP stdio wrapper

Python stdio MCP server that proxies to the radio REST API (`cybr_http`).

## Files

- `mcp/server.py` (from lab `cybr-mcp/server.py`; MIT, see `mcp/LICENSE`)
- `mcp/requirements.txt` (optional `httpx`)

## Environment

| Variable | Default (sanitize for OSS) | Meaning |
|----------|----------------------------|---------|
| `CYBR_RADIO_URL` | `http://192.0.2.10:58080` (placeholder; lab radio was `192.168.60.117`) | Base URL (no trailing slash required) |
| `CYBR_RADIO_TOKEN` | `CYBRX6100` (firmware dev default — **CHANGE ME**) | Value for `X-Cybr-Token` |

Uses `httpx` if installed; otherwise stdlib `urllib`.

## Tools

| Tool | REST |
|------|------|
| `radio_health` | `GET /` |
| `radio_current_app` | `GET /apps` |
| `radio_open_app` | `POST /apps/{app}` — enum: rtty, ft8, swr, gps, recorder, settings, callsign, qth, wifi |
| `radio_close_app` | `POST /apps/close` |
| `radio_action` | `POST /actions/{name}` — enum: mute, nr, nb, step_up, step_down, voice_mode, battery, screenshot |

## Auth gotcha

The radio expects **`X-Cybr-Token`**. Do not configure clients to send `Authorization: Bearer …`.

## Example MCP config (Cursor / Claude Desktop style)

```json
{
  "mcpServers": {
    "cybr-x6100": {
      "command": "python3",
      "args": ["/path/to/cybrx6100/mcp/server.py"],
      "env": {
        "CYBR_RADIO_URL": "http://192.0.2.10:58080",
        "CYBR_RADIO_TOKEN": "CHANGE_ME_TOKEN"
      }
    }
  }
}
```

Server advertises `serverInfo.name = "cybr-x6100"`, protocol `2024-11-05`, version `0.1.0`.
