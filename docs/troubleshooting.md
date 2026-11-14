# LED codes and troubleshooting

The lamp reports problems as LED effects and as lines in its log. Most errors turn the matrix
dark, so the log tells you more than the matrix. Open `/webserial` on the lamp, in your
network or at `http://192.168.4.22/webserial` on its access point (see
[Web interface](web-interface.md#log-page)), and copy the log lines when you report a
problem.

Effects in the tables use these names:

- **snake** — one or more lit LEDs run along the matrix.
- **fading** — the whole matrix lights up and dims step by step.
- **static** — the whole matrix keeps one colour.
- **growing** — the matrix fills with one colour row by row, again and again.
- **random pixels** — the LEDs light up one by one in random colours, again and again.

---

## Start-up effects

| Effect | Cause | Action |
|------------------------|-------|------------|
| Lime snake | The lamp starts, or it reconnects to a stored WiFi network. | Wait for it to end. |
| Green, whole matrix, about 3 seconds | The lamp joined your WiFi network and got an address from your router. | Nothing. The matrix goes dark, then shows colours after about a minute. |
| Red growing for about 12 seconds, then an aqua fading. Before the red growing the lime snake runs for about 2 seconds, or for about 30 seconds when a network is stored | No network is stored, or the lamp could not join the stored one. It opens its access point. | Nothing on the first start. Otherwise check the name and the password of your network and add it again ([Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi)). |
| Aqua fading, once | The access point runs and the sensors start. | Join the lamp's network and open `http://192.168.4.22/`. |
| Dark matrix that stays dark, the lamp's WiFi network usually does not appear, the LED on the ESP32 board does not blink | The lamp could not create its access point and stops. | Unplug the lamp and plug it in again. If it repeats, check the access point password in your build (8 to 63 characters). |
| Random pixels after the green flash or the aqua fading, until the sensors start | The lamp could not start its sensors (the CCS811, the DHT22 or the ZH03B does not answer). It tries up to three times to power the sensors off and on, keeps the effect, and tries again every 5 minutes. Each round of tries blocks the lamp for several seconds, up to about 27 seconds when the DHT22 or the ZH03B is missing. | Check the sensor power and the wiring; the log names the missing sensor (see [Log messages](#log-messages)). The colours start by themselves once the sensors answer. In your network, when the time server has not answered since the start, the matrix stays dark instead of showing the effect. |
| Red fading, again and again; it does not stop | The lamp could not start its file system and stops. | Unplug the lamp and plug it in again. If it repeats, flash the lamp again. |

---

## Running effects

| Effect | Cause | Action |
|------------------------|-------|------------|
| Red, whole matrix, about 3 seconds, then a lime snake | The lamp lost the WiFi connection and reconnects. | Wait. |
| Red, whole matrix, about 3 seconds, then a dark matrix | You added a network while the lamp ran in your network, and it left its network to try the new one. | Wait for the restart; see [While the lamp is in your network](web-interface.md#add-in-station-mode). |
| Lime snake that does not end after the lamp lost its WiFi <span class="wip">WIP</span> | The router is gone. The lamp retries without a limit. | Bring the router back, or unplug the lamp and plug it in again. |

!!! note "WIP"
    Recovery after a WiFi loss is not ready: the lamp does not open its access point and retries
    without a limit. Once it rejoins, the matrix flashes green for about 3 seconds and the colours
    return. If the router stays away, unplug the lamp and plug it in again.

---

## The matrix looks wrong

| Effect | Cause | Action |
|------------------------|-------|------------|
| The matrix is dark for the first minute after a start | The lamp averages the first minute of readings before it draws. | Wait. In the standard and night mode the matrix can show a meaningless colour during this minute. |
| The picture does not change for several minutes | In multi mode, one of temperature, eCO2 or PM2.5 delivers no reading, and the lamp keeps the old picture. One missed CCS811 reading discards the whole minute of eCO2 and TVOC. | Open the log and look for `Failed to get ... meas` lines (see [Log messages](#log-messages)). Check the wiring of that sensor. |
| eCO2 and TVOC never show green | The lamp shows these two values in steps. | Nothing; see [Colour scale](display-modes.md#colour-scale). |
| An animation, for example the lime snake, pauses for a moment | The lamp reads or writes its file system (log lines, saved WiFi networks). A picture that has to wait more than half a second is skipped, and the matrix keeps the previous one. The measurement colours are sent again soon after. | Nothing. Parts of the matrix no longer flash random colours during these writes. |
| The LED on the ESP32 board does not blink | The lamp has not finished its start, or its main loop is blocked, usually while it retries a lost WiFi connection. | Wait for the start or the reconnect. If the LED stays still for minutes, unplug the lamp and plug it in again. |

The LED on the ESP32 board blinks once per second while the lamp works normally.

---

## Log messages

Each line has the form `time - level : message`; see [Log page](web-interface.md#log-page).

| Message | Level | Cause | Action |
|---------|-------|-------|------------|
| `Started File system` | Info | The file system works. | Nothing. |
| `Connected to WiFi` | Info | The lamp joined your network. | Nothing. |
| `Created Acces Point` | Info | The lamp opened its access point. | Join it and open `http://192.168.4.22/`. |
| `Started sensors` | Info | The sensors answer. | Nothing. |
| `Failed connection to WiFi` | ERROR | The lamp could not join a stored network. | Check the name and the password of your network. |
| `Failed creating Acces Point` | ERROR | The access point cannot start. | See the dark matrix above. |
| `CreateAP: captive DNS not started` | ERROR | The lamp's DNS for the access point did not start. | Open `http://192.168.4.22/` by hand. |
| `startMdns: mDNS responder not started` | ERROR | The name `haquda.local` does not work. | Open the page by the IP address. |
| `startMdns: http service not added to mDNS` | ERROR | The name `haquda.local` may not work. | Open the page by the IP address. |
| `WiFi check started` | Info | The lamp began to check a new network in access point mode. | Nothing. |
| `WiFi check: ok, network saved` | Info | The check passed. The lamp restarts in about 20 seconds. | Nothing. |
| `WiFi check: wrong_password` | ERROR | The router refused the password. Nothing is saved. Other endings: `no_network`, `timeout`, `save_failed`. | See [Results of adding a network](web-interface.md#add-results). |
| `WiFi switch: scanning for the network` | Info | The lamp began to check a new network in your network. | Nothing. |
| `WiFi switch: network found, leaving the current one` | Info | The lamp found the network and leaves its own to try it. | Nothing. |
| `WiFi switch: ok, network saved first` | Info | The lamp joined the new network. It restarts in about 1 second. | Nothing. |
| `WiFi switch: no_network, staying on the current network` | ERROR | The lamp does not see the network. Nothing changes. The ending can also be `scan_failed`. | See [Results of adding a network](web-interface.md#add-results). |
| `WiFi switch: wrong_password, restarting to rejoin the saved networks` | ERROR | The lamp could not join the new network and restarts. Other reasons: `no_network`, `timeout`, `save_failed`. | Wait; both pages then show the reason. |
| `Restarting after a WiFi network change` | Info | The lamp restarts after a passed check or after a switch. | Nothing. |
| `Sensors start: CCS811 not found` | ERROR | The CCS811 does not answer. | Check the CCS811 power and I2C wiring. |
| `Sensors start: DHT22 not responding` | ERROR | The DHT22 gave no reading within 5 seconds after the sensors powered on. | Check the DHT22 wiring. |
| `Sensors start: ZH03B sends no frames` | ERROR | The ZH03B sent no valid frame within 5 seconds after the sensors powered on. | Check the ZH03B wiring and its 5 V supply. |
| `Failed to start sensors` | ERROR | At least one of the CCS811, DHT22 and ZH03B did not answer; the lines above say which. | See the random pixels above. |
| `Sensors off, retry in 5 min` | ERROR | The sensors did not start after the power cycles. | The lamp tries again by itself; check the sensor wiring. |
| `Sensors retry failed` | ERROR | A retry did not start the sensors. | As above. |
| `Failed to get eCO2 meas` | ERROR | The CCS811 delivered no eCO2 reading in the last 2 seconds. | A single line is harmless. Lines that repeat point to the CCS811 wiring. |
| `Failed to get TVOC meas` | ERROR | The CCS811 delivered no TVOC reading. | As above. |
| `Failed to get PM 2.5 meas` | ERROR | The ZH03B sent no valid frame. The lamp does not log this line during the first 12 seconds after the sensors power on (boot, the 5-minute retry), while the ZH03B warms up. | Check the ZH03B wiring and its 5 V supply. |
| `Failed to get TEMP meas` | ERROR | The DHT22 delivered no temperature. | Check the DHT22 wiring. |
| `Failed to get HUMID meas` | ERROR | The DHT22 delivered no humidity. | Check the DHT22 wiring. |

Lines about a failed reading appear again every few seconds while the failure lasts.
