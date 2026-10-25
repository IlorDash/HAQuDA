# HAQuDA
Home Air Quality Display And so on

![workflow status](https://github.com/IlorDash/HAQuDA/actions/workflows/build-haquda.yml/badge.svg)

Events description:
* :white_circle::snake: Lime snake - Booting
* :green_circle: Green - Connected to WiFi

Errors description:
* :red_circle::arrow_up_small::arrow_double_up: Red grow - Failed to connect to WiFi
* :red_circle::snake: Red snake - Failed to create Access Point
* :red_circle::high_brightness::low_brightness: Red fading - Failed to start FS
* :yellow_circle::low_brightness::high_brightness: Yellow reverse fading - Failed to update NTP client
* :alarm_clock::broken_heart::snail: Random - Failed to start sensors

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

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

## Unit tests

The unit tests run on your PC. They never touch the lamp hardware.

1. Open `Software/Software_ver_2/HAQuDA_Esp32/HAQuDA_Esp32.sln` in Visual Studio 2022.
2. Choose Build -> Build Solution. Visual Studio restores the `gmock` NuGet package into `Software/Software_ver_2/HAQuDA_Esp32/packages` on the first build.
3. Open Test -> Test Explorer.
4. Click Run All Tests.

The test project is `HAQuDA_Tests` under `Software/Software_ver_2/HAQuDA_Esp32/tests`. Both the Debug and the Release solution configuration build it for x64.
If Test Explorer lists no tests, build the solution again and check that the `HAQuDA_Tests` row in Build -> Configuration Manager is ticked.
The firmware build does not include the tests.
