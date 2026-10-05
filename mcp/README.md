# CybrX6100 MCP server (`cybr-x6100`)

Stdio MCP server that proxies to the radio's Cybr REST API (`cybr_http.c`, TCP **58080**).
Full docs: [../docs/MCP.md](../docs/MCP.md). License: **MIT** ([LICENSE](LICENSE)).

```bash
pip install -r requirements.txt           # optional (httpx); stdlib urllib fallback otherwise
export CYBR_RADIO_URL=http://<radio-ip>:58080
export CYBR_RADIO_TOKEN=CHANGE_ME_TOKEN   # firmware dev default is CYBRX6100
python3 server.py
```

## Auth

Every request sends **`X-Cybr-Token: $CYBR_RADIO_TOKEN`**. The radio does **not** accept
`Authorization: Bearer …` — do not configure clients that way.

## Tools

| Tool | REST |
|------|------|
| `radio_health` | `GET /` |
| `radio_current_app` | `GET /apps` |
| `radio_open_app` | `POST /apps/{rtty,ft8,swr,gps,recorder,settings,callsign,qth,wifi}` |
| `radio_close_app` | `POST /apps/close` |
| `radio_action` | `POST /actions/{mute,nr,nb,step_up,step_down,voice_mode,battery,screenshot}` |

## Client config example

```json
{
  "mcpServers": {
    "cybr-x6100": {
      "command": "python3",
      "args": ["/path/to/CybrX6100/mcp/server.py"],
      "env": { "CYBR_RADIO_URL": "http://192.0.2.10:58080", "CYBR_RADIO_TOKEN": "CHANGE_ME_TOKEN" }
    }
  }
}
```

Defaults in `server.py`: URL placeholder `http://192.0.2.10:58080` (set your radio IP), token `CYBRX6100`
(matches the firmware dev default — **CHANGE ME**).
