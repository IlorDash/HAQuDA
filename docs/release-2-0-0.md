# Version {{ app_version }}

<!-- unreleased: at release, delete this comment and the banner below it, and put a
     release date line in their place, e.g. `*Released 2 September 2026.*`. While this
     marker is present in any markdown file under docs/, CI blocks the release pull
     request and refuses to publish the site. -->
!!! info "In preparation — not released yet"
    This version is not out. Everything below is already in the build, but it can still
    change before the release.

## What's new

- **User guide.** This version is the first one with a user guide, in English and Russian. It
  covers the first start, the WiFi setup, the display modes, the web pages and the LED effects.
- **Version at start.** The lamp prints `HAQuDA firmware v{{ app_version }}` on its serial port
  at 115200 baud when it starts, so you can tell which firmware runs on a lamp.

## Improvements

- **Log time.** In your network, log lines carry the date and time, for example
  `12.11.2026 14:05:31`. Lines logged without a time server answer keep the uptime stamp
  `up 12.345 s`. The lamp asks the time server at most once a minute, so log lines no longer
  stall when the server is unreachable.
- **Slow routers.** The lamp waits up to about 8 seconds for each connection attempt instead of
  about 3, so a slow router gets connected. With a wrong stored password the lime snake now runs
  for about 30 seconds before the access point.
- **WiFi error visible.** After a failed connection the red growing shows for about 10 seconds,
  so you can see it before the access point effect.
- **Green flash.** The green flash starts once the lamp has an address from your router. After
  a lost WiFi connection returns, the green flash shows and the colours come back.
- **Sensors that do not start.** The lamp no longer restarts itself. It shows random pixels and
  tries the sensors again every 5 minutes; the colours start when they answer.
- **File system error.** The matrix shows the red fading.

## Known limitations (WIP)

These features are not ready in this version.

- [The lamp's address in your network](web-interface.md#page-addresses) is not reported.
- [Opening the page automatically](web-interface.md#page-addresses) in access point mode.
- [The **Show** button](web-interface.md#main-page).
- [The **IP Address** and **Gateway Address** fields, and several stored networks](web-interface.md#add-wifi-credentials-page).
- [Recovery after a WiFi loss](troubleshooting.md#running-effects).
- [Display mode selection](quick-guide.md#select-the-display-mode).
- [DuckDNS updates](build-and-flash.md#set-the-build-variables).

In access point mode the [log page](web-interface.md#log-page) is not available.
