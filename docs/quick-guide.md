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
   and then goes dark.
4. If no network is stored, or the lamp cannot join it, the matrix goes dark for about twenty
   seconds. Then the lamp has opened its own access point, and an aqua fading plays once. This
   is normal on the first start.
5. Wait about one minute. The lamp averages the sensor readings of the first minute before it
   draws the first colours.
6. Read the colours as described in [Display modes](display-modes.md).

!!! warning "If it fails"
    - **Pixels light up one by one in random colours and nothing else happens** — the lamp
      cannot start its file system and stops. Known bug: the intended effect is a red fading;
      the firmware swaps the effects of the file system and sensor errors. Unplug the lamp and plug it in again. If this
      repeats, the flash memory needs attention: flash the lamp again.
    - **The matrix goes dark and stays dark, and the lamp's WiFi network usually does not
      appear** — the lamp cannot create its access point and stops. Unplug the lamp and plug it
      in again. If this repeats, check the access point password in your build (8 to 63
      characters).
    - **The matrix goes dark for about 15–20 seconds after the green flash or the aqua fading,
      then the lime snake starts again, again and again** — the lamp cannot start its sensors and
      restarts itself. Check the sensor wiring; see
      [LED codes and troubleshooting](troubleshooting.md).

→ [All LED effects](troubleshooting.md#start-up-effects)

---

## Connect the lamp to your WiFi

**Before you start:** the lamp runs in access point mode (no network stored, or the stored one
is out of reach), you have a phone or a laptop with WiFi, and you know the access point name and
password that were set when the firmware was built. Your own network name and password are each
at most 31 characters long.

1. Unplug the lamp and plug it in again if you are not sure it runs in access point mode. Wait
   for the aqua fading.
2. On your phone or laptop, join the lamp's WiFi network with the access point name and password
   from the build.
3. Open `http://192.168.4.22/` in a browser. Type the address: opening the page automatically is
   <span class="wip">WIP</span>.
4. If the lamp already stores a network, for example a wrong one, press **Delete** first. The
   lamp stores one network at a time <span class="wip">WIP</span>.
5. Press the link **Add WiFi credentials**.
6. Enter your network name in **SSID** and its password in **Password**. Leave **IP Address** and
   **Gateway Address** as they are; the lamp ignores both <span class="wip">WIP</span>.
7. Press **Submit**.
8. The lamp restarts about 3 seconds after it accepts the network. The browser can show
   **Success** before that. Wait until the lime snake ends and the matrix flashes green.
9. Join your phone or laptop to your home network again. The lamp's access point is gone.

!!! note "WIP"
    The lamp does not report its address in your network. Find it in the client list of your
    router, under the name of the lamp's access point.

!!! warning "If it fails"
    - **Error check WiFi creds** — the SSID or the password field is empty. Press **Go back**,
      which opens the main page, press **Add WiFi credentials** again and fill in both fields.
    - **Error reading stored WiFi credentials** — a network is already stored. Press **Delete**
      on the main page, then add your network again.
    - **Error saving stored WiFi credentials** — the lamp could not write the file. Try once
      more; if it repeats, unplug the lamp and plug it in again.
    - **The lamp restarts but the matrix goes dark for about twenty seconds again** — it could
      not join your network. Check the name and the password and that the router is in range,
      then add the network again.

→ [Every control of the web pages](web-interface.md)

---

## Select the display mode

The lamp starts in **multi** mode.

!!! note "WIP"
    Selecting the display mode on the lamp or in the web pages is not ready yet. Until then, the
    mode is fixed in `setup()` of `sketches/HAQuDA_Esp32.ino`.

→ [Modes in detail](display-modes.md)
