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
- **The Lamp card.** The main page switches the lamp on and off, sets the brightness from 5 to
  100 % in steps of 5 and selects the mode: **Standard**, **Multi**, **Night** with its own night
  window, or **Lamp light** in one of six colours. The grid shows a change at once; the lamp shows
  it within about a second. See [The Lamp card](web-interface.md#lamp-card) and
  [Display modes](display-modes.md).
- **Saved settings.** The lamp writes every change to its flash before it shows it, so a reset
  right after a change keeps it. With no file, or a damaged one, it starts with the defaults: on,
  35 %, multi mode. See [Saved settings](display-modes.md#saved-settings).
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
- **Delete one network.** Every saved network has a **×** button with a confirmation in its row.
  Deleting the network the lamp is in, or all networks while the lamp is in your network, restarts
  the lamp after about 1 second; it then tries the remaining networks in order and starts its
  access point if none joins. Before, deleting all networks left the lamp in a network that was no
  longer saved. The page shows the results in its language. See
  [Saved networks](web-interface.md#saved-networks).

- **Restart from the page.** The red button **Restart lamp** on the main page restarts the lamp
  without unplugging it, after you confirm. The page shows **Lamp is restarting…** and reloads
  itself when the lamp is back. The lamp refuses while it checks a new WiFi network. The log line
  of a WiFi restart is now `Restart: WiFi network change`, and a restart from the page logs
  `Restart: web page request`. See [Restart the lamp](web-interface.md#restart-lamp).

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
  tries the sensors again every 5 minutes; the colours start when they answer. Lamp light and
  power off work meanwhile and replace the random pixels; a measurement mode keeps them.
- **Brightness.** The lamp starts at 35 %. The colours of the measurements no longer depend on the
  brightness: the colour scale is fixed and the LEDs dim it. The start-up snake and the WiFi
  pictures play at 35 %, not at the saved brightness.
- **Night light.** The night mode shows 18 cells that fill from the bottom over a window you set,
  instead of six bars that grew by one LED per hour from 21:00 to 09:00.
- **Overall reading.** The standard mode can show **Overall**: the first reading out of its range,
  or the average colour.
- **No dark blink.** The matrix no longer goes dark for about 0.3 seconds before each redraw or
  after a change of brightness or mode.
- **First picture sooner.** A picture is drawn as soon as its readings exist, not after the first
  full minute, and a mode with no reading yet no longer shows a meaningless colour.
- **All three sensors must answer.** The lamp starts the measurements only when the CCS811, the
  DHT22 and the ZH03B all answer. A missing DHT22 or ZH03B now gives the random pixels, and the
  log names each missing sensor (`Sensors start: DHT22 not responding`, `Sensors start: ZH03B
  sends no frames`, `Sensors start: CCS811 not found`) instead of repeating `Failed to get ... meas`.
- **Slower start, longer retry.** With working sensors the start takes about 2 seconds longer. With
  a missing DHT22 or ZH03B each start attempt waits up to 5 seconds, so the start and every 5-minute
  retry block the lamp (the web pages, the blinking LED) for up to about 27 seconds instead of
  about 6 seconds.
- **No random colour flashes.** Parts of the matrix no longer flash random colours when the lamp
  writes to its file system or connects to WiFi. The matrix is now driven by the I2S hardware
  through DMA, which flash access does not disturb; colours and brightness are unchanged. An
  animation may still freeze for a fraction of a second when WiFi connects; see
  [The matrix looks wrong](troubleshooting.md#the-matrix-looks-wrong).
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
- [DuckDNS updates](build-and-flash.md#set-the-build-variables).
