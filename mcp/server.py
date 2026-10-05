# SPDX-License-Identifier: MIT
"""CybrX6100 V.01 MCP wrapper — stdio MCP server for the radio REST API."""
from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
from typing import Any

# Prefer httpx if installed; fall back to urllib.
try:
    import httpx  # type: ignore

    _HAS_HTTPX = True
except ImportError:
    _HAS_HTTPX = False

# Placeholder (RFC 5737); set CYBR_RADIO_URL to your radio, e.g. http://<radio-ip>:58080
BASE_URL = os.environ.get("CYBR_RADIO_URL", "http://192.0.2.10:58080").rstrip("/")
# Firmware dev default is CYBRX6100 -- CHANGE ME (see SECURITY.md). Sent as X-Cybr-Token, never Bearer.
TOKEN = os.environ.get("CYBR_RADIO_TOKEN", "CYBRX6100")

APPS = ("rtty", "ft8", "swr", "gps", "recorder", "settings", "callsign", "qth", "wifi")
ACTIONS = ("mute", "nr", "nb", "step_up", "step_down", "voice_mode", "battery", "screenshot")


def _headers() -> dict[str, str]:
    return {
        "X-Cybr-Token": TOKEN,
        "Accept": "application/json",
        "Content-Type": "application/json",
    }


def _request(method: str, path: str, timeout: float = 10.0) -> dict[str, Any]:
    url = f"{BASE_URL}{path}"
    if _HAS_HTTPX:
        with httpx.Client(timeout=timeout) as client:
            r = client.request(method, url, headers=_headers())
            body: Any
            try:
                body = r.json()
            except Exception:
                body = {"raw": r.text}
            return {"ok": r.is_success, "status": r.status_code, "url": url, "body": body}
    data = None if method.upper() == "GET" else b"{}"
    req = urllib.request.Request(url, data=data, headers=_headers(), method=method.upper())
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            raw = resp.read().decode("utf-8", errors="replace")
            try:
                body = json.loads(raw) if raw else {}
            except json.JSONDecodeError:
                body = {"raw": raw}
            return {"ok": True, "status": resp.status, "url": url, "body": body}
    except urllib.error.HTTPError as e:
        raw = e.read().decode("utf-8", errors="replace")
        try:
            body = json.loads(raw) if raw else {}
        except json.JSONDecodeError:
            body = {"raw": raw}
        return {"ok": False, "status": e.code, "url": url, "body": body}
    except Exception as e:
        return {"ok": False, "status": 0, "url": url, "body": {"error": str(e)}}


TOOLS = [
    {
        "name": "radio_health",
        "description": "GET / — CybrX6100 REST health/version check.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "radio_current_app",
        "description": "GET /apps — current GUI app name (none/ft8/rtty/...).",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "radio_open_app",
        "description": "POST /apps/{app} — open a GUI app dialog.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "app": {
                    "type": "string",
                    "enum": list(APPS),
                    "description": "App to open",
                }
            },
            "required": ["app"],
            "additionalProperties": False,
        },
    },
    {
        "name": "radio_close_app",
        "description": "POST /apps/close — close the current GUI app/dialog.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "radio_action",
        "description": "POST /actions/{name} — run a radio UI action.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "name": {
                    "type": "string",
                    "enum": list(ACTIONS),
                    "description": "Action name",
                }
            },
            "required": ["name"],
            "additionalProperties": False,
        },
    },
]


def _tool_result(payload: dict[str, Any]) -> dict[str, Any]:
    text = json.dumps(payload, indent=2)
    return {"content": [{"type": "text", "text": text}], "isError": not payload.get("ok", False)}


def call_tool(name: str, arguments: dict[str, Any] | None) -> dict[str, Any]:
    args = arguments or {}
    if name == "radio_health":
        return _tool_result(_request("GET", "/"))
    if name == "radio_current_app":
        return _tool_result(_request("GET", "/apps"))
    if name == "radio_close_app":
        return _tool_result(_request("POST", "/apps/close"))
    if name == "radio_open_app":
        app = str(args.get("app", "")).strip().lower()
        if app not in APPS:
            return _tool_result({"ok": False, "status": 0, "body": {"error": f"invalid app: {app}", "allowed": list(APPS)}})
        return _tool_result(_request("POST", f"/apps/{app}"))
    if name == "radio_action":
        action = str(args.get("name", "")).strip().lower()
        if action not in ACTIONS:
            return _tool_result({"ok": False, "status": 0, "body": {"error": f"invalid action: {action}", "allowed": list(ACTIONS)}})
        return _tool_result(_request("POST", f"/actions/{action}"))
    return _tool_result({"ok": False, "status": 0, "body": {"error": f"unknown tool: {name}"}})


def _send(msg: dict[str, Any]) -> None:
    line = json.dumps(msg, separators=(",", ":"))
    sys.stdout.write(line + "\n")
    sys.stdout.flush()


def _respond(req_id: Any, result: Any = None, error: dict[str, Any] | None = None) -> None:
    if error is not None:
        _send({"jsonrpc": "2.0", "id": req_id, "error": error})
    else:
        _send({"jsonrpc": "2.0", "id": req_id, "result": result})


def handle(msg: dict[str, Any]) -> None:
    method = msg.get("method")
    req_id = msg.get("id")
    params = msg.get("params") or {}

    # Notifications (no id) — ignore silently except initialized
    if req_id is None and method:
        return

    if method == "initialize":
        _respond(
            req_id,
            {
                "protocolVersion": "2024-11-05",
                "capabilities": {"tools": {}},
                "serverInfo": {"name": "cybr-x6100", "version": "0.1.0"},
            },
        )
        return

    if method == "tools/list":
        _respond(req_id, {"tools": TOOLS})
        return

    if method == "tools/call":
        name = params.get("name", "")
        arguments = params.get("arguments") or {}
        _respond(req_id, call_tool(name, arguments))
        return

    if method == "ping":
        _respond(req_id, {})
        return

    _respond(req_id, error={"code": -32601, "message": f"Method not found: {method}"})


def main() -> None:
    for raw in sys.stdin:
        line = raw.strip()
        if not line:
            continue
        try:
            msg = json.loads(line)
        except json.JSONDecodeError:
            continue
        try:
            handle(msg)
        except Exception as e:
            rid = msg.get("id") if isinstance(msg, dict) else None
            if rid is not None:
                _respond(rid, error={"code": -32000, "message": str(e)})


if __name__ == "__main__":
    main()
