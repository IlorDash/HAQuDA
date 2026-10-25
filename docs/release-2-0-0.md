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

## Known limitations (WIP)

These features are not ready in this version.

- [The lamp's address in your network](web-interface.md#page-addresses) is not reported.
- [Opening the page automatically](web-interface.md#page-addresses) in access point mode.
- [The clock time in log lines](web-interface.md#log-page).
- [The **Show** button](web-interface.md#main-page).
- [The **IP Address** and **Gateway Address** fields, and several stored networks](web-interface.md#add-wifi-credentials-page).
- [Recovery after a WiFi loss](troubleshooting.md#running-effects).
- [Display mode selection](quick-guide.md#select-the-display-mode).
- [DuckDNS updates](build-and-flash.md#set-the-build-variables).

In access point mode the [log page](web-interface.md#log-page) is not available.
