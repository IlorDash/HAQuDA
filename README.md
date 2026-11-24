# HAQuDA
Home Air Quality Display And so on

![workflow status](https://github.com/IlorDash/HAQuDA/actions/workflows/build-haquda.yml/badge.svg)

![host tests status](https://github.com/IlorDash/HAQuDA/actions/workflows/host-tests.yml/badge.svg)

LED effects and what to do:

| Effect | Meaning | Action |
|--------------|---------------|-------------|
| Lime snake | The lamp starts or reconnects to a stored WiFi network. | Wait for it to end. |
| Green matrix for 3 seconds | The lamp joined your WiFi network. | Nothing. |
| Red matrix for 3 seconds, then a lime snake | The lamp lost the WiFi connection and reconnects. | Wait. |
| Red matrix for 3 seconds, then a dark matrix | You added a network while the lamp ran in your network, and it left its network to try the new one. | Wait for the restart. |
| Red matrix that fills row by row, again and again, for about 12 seconds, then an aqua fading | No network is stored or the lamp could not join it, so it opens its access point. | Join the access point and add your network on the WiFi page. The lamp tests the network first and keeps the access point on. It saves the network and restarts only when it joined, and the page then shows the lamp's new address. A wrong password or a missing network shows a message on the page, and the lamp stays in access point mode. |
| Red fading, again and again, without end | The file system does not start. The lamp stops. | Unplug the lamp and plug it in again. |
| Red matrix that fills row by row for at least 10 seconds, then one red pixel running along the matrix without end | The access point cannot be created. The lamp stops. | Unplug the lamp and plug it in again. |
| Random pixels, one by one, again and again. | The sensors do not start. The lamp does not restart: the access point, the web pages and WebSerial keep working, and the lamp tries to start the sensors again every 5 minutes. When a retry succeeds, the lamp switches to the picture you chose on the web page. While the sensors are off, Lamp light and power off from the web page replace the effect, because they need no readings. A measurement mode shows the effect again. After the sensors start, a measurement mode starts dark and draws its colours as soon as the readings it shows exist. That takes a few seconds, and for PM2.5 about 12 seconds, because the PM sensor warms up first. | Check the sensor wiring. Read the reason in the WebSerial log. The page shows which readings are missing. |
| Amber matrix for 0.5 second | You locked the touch buttons: they ignore touches. | Hold touch buttons 6 and 8 for 3 seconds to unlock them. |
| Green matrix for 0.5 second | You unlocked the touch buttons. | Nothing. |
| Red matrix for 0.5 second | The lamp refused a button action, for example Night mode while the lamp has no time. | Change the action or fix the cause. The WebSerial log names the action. |

The user guide below explains each effect in detail.

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

To get a ready firmware, open the Actions tab, select the latest successful "Build HAQuDA" run and download the artifact `HAQuDA_esp32_esp32_esp32`. It holds the merged image `HAQuDA_Esp32.merged.bin`, the separate parts, `HAQuDA_Esp32.ino.elf` for decoding backtraces, and a `flash_args` file with the offsets. You can flash the artifact to a lamp in two ways.

- Run `esptool.py --chip esp32 write_flash 0x0 HAQuDA_Esp32.merged.bin` with the lamp connected over UART.
- Open the browser tool esptool-js at https://espressif.github.io/esptool-js/ and write `HAQuDA_Esp32.merged.bin` at address 0x0. It needs desktop Chrome or Edge, because it uses Web Serial.

If the `esptool.py` command is missing, install it with `pip install esptool`. If esptool cannot connect, hold the BOOT button while it starts. If several serial ports exist, add `--port <COMx>` before `write_flash`.

To flash the separate parts instead, unzip the artifact and run `esptool.py --chip esp32 write_flash @flash_args` in that folder.

A lamp flashed this way opens its setup access point as `actions_ssid` with the password `actions_pass`. Anyone nearby can join it with this public password until you flash your own build. DuckDNS does not work, because the artifact holds dummy values. To use your own values, build the firmware in Visual Studio.

Build the firmware yourself in Visual Studio with Build -> Build Solution. Then flash over UART with Debug -> Program and Start Without Debugging. To flash over OpenOCD instead, run Debug -> Start Debugging with GDB.

## Build and flash from VS Code
Build the main clone or any of its worktrees from one VS Code window and flash the lamp over UART. All builds share one build cache in `.bench` of the main clone; only `sketches/` comes from the picked folder, everything else from the main clone.

Once:
1. Set up the build in Visual Studio in the main clone as usual (VisualGDB, ESP32 core, secret build flags).
2. Install Python 3 and run `py -3 -m pip install esp-idf-monitor`.
3. In VS Code, run the task "HAQuDA: Install picker" (Terminal -> Run Task..., or Ctrl+Shift+P -> Tasks: Run Task), then run "Developer: Reload Window". Repeat when the picker says it is outdated: the task does not start until you do.

Each time:
1. Press Ctrl+Shift+B and pick the folder in the popup.
2. While it builds, put the ESP32 into the bootloader mode (hold BOOT, press reset), if the lamp has no automatic reset.
3. The lamp is flashed and the serial monitor opens next to the build. Quit the monitor with Ctrl+] or Ctrl+C.

In VS Code, run the other tasks (Terminal -> Run Task..., or Ctrl+Shift+P -> Tasks: Run Task):

| Task | What it does |
|------|--------------|
| HAQuDA: Build + Flash + Monitor | Asks for the folder, builds, flashes, and starts the monitor. This is the Ctrl+Shift+B task. |
| HAQuDA: Build + Flash | Asks for the folder, builds and flashes. The monitor, if it runs, releases the port for the flash. |
| HAQuDA: Build | Asks for the folder and builds only. |
| HAQuDA: Flash | Flashes the last bench build without building. |
| HAQuDA: Monitor | Starts the serial monitor only. |
| HAQuDA: Change COM port | Asks for the COM port and remembers it for the flash and the monitor. |
| HAQuDA: Install picker | Installs the extension behind the popups. |
| HAQuDA: Remove bench | Asks for confirmation and deletes `.bench` safely. The next build is a full one. |

Only one build runs at a time. While it runs, a second build, Flash, Change COM port and Remove bench stop with an error; run them again when the build ends. A build also stops with an error while a flash runs.

Never share `.bench`: its build log and a link in it hold the secret build flags. To delete `.bench`, run the task HAQuDA: Remove bench. Never delete it by hand: it holds junctions to real source folders.

## Open the lamp's web page
Open `http://haquda.local/` in a browser on the same network as the lamp. The hostname `haquda` is fixed and the lamp answers to it over mDNS, also on its own access point. If your device does not resolve `.local` names, use the lamp's IP address from your router. On the access point the address is `http://192.168.4.22/`.

The main page shows the measurement tiles, the air index and a 4 x 6 grid that mirrors the picture on the lamp's LEDs: the current effect, an error effect or the measurement colours, with a caption. The page has links to the WiFi page and to the WebSerial log.

The Lamp card on the main page controls the lamp. It has a power switch, a brightness slider (5 to 100 % in steps of 5) and four modes:
- Standard shows one reading on the whole matrix. Overall shows the first reading that is out of its range, or the average colour when all are in range.
- Multi shows three readings in three bands, top to bottom.
- Night shows one reading in every third column and a night light in the other columns. The light fills up from the bottom, a stripe of three cells at a time, from the start time you set to the end time (18 cells, one more every `window / 18` minutes). Outside the window all cells are on. The mode needs the time from the internet, so on the access point the page shows it as unavailable and the lamp refuses it. A Night mode saved before a restart still applies when the lamp has no time yet (on the access point, or before the time server answers). It then shows the reading with the night light cells dark until the time is known.
- Lamp light shows one of six solid colours.

The grid next to the card shows a change at once, before the lamp answers. When you switch the lamp off, the matrix goes dark and the measurements, the web pages and the log keep running. When the lamp refuses a change, the card returns to the last accepted settings and shows the lamp's reply. The lamp accepts a change at once and puts it on the display within about a second. While the sensors do not start, Lamp light and power off work as usual and replace the sensor error picture. A measurement mode shows the error picture, and the lamp draws the mode once the sensors start. The other errors stop the lamp before the page can change anything.

The lamp keeps your settings in the file `/lamp` and reads them at the next start. A missing or damaged file gives the defaults: on, 35 % brightness, Multi mode with PM2.5 on top, eCO₂ in the middle and temperature at the bottom, night light from 21:00 to 09:00. The boot, WiFi and error pictures play over your settings, and your picture comes back when they end. At start-up, until your picture first appears, they play at the default 35 % brightness, not at your saved brightness. Every settings change is written to flash at once, before the lamp shows it, so a reset right after a change keeps it.

The four touch buttons change the lamp without the page. A touch is a release within 0.6 second of one button. The default buttons are: 2 switches the lamp on or off, 6 makes it brighter by 5 %, 9 makes it dimmer by 5 %, and 8 shows the next mode (Standard, Multi, Night, Lamp light; Night is skipped while the lamp has no time). Hold 6 or 9 to change the brightness smoothly, one step every 0.2 second; the lamp saves the brightness when you release the button. Touching two buttons together does nothing. Hold 6 and 8 together for 3 seconds to lock the buttons or unlock them again; the lamp flashes amber when it locks and green when it unlocks (see the table above). While the buttons are locked, touches do nothing and the lock chord still works. The WebSerial log shows one line for every action, for example `Touch 6: brighter`.

The WiFi page lists up to 10 saved networks, the newest first, and deletes all of them at once. It does not delete a single network. A network you add is checked before it is saved:
- On the access point the lamp tests the join and keeps the access point up. When the join works, the lamp saves the network, restarts, and the page shows the new IP address and `http://haquda.local/`, then opens it. A wrong password or a missing network shows a message, and the lamp stays on the access point.
- On a saved network the lamp first scans for the new one. When the scan does not find it (for example a 5 GHz network), the lamp stays on its network, does not restart, and the page shows the reason at once. When the scan finds it, the lamp switches to it and restarts. The page does not answer for a short time. When the join fails, the lamp returns to the old network and the pages show a failure banner. Close the banner and enter the password again.

## Web pages
The lamp serves its web pages from `Software/Web_Pages/`: `index.html`, `wifi_manager.html`, `style.css`, `app.js` and `wifi.js`. The pages use no external resources, so they work when the lamp runs as an access point without internet.

The firmware does not read these files. It contains their gzipped bytes in `Software/Software_ver_2/HAQuDA_Esp32/sketches/WebPages.h`. After you change a page source, run `py -3 Software/Web_Pages/gen_web_pages.py` from any folder and commit the changed `WebPages.h` together with the source. The script needs Python 3 and nothing else. The packed copy has the comments removed to save flash, and the sources keep them. The script stops with exit code 1 when a string, template literal, regular expression or comment in a source does not end, or when an HTML comment or a `<script>`, `<style>`, `<pre>` or `<textarea>` element is not closed. The comment stripper has unit tests: run `py -3 -m unittest discover -s Software/Web_Pages`, and the CI workflow runs them too.

To design the pages without the lamp, run `py -3 Software/Web_Pages/preview_server.py` from any folder. The script opens `http://127.0.0.1:8080/` in your browser. It serves the page sources from disk on every request, so a saved change shows after a refresh. Press Ctrl+C in the terminal to stop it.

The script answers `/api/status`, `/api/disp`, `/api/wifi`, `/api/join`, `POST /api/lamp` and the WiFi routes with mock data. `/api/disp` returns the picture the lamp shows on its LEDs: an effect, an error effect or the measurement layout. `/api/status` carries the `lamp` block and `POST /api/lamp` changes it with the firmware's rules (form fields `on`, `bright`, `mode`, `std`, `multi`, `night_param`, `night_from`, `night_to`, `color`); the settings live in the script's memory. The firmware and the lamp's access point are not needed.

Pick a mock state with `--scenario`: `normal` (default), `bad-air`, `sensor-fail`, `ap`, `sta-offline` or `errors`. The `errors` state returns the lamp's 500 replies. The `ap`, `join-ok`, `join-wrong-pass` and `join-no-network` states run as an access point: a new network is checked for about 4 seconds, then the page shows the address or the failure. `ap` and `join-ok` pass, the other two fail. The `sta-switch-ok`, `sta-switch-wrong-pass` and `sta-switch-no-network` states run as a lamp joined to a saved network: a new network goes through the scan, a switching notice and about 10 seconds with no answer at all, then the lamp comes back and `/api/join` reports the outcome in `last`. `sta-switch-ok` joins the new network and makes it the first saved one, `sta-switch-wrong-pass` returns to the old network and the pages show the failure banner, and `sta-switch-no-network` fails at the scan at once and the lamp keeps answering. The other states answer a new network like `sta-switch-ok`. The `disp-effects`, `disp-errors` and `disp-night` states change the picture of `/api/disp` over time: the five effects for 25 seconds each, the five error effects for 15 seconds each, and the night light growing by one cell every 4 seconds. The `lamp-refuses` state answers every `POST /api/lamp` with a 400, so the card shows the lamp's reply and returns to the accepted settings. The `no-lamp` state leaves out the `lamp` block, so the page hides the card. The access point states report no time, so the card shows the night mode as unavailable. Add `--latency 800` to delay every answer by 800 ms. Use `--port` to change the port and `--no-browser` to keep the browser closed.

Saved networks live only in the script's memory, and nothing restarts: the script only stops answering for a while. The `WebSerial` link opens a stub page, because WebSerial runs only on the lamp.

The `Web pages` workflow runs `python3 Software/Web_Pages/gen_web_pages.py --check`. The check fails when `WebPages.h` does not match the sources, so a forgotten regeneration shows up in the pull request. The workflow also runs the unit tests and syntax-checks `preview_server.py`.

## Commit messages and formatting
Commit hooks check every message and the formatting of the lines you change. Set them up once per clone:
1. Install Python 3 and run `pip install pre-commit`;
2. Open View -> Terminal in Visual Studio, go to the repository root and run `pre-commit install`.

If `pre-commit` is not recognized, run `py -m pre_commit install` instead. The first commit after the install downloads gitlint and clang-format, so it needs internet access.

The commit title has the form `area: Summary`, for example `MeasHandler: Fix averaging of empty readings`. The `area` is the module name without the `HAQuDA_` prefix, or a directory such as `sketches` or `ci`. Commits that change only `README.md` use `README: Summary`. The title is at most 65 characters and has no trailing punctuation. Body lines are at most 79 characters, and a blank line separates the title from the body. Lines with a URL may be longer. When the message breaks a rule, the commit fails. Fix the message and commit again.

The clang-format hook checks only the lines you changed in `.h`, `.cpp` and `.ino` files under `Software/`. It does not reformat whole files and skips `Libs/` and `Core/`. The style comes from `Software/.clang-format`. When the hook fails, it shows the lines to change. Format them, stage the files again and commit again. Edit -> Advanced -> Format Selection uses `Software/.clang-format` only when ClangFormat support is enabled under Tools -> Options -> Text Editor -> C/C++ -> Code Style -> Formatting.

Git runs the hooks for every commit, including commits from the Git Changes window.

## User guide

The user guide is in English and Russian. It describes the first start, the WiFi setup, the display modes, the web pages and the LED effects. The site at https://ilordash.github.io/HAQuDA/ goes live with the next release, when `Software_Ver_2` is merged into `main`. Until then, build the guide on your PC and read it there.

To publish the site, set the source once in the repository: Settings -> Pages -> Source -> "GitHub Actions".

To build the guide on your PC:
1. Create a virtual environment: `py -3 -m venv .venv`;
2. Install the packages: `.venv\Scripts\python.exe -m pip install -r docs/requirements.txt`;
3. Start the preview at http://127.0.0.1:8000: `.venv\Scripts\python.exe -m mkdocs serve`.

Run `.venv\Scripts\python.exe -m mkdocs build --strict` before you open a pull request. The build fails on a broken link.

## Unit tests

The unit tests run on your PC. They never touch the lamp hardware.

1. Open `Software/Software_ver_2/HAQuDA_Esp32/HAQuDA_Esp32.sln` in Visual Studio 2022.
2. Choose Build -> Build Solution. Visual Studio restores the `gmock` NuGet package into `Software/Software_ver_2/HAQuDA_Esp32/packages` on the first build.
3. Open Test -> Test Explorer.
4. Click Run All Tests.

The test project is `HAQuDA_Tests` under `Software/Software_ver_2/HAQuDA_Esp32/tests`. Both the Debug and the Release solution configuration build it for x64.
If Test Explorer lists no tests, build the solution again and check that the `HAQuDA_Tests` row in Build -> Configuration Manager is ticked.
The firmware build does not include the tests.
