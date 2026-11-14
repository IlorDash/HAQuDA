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

- **New web pages.** The main page shows the readings in tiles with an air index, the signal
  bars and the name of the network the lamp is in. It loads nothing from the internet. Both pages
  come in English and Russian, in a light and a dark theme; see [Main page](web-interface.md#main-page).
- **The lamp's picture on the page.** A grid of 4 × 6 cells repeats what the matrix shows: the
  measurement layouts in the colours the lamp drew, the snake, the growing, the random pixels, the
  fading and the single colours, and a caption that names what the lamp is doing. See
  [The lamp's picture](web-interface.md#lamp-picture).
- **Address `haquda.local`.** The pages open at `http://haquda.local/` in your network and on the
  lamp's access point, and the router lists the lamp as `haquda`. See
  [Page addresses](web-interface.md#page-addresses).
- **WiFi check before saving.** In access point mode the lamp joins a new network before it saves
  it, for up to 20 seconds, and the page shows the progress, the new address and, after the
  restart, opens that address by itself. A wrong password, a missing network or no answer is
  reported and nothing is saved. See [Add a network](web-interface.md#add-a-network).
- **Adding a network in your network.** The lamp scans for the new network first. If it is there,
  the page warns that the lamp goes away, and the lamp tries it. After a failure the lamp restarts
  to its stored networks and both pages show the reason in a banner you can close. See
  [While the lamp is in your network](web-interface.md#add-in-station-mode).
- **Up to 10 networks.** The newest network goes first and the lamp tries it first. When the list
  is full, the page names the oldest network that the new one removes; see
  [Saved networks](web-interface.md#saved-networks).

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
- **All three sensors must answer.** The lamp starts the measurements only when the CCS811, the
  DHT22 and the ZH03B all answer. A missing DHT22 or ZH03B now gives the random pixels, and the
  log names each missing sensor (`Sensors start: DHT22 not responding`, `Sensors start: ZH03B
  sends no frames`, `Sensors start: CCS811 not found`) instead of repeating `Failed to get ... meas`.
- **Slower start, longer retry.** With working sensors the start takes about 2 seconds longer. With
  a missing DHT22 or ZH03B each start attempt waits up to 5 seconds, so the start and every 5-minute
  retry block the lamp (the web pages, the blinking LED) for up to about 27 seconds instead of
  about 6 seconds.
- **No random colour flashes.** Parts of the matrix no longer flash random colours when the lamp
  writes to its file system. While the lamp reads or writes it, an animation can pause for a
  moment; see [The matrix looks wrong](troubleshooting.md#the-matrix-looks-wrong).
- **File system error.** The matrix shows the red fading.
- **Correct PM values.** The lamp reads PM1.0, PM2.5 and PM10 from the right fields of the ZH03B
  frame. The PM2.5 colour in the multi, standard and night modes can differ from earlier builds.
- **Fresh PM readings.** Every 2 seconds the lamp uses the newest frame the ZH03B has sent, so the
  PM2.5 colour follows the air instead of lagging by up to about 30 seconds.
- **PM warm-up.** For 12 seconds after the sensors power on (start, the 5-minute retry, a sensor
  reboot) the lamp takes no PM readings, so the ZH03B's start-up number does not end up in the
  first PM2.5 average. The line `Failed to get PM 2.5 meas` is not logged in this window.
- **Quieter log.** With the ZH03B connected, `Failed to get PM 2.5 meas` no longer appears every
  40 to 50 seconds. With the ZH03B disconnected it still repeats every few seconds.

## Removed

- **Old controls.** The main page no longer has **Delete** and **Show**, and the form no longer
  has **IP Address** and **Gateway Address**. **Delete all networks** is on the
  [WiFi networks page](web-interface.md#wifi-networks-page).

## Known limitations (WIP)

These features are not ready in this version.

- [Opening the page automatically](web-interface.md#page-addresses) in access point mode.
- [Recovery after a WiFi loss](troubleshooting.md#running-effects).
- [Display mode selection](quick-guide.md#select-the-display-mode).
- [DuckDNS updates](build-and-flash.md#set-the-build-variables).
