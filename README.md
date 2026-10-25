# HAQuDA
Home Air Quality Display And so on

![workflow status](https://github.com/IlorDash/HAQuDA/actions/workflows/build-haquda.yml/badge.svg)

LED effects and what to do:

| Effect | Meaning | Action |
|--------------|---------------|-------------|
| Lime snake | The lamp starts or reconnects to a stored WiFi network. | Wait for it to end. |
| Green matrix for 3 seconds | The lamp joined your WiFi network. | Nothing. |
| Red matrix for 3 seconds, then a lime snake | The lamp lost the WiFi connection and reconnects. | Wait. |
| Dark matrix for about 20 seconds, then an aqua fading | No network is stored or the lamp could not join it, so it opens its access point. | Join the access point and add your network. |
| Random pixels, one by one, without end | The file system does not start. The lamp stops. | Unplug the lamp and plug it in again. |
| Dark matrix that stays dark, the access point usually does not appear | The access point cannot be created. The lamp stops. | Unplug the lamp and plug it in again. |
| Dark matrix for about 15–20 seconds after the green flash or the aqua fading, then the lime snake again and again | The sensors do not start. The lamp restarts itself. | Check the sensor wiring. |

The user guide below explains each effect in detail.

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

Do not flash the builds that GitHub Actions produces: the workflow installs the latest Arduino core instead of 2.0.11. Build the firmware yourself.

## User guide

The user guide is in English and Russian. It describes the first start, the WiFi setup, the display modes, the web pages and the LED effects. The site at https://ilordash.github.io/HAQuDA/ goes live with the next release, when `Software_Ver_2` is merged into `main`. Until then, build the guide on your PC and read it there.

To publish the site, set the source once in the repository: Settings -> Pages -> Source -> "GitHub Actions".

To build the guide on your PC:
1. Create a virtual environment: `py -3 -m venv .venv`;
2. Install the packages: `.venv\Scripts\python.exe -m pip install -r docs/requirements.txt`;
3. Start the preview at http://127.0.0.1:8000: `.venv\Scripts\python.exe -m mkdocs serve`.

Run `.venv\Scripts\python.exe -m mkdocs build --strict` before you open a pull request. The build fails on a broken link.
