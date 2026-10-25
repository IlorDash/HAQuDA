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
| Random pixels, one by one, without end | The file system does not start. The lamp stops. Known bug: the intended effect is a red fading; the firmware swaps the effects of the file system and sensor errors. | Unplug the lamp and plug it in again. |
| Dark matrix that stays dark, the access point usually does not appear | The access point cannot be created. The lamp stops. | Unplug the lamp and plug it in again. |
| Dark matrix for about 15–20 seconds after the green flash or the aqua fading, then the lime snake again and again | The sensors do not start. The lamp restarts itself. | Check the sensor wiring. |

The user guide below explains each effect in detail.

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

CI and the VisualGDB build both use esp32 core 2.0.11.

There are CI builds, that can be flashed into your own ESP32.
To do this if you are using OpenOCD:
1. Download last success build artifacts from GitHub Actions page;
2. Copy dowloaded artifacts to HAQuDA/Software/Software_ver_2/HAQuDA_Esp32/output;
3. Open HAQuDA project in Visual Studio with VisualGDB plugin;
4. In Visual Studio run Debug -> Start Debugging with GDB.

If you are using UART, repeat steps 1-3 from above and then:
* In Visual Studio run Debug -> Program and Start Without Debugging.

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
