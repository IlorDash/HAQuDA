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
| Lime snake | The lamp starts and joins a stored WiFi network. A stored network that does not answer keeps the snake for about 30 seconds. | Wait for the result: the green matrix, the red growing or the aqua fading. |
| Green, whole matrix, 5 seconds | The lamp joined your WiFi network and got an address from your router, at the start or after a lost connection. | Nothing. |
| Lime snake after the green matrix | The lamp is in your network and waits for the first readings of the mode you chose. It plays again when the sensors start after a failed start. | Wait. The snake ends when the readings exist, a few seconds after the start. PM2.5 needs about 12 seconds, because the ZH03B warms up first; eCO2 and TVOC wait until the CCS811 reports at least 400 ppm. **Lamp light** and power off from the web page end it at once. |
| Red growing for 5 seconds, then an aqua fading again and again. Before the red growing the lime snake runs briefly, or for about 30 seconds when a network is stored | No network is stored, or the lamp could not join the stored one. It opens its access point. The aqua fading goes from full aqua to dark and stays until the readings of the mode exist, or until you choose **Lamp light** or power off. | Nothing on the first start. Otherwise check the name and the password of your network and add it again ([Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi)). To set the lamp up, join its network and open `http://192.168.4.22/`. |
| Red growing, then one red pixel that runs along the matrix without end; the LED on the ESP32 board does not blink | The lamp could not create its access point and stops. Its WiFi network usually does not appear. | Unplug the lamp and plug it in again. If it repeats, check the access point password in your build (8 to 63 characters). |
| Random pixels, one by one, again and again | The lamp could not start its sensors (the CCS811, the DHT22 or the ZH03B does not answer). It tries up to three times to power the sensors off and on, keeps the effect, and tries again every 5 minutes. Each round of tries blocks the lamp, up to about 27 seconds when the DHT22 or the ZH03B is missing, so the web pages answer slowly meanwhile. When a retry succeeds, the matrix waits for the first readings (the lime snake or the aqua fading above) and then shows the mode you chose. | Check the sensor power and the wiring; the log names the missing sensor (see [Log messages](#log-messages)). **Lamp light** and power off from the web page replace the effect, because they need no readings; a measurement mode shows it again. |
| Red fading, again and again; it does not stop | The lamp could not start its file system and stops. | Unplug the lamp and plug it in again. If it repeats, flash the lamp again. |

---

## Running effects

| Effect | Cause | Action |
|------------------------|-------|------------|
| Red, whole matrix, 10 seconds, then a lime snake until the lamp has joined, then green, whole matrix, 5 seconds | The lamp lost the WiFi connection and reconnects. A connection that returns within the 10 seconds still shows the red matrix to its end, and the green matrix follows. | Wait. |
| Lime snake until the restart | You added a network while the lamp ran in your network, and the lamp found it: it leaves its network to try the new one. When the lamp does not find the network, the picture does not change. | Wait for the restart; see [While the lamp is in your network](web-interface.md#add-in-station-mode). |
| Lime snake that does not end after the lamp lost its WiFi <span class="wip">WIP</span> | The router is gone. The lamp retries without a limit. | Bring the router back, or unplug the lamp and plug it in again. |
| Yellow fading from dark to full yellow for 10 seconds, every minute | The lamp is in your network, but it has not received the time from the internet since the start. It asks again about once a minute. After the 10 seconds your picture shows for 50 seconds (the measurements, **Lamp light**, the dark matrix when the lamp is off, or the lime snake while the lamp waits for readings), and so on until the first answer. When another problem is active too, the two take turns as in the last row, and your picture does not come in between. A new WiFi or error picture shows at once, also during the 50 seconds. After the first answer a lost time server shows nothing: the lamp keeps the last time. | Check that your router has internet access. The fading ends by itself at the first answer. The night mode needs this time. |
| Two or more of the pictures above that have no end of their own, for example the random pixels and the aqua fading | Several problems are active at once. They take turns, every picture for at least 10 seconds (5 seconds for the lime snake and the aqua fading), in the order they started. A new error shows at once over any playing effect, the green matrix included, and the interrupted effect comes back later. A new error waits until the error on the matrix has shown for 10 seconds. The red pixel and the red fading stop the lamp and play without taking turns. | Fix each problem as described in its row. |

!!! note "WIP"
    Recovery after a WiFi loss is not ready: the lamp does not open its access point and retries
    without a limit. Once it rejoins, the matrix shows green for 5 seconds and the colours
    return. If the router stays away, unplug the lamp and plug it in again.

---

## The matrix looks wrong

| Effect | Cause | Action |
|------------------------|-------|------------|
| The lime snake or the aqua fading keeps playing instead of colours | The lamp draws once every value of the mode has a reading. PM2.5 needs a 12-second warm-up, and eCO2 and TVOC wait until the CCS811 reports at least 400 ppm. | Wait. If it lasts minutes, look for `Failed to get ... meas` lines in the log (see [Log messages](#log-messages)). |
| The matrix is dark and the web page shows the lamp as off | You switched the lamp off in the Lamp card. | Press the power switch ([The Lamp card](web-interface.md#lamp-card)). |
| The picture does not change for several minutes | In a measurement mode, one of the shown values delivers no reading, and the lamp keeps the old picture. One missed CCS811 reading discards the whole minute of eCO2 and TVOC. | Open the log and look for `Failed to get ... meas` lines (see [Log messages](#log-messages)). Check the wiring of that sensor. |
| eCO2 and TVOC never show green | The lamp shows these two values in steps. | Nothing; see [Colour scale](display-modes.md#colour-scale). |
| An animation, for example the lime snake, freezes for a fraction of a second when WiFi connects | The WiFi connection and the matrix share one processor core, so the picture waits while WiFi starts. | Nothing. It is harmless. Parts of the matrix no longer flash random colours during file system writes or a WiFi connection. |
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
| `Lamp settings loaded` | Info | The lamp read its saved settings from `/lamp`. | Nothing. |
| `Lamp settings bad, defaults` | ERROR | The file `/lamp` is damaged or of another format. The lamp uses the defaults. | Set your choice again on the web page; the lamp overwrites the file. |
| `Lamp settings: save failed` | ERROR | The lamp could not write `/lamp`. The change works until the next restart. | Make the change again. If it repeats, flash the lamp again. |
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
| `NTP not synced` | ERROR | The lamp is in your network and has not received the time since the start. It logs the line once. | See the yellow fading above. |

Lines about a failed reading appear again every few seconds while the failure lasts.
