# Web configurator theme (Eva / WEBMODERN)

The on-radio Python webserver (`x6100_webserver`) is restyled with CybrX branding.

## Assets

Installed via package post-install hook and/or rootfs overlay:

- `static/css/cybrtch.css` — design tokens and `simple.min.css` overrides
- `views/base.html` — wordmark **CybrX 6100**, nav tabs, no CYBRTCH subtitle
- Concise page templates: `index`, `bands`, `digital_modes`, `files`, `time`

## Tokens (excerpt)

```css
--cybr-bg: #0B100E;
--cybr-surface: #121816;
--cybr-text: #E8ECEA;
--cybr-muted: #9AA3A0;
--cybr-border: #2A3330;
--cybr-accent: #39FF14;
```

`header .subtitle { display: none }` removes leftover subtitle text. Nav links are restyled so they are not mistaken for `simple.css` buttons.

## Access

Same as upstream web UI (typically HTTP on the radio’s LAN IP — see upstream webserver package docs). Theme does not change authentication model of the Bottle app.
