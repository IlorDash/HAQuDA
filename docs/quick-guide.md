# Quick guide

Three everyday tasks: start the lamp, connect it to your WiFi, and select the display mode.

---

## Start the lamp

**Before you start:** the lamp is built and flashed with firmware {{ app_version }} (see
[Building and flashing](build-and-flash.md)), and you have a 5 V power supply with a Micro-USB
plug.

1. Plug the power supply into the Micro-USB socket of the lamp.
2. Watch the matrix. A lime snake runs while the lamp starts the file system and looks for a
   stored WiFi network.
3. If a network is stored and the lamp joins it, the whole matrix turns green for about 3 seconds
   and then goes dark. The green flash starts once your router has given the lamp an address.
4. If no network is stored, the matrix shows a red growing for about 12 seconds. Then the lamp
   has opened its own access point, and an aqua fading plays once. This is normal on the first
   start. If a network is stored but the lamp cannot join it, the lime snake runs for about 30
   seconds first, and then the same red growing and aqua fading follow.
5. Wait about one minute. The lamp averages the sensor readings of the first minute before it
   draws the first colours.
6. Read the colours as described in [Display modes](display-modes.md).

!!! warning "If it fails"
    - **The whole matrix fades in red again and again and nothing else happens** — the lamp
      cannot start its file system and stops. Unplug the lamp and plug it in again. If this
      repeats, the flash memory needs attention: flash the lamp again.
    - **The matrix goes dark and stays dark, and the lamp's WiFi network usually does not
      appear** — the lamp cannot create its access point and stops. Unplug the lamp and plug it
      in again. If this repeats, check the access point password in your build (8 to 63
      characters).
    - **Pixels light up one by one in random colours after the green flash or the aqua fading,
      and the colours do not start** — the lamp cannot start its sensors. It tries again every
      5 minutes and starts the colours as soon as the sensors answer. Check the sensor wiring;
      see [LED codes and troubleshooting](troubleshooting.md).

→ [All LED effects](troubleshooting.md#start-up-effects)

---

## Connect the lamp to your WiFi

**Before you start:** the lamp runs in access point mode (no network stored, or the stored one
is out of reach), you have a phone or a laptop with WiFi, and you know the access point name and
password that were set when the firmware was built. Your own network name and password are each
at most 31 bytes long, and the network works at 2.4 GHz.

1. Unplug the lamp and plug it in again if you are not sure it runs in access point mode. Wait
   for the aqua fading.
2. On your phone or laptop, join the lamp's WiFi network with the access point name and password
   from the build.
3. Open `http://192.168.4.22/` or `http://haquda.local/` in a browser. Type the address: opening
   the page automatically is <span class="wip">WIP</span>.
4. Press **Add WiFi network** in the banner, or the link **WiFi networks**.
5. Enter your network name in **Network name (SSID)** and its password in **Password**.
6. Press **Save and restart**. The lamp checks the network first, for up to 20 seconds, and the
   page shows **Checking “network”…**. Nothing is saved until the lamp has joined the network.
7. When the check passes, the page shows **The lamp joined “network”** with two addresses and a
   countdown. About 20 seconds later the lamp restarts: the lime snake runs and the matrix flashes
   green.
8. Join your phone or laptop to your home network. The lamp's access point is gone. The page
   opens the lamp's new address by itself within 2 minutes; if it does not, open
   `http://haquda.local/`.

!!! warning "If it fails"
    - **The password for “network” was not accepted** — the router refused the password. Check it
      and press **Save and restart** again.
    - **The lamp cannot see “network”** — the lamp works only with 2.4 GHz WiFi. Choose the 2.4 GHz
      network of your router, check the name and that the router is in range, then try again.
    - **“network” did not answer in time** — check the password and the signal, then try again.
    - **The lamp joined “network” but could not save it** — try again. If it repeats, press
      **Delete all networks** and add the network again.
    - **The lamp restarts, the lime snake runs for about 30 seconds, then a red growing and the
      aqua fading follow** — the lamp could not join your network. Check the name and the
      password and that the router is in range, then add the network again.

Every message of the page is in [Results of adding a network](web-interface.md#add-results). To
add a network while the lamp already runs in your network, see
[While the lamp is in your network](web-interface.md#add-in-station-mode).

→ [Every control of the web pages](web-interface.md)

---

## Select the display mode

The lamp starts in **multi** mode.

!!! note "WIP"
    Selecting the display mode on the lamp or in the web pages is not ready yet. Until then, the
    mode is fixed in `setup()` of `sketches/HAQuDA_Esp32.ino`.

→ [Modes in detail](display-modes.md)
