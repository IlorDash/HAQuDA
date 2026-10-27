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
| Dark matrix for about 20 seconds, then an aqua fading | No network is stored or the lamp could not join it, so it opens its access point. | Join the access point and add your network. |
| Random pixels, one by one, without end | The file system does not start. The lamp stops. Known bug: the intended effect is a red fading; the firmware swaps the effects of the file system and sensor errors. | Unplug the lamp and plug it in again. |
| Dark matrix that stays dark, the access point usually does not appear | The access point cannot be created. The lamp stops. | Unplug the lamp and plug it in again. |
| Dark matrix for about 15–20 seconds after the green flash or the aqua fading, then the lime snake again and again | The sensors do not start. The lamp restarts itself. | Check the sensor wiring. |

The user guide below explains each effect in detail.

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

To get a ready firmware, open the Actions tab, select the latest successful "Build HAQuDA" run and download the artifact `HAQuDA_esp32_esp32_esp32`. It holds the merged image `HAQuDA_Esp32.merged.bin`, the separate parts, `HAQuDA_Esp32.ino.elf` for decoding backtraces, and a `flash_args` file with the offsets. You can flash the artifact to a lamp in two ways.

- Run `esptool.py --chip esp32 write_flash 0x0 HAQuDA_Esp32.merged.bin` with the lamp connected over UART.
- Open the browser tool esptool-js at https://espressif.github.io/esptool-js/ and write `HAQuDA_Esp32.merged.bin` at address 0x0. It needs desktop Chrome or Edge, because it uses Web Serial.

If the `esptool.py` command is missing, install it with `pip install esptool`. If esptool cannot connect, hold the BOOT button while it starts. If several serial ports exist, add `--port <COMx>` before `write_flash`.

To flash the separate parts instead, unzip the artifact and run `esptool.py --chip esp32 write_flash @flash_args` in that folder.

A lamp flashed this way opens its setup access point as `actions_ssid` with the password `actions_pass`. Anyone nearby can join it with this public password until you flash your own build. DuckDNS does not work, because the artifact holds dummy values. To use your own values, build the firmware in Visual Studio.

Build the firmware yourself in Visual Studio with Build -> Build Solution. Then flash over UART with Debug -> Program and Start Without Debugging. To flash over OpenOCD instead, run Debug -> Start Debugging with GDB.

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
