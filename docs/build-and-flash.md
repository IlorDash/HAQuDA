# Building and flashing

You build the firmware in Visual Studio with the VisualGDB plugin and flash it to the ESP32 over
the USB cable. This page describes the steps for firmware {{ app_version }}.

---

## Requirements

| Item | Version |
|------|---------|
| Visual Studio | 2022 Community, with VisualGDB 5.6 |
| Arduino core | `esp32` **2.0.11** |
| Board | ESP32 Dev Module, main board Ver 1.1 |

!!! warning "Use core 2.0.11 only"
    Core 3.x removes WiFi event names that the firmware uses, and its libraries do not match the
    ones in the `Libs` folder. The build fails. Other 2.0.x versions are not tested.

---

## Install the core

1. In Visual Studio, open **Tools → VisualGDB → Manage VisualGDB Packages**.
2. Tick **Show old packages**. Visual Studio hides 2.0.11 otherwise.
3. Select the package **esp32** in version 2.0.11 and install it.

---

## Get the sources

1. Clone the repository with its submodules `Libs/NTPClient` and `Libs/WebSerial`. Visual Studio
   has no command for submodules; in a terminal, run `git clone --recurse-submodules` with the
   repository address, or `git submodule update --init` in an existing clone.
2. Open `Software/Software_ver_2/HAQuDA_Esp32/HAQuDA_Esp32.sln` in Visual Studio.

---

## Set the build variables

The firmware reads four texts from build variables. They are not in the source on purpose.

| Variable | Used for |
|----------|----------|
| `AP_ssid_uv` | Name of the lamp's access point. Also the lamp's host name in your network. |
| `AP_pass_uv` | Password of the access point, 8 to 63 characters. |
| `DDNS_Domain_uv` | DuckDNS domain. <span class="wip">WIP</span> |
| `DDNS_Token_uv` | DuckDNS token. <span class="wip">WIP</span> |

1. Open the VisualGDB project properties of `HAQuDA_Esp32` (**Project → VisualGDB Project
   Properties**).
2. Set the four user-defined variables.

VisualGDB stores your values in the file `HAQuDA_Esp32.vgdbproj.<user>.user`, which Git ignores.
Never commit real values.

!!! note "WIP"
    DuckDNS updates are not ready: the lamp sends none. The build still needs both values.

---

## Build

1. In the toolbar, select the configuration **Debug** and the platform **ESP32_Dev_Module**.
2. Choose **Build → Build Solution**.
3. Find the result in `Software/Software_ver_2/HAQuDA_Esp32/Output/ESP32_Dev_Module/Debug`.

---

## Flash

Connect the lamp to your PC with a USB cable. The main board has an FT232RL USB-to-serial chip.

| Way | Steps |
|-----|-------|
| UART | Choose **Debug → Program and Start Without Debugging**. VisualGDB uses the port `COM8` by default; select your port in the project properties if it differs. The chip resets the ESP32 into the loader by itself. |
| JTAG with OpenOCD | Connect a J-Link to the JTAG header, then choose **Debug → Start Debugging with GDB**. |

### Check the version

Open a serial terminal on the lamp's port at 115200 baud, 8N1, and restart the lamp. The ESP32
bootloader prints its own lines first. The line `HAQuDA firmware v{{ app_version }}` follows
them.

---

## Builds from GitHub Actions

GitHub Actions builds the firmware on every push and pull request that changes the firmware
folder. The build only checks that the firmware compiles. It uses the same core 2.0.11 as the
local build. The artifact `HAQuDA_esp32_esp32_esp32` holds the arduino-cli build folder: the
firmware image, the bootloader, the partition table and the `.elf` file.

The workflow fills the build variables with dummy values: the access point name `actions_ssid`,
the password `actions_pass` and fake DuckDNS values. Do not flash the artifact to a lamp you
use. Build the firmware yourself with your own values and flash that build, as described above.

### Unit tests

The repository has host unit tests in the project `HAQuDA_Tests` of the same solution. In Visual
Studio, choose **Build → Build Solution**, then open **Test → Test Explorer** and click
**Run All Tests**. They test the firmware logic on the PC, not on the lamp. GitHub Actions runs
them too, in the check
"Host tests / Test", and a failed test turns the check red.
