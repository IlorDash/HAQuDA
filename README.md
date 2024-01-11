# HAQuDA
Home Air Quality Display And so on

![workflow status](https://github.com/IlorDash/HAQuDA/actions/workflows/build-haquda.yml/badge.svg)

Software is an Arduino project created using VisualGDB 5.6. Currently **we support esp32 2.0.11 version** of Arduino core, so if you encounter problems with unsuported methods or something like that, **check version of this package**.

There are CI builds, that can be flashed into your own ESP32.
To do this if you are using OpenOCD:
1. Download last success build artifacts from GitHub Actions page;
2. Copy dowloaded artifacts to HAQuDA/Software/Software_ver_2/HAQuDA_Esp32/output;
3. Open HAQuDA project in Visual Studio with VisualGDB plugin;
4. In Visual Studio run Debug -> Start Debugging with GDB.

If you are using UART, repeat steps 1-3 from above and then:
* In Visual Studio run Debug -> Program and Start Without Debugging.