# Web interface

The lamp runs a small web server as soon as it has joined your WiFi network or opened its own
access point. The pages come in English and Russian: the buttons **EN** and **RU** in the header
switch the language (the first time, the page takes the language of your browser), and the round
button next to them switches the light and the dark theme. The browser remembers both choices.

!!! note "The pages have no password"
    Anyone who can reach the lamp in the browser can add or delete its WiFi networks. The lamp
    receives your WiFi password over plain HTTP. Use the pages on a network you trust.

---

## Page addresses

| Lamp mode | Address |
|-----------|---------|
| Access point | `http://192.168.4.22/` or `http://haquda.local/` after you join the lamp's own WiFi network. Type the address: opening the page automatically is <span class="wip">WIP</span>. |
| Joined to your network | `http://haquda.local/`. If your device does not resolve the name, use the address your router gave the lamp: the main page shows it in its footer, and your router lists the lamp under the name `haquda`. |

| Address | Content |
|---------|---------------|
| `/` | The main page. |
| `/wifi_manager` | The page that lists, adds and deletes WiFi networks. |
| `/buttons` | The page that sets what the touch buttons do. |
| `/webserial` | The live log of the lamp. Works in access point mode too. |

In your network, any other address answers with an empty 404 page. In access point mode, any other
address returns the main page.

---

## Main page

The page asks the lamp for new values every 5 seconds and loads nothing from the internet, so it
works in access point mode. From top to bottom it shows the header, the lamp's picture with the air
index, the Lamp card, the tiles and the links.

### Header and banners

The header shows signal bars and one status line.

| Status line | Meaning |
|-------------|---------|
| **Connecting to the lamp…** | The page has had no answer yet. |
| **Connected to** *network* **· -60 dBm** | The lamp is in your network. Four bars above -55 dBm, three above -65, two above -75, one below. |
| **Access point · no internet** | The lamp runs its own access point. |
| **WiFi connection lost** | The lamp is in station mode and has no connection. |
| **Lamp not responding · retry in 5 s** | The page gets no answer from the lamp. |
| **Lamp is restarting…** | You pressed **Restart lamp** and the lamp accepted it; see [Restart the lamp](#restart-lamp). It replaces the line above. |

A banner appears under the header when there is something to do.

| Banner | Cause | Action |
|--------|-------|------------|
| **The lamp couldn't find a saved network and started its own. Add your home WiFi.** and the button **Add WiFi network** | The lamp runs its access point. | Press the button; see [Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi). |
| **No response from the lamp. ...** | The lamp restarts, is off, or your phone changed networks. | Wait. The page keeps the last values, dims them and recovers by itself. |
| **You opened the lamp’s own network with the touch buttons. Your saved networks are kept — restart the lamp to return to them.** and the button **Add WiFi network** | You held the touch buttons TOUCH 2 and TOUCH 8 for 10 seconds; see [Use the touch buttons](quick-guide.md#use-the-touch-buttons). | Add a network, or press **Restart lamp**: the lamp then joins its saved networks. |
| Red banner **Could not join “network”: reason. The lamp is back on “network”.** and the button **Close** | The lamp tried a new network in station mode and failed; see [While the lamp is in your network](#add-in-station-mode). | Read the reason. **Close** hides the banner, and the lamp forgets the outcome. |

### The lamp's picture {#lamp-picture}

The grid of 4 × 6 cells mirrors the lamp's matrix. The page asks the lamp for its picture every
second and starts an animation in step with the lamp.

| The lamp shows | The grid shows |
|----------------|----------------|
| A measurement picture: multi, standard or night mode | The same layout in the colours the lamp drew. The colours change when the lamp redraws, about once a minute, not with every reading. |
| Snake | A snake runs along the grid, one lap as long as on the lamp. |
| Growing | The rows fill from the top down. |
| Random pixels | The cells light up one by one in random colours; every cycle takes a new order. The order is the page's own. |
| Fading | The grid fades in the colour of the effect with the lamp's timing. |
| One colour | The whole grid in that colour. |
| Nothing new: it keeps its last frame | The grid keeps its last picture and stops the animation. |

With "reduce motion" set in your device, the grid shows still frames: the fading at its start level,
the snake at the start of its path, the growing and the random pixels as the full grid. Until the
lamp has answered, and after three missed answers in a row, the grid shows three bands of the
readings instead: PM2.5 above, eCO₂ in the middle, temperature below. When the lamp does not answer
at all, the grid goes dark.

While the lamp shows your picture, the legend beside the grid lists the readings of the current
mode, or **Lamp light** with the colour name, or **Lamp is off**. While the grid shows an effect or
an error, the legend gives a caption instead.

| Caption | The lamp is |
|---------|-------------|
| **Starting or connecting to WiFi…** | Running the lime snake: start or reconnect. |
| **Access point mode: add your WiFi** | Fading in access point mode. |
| **Connected to WiFi** | Flashing green after it joined your network. |
| **WiFi connection lost** | Flashing red after it lost the network. |
| **The lamp is showing an effect, not readings** | Showing any other effect, for example the random pixels. |
| **Can't connect to WiFi**, **Can't start the access point**, **Storage error**, **Sensor error**, **Can't get the time** | Signalling this error; see [LED codes](troubleshooting.md#start-up-effects). |

### The Lamp card {#lamp-card}

The card controls the lamp. It is hidden when the lamp sends no lamp settings.

| Control | Action |
|---------|--------|
| Power switch | Switches the lamp on and off. When it is off, the matrix goes dark, and the other controls of the card are locked. The measurements, the web pages and the log keep running. |
| **Brightness** | Slider from 5 to 100 % in steps of 5. The grid follows the slider while you drag it, and the page sends the value when you release it. |
| **Standard** | One reading on the whole matrix. Pick it from the chips: **Overall**, **Temperature**, **Humidity**, **eCO₂**, **TVOC** or **PM2.5**. **Overall** shows the colour of the first reading out of its range, in the order temperature, humidity, eCO₂, TVOC, PM2.5, or the average colour of all readings when none is out of range. |
| **Multi** | Three readings in three bands, **Top**, **Middle** and **Bottom**, each from a list. |
| **Night** | One reading from the list **Reading** and the night light, set with the times **Night light from** and **to** (`HH:MM`). The button reads **needs internet** and stays locked while the lamp has no time. |
| **Lamp light** | One of six solid colours: **Warm white**, **Neutral**, **Cool white**, **Amber**, **Lavender**, **Teal**. |

The grid shows a change at once, before the lamp answers; the lamp draws it within about a
second. When the lamp refuses a change, the card returns to the last settings the lamp accepted and
shows the lamp's reply in red for about 10 seconds. The replies are in English:

| Reply | Cause |
|-------|-------|
| **The night mode needs the time from the internet.** | The lamp has no time, for example on its access point. |
| **The night light must start and end at different times.** | **Night light from** and **to** are equal. |
| **The lamp did not accept the change.** or **No answer from the lamp.** | The page got no usable answer. |

When the lamp could not write a change to its memory, it goes back to the last saved settings and
the card shows them. A window opens: **Couldn’t save**, **The lamp couldn’t write the settings to
its memory. It went back to the last saved settings.** and the button **OK**. The lamp does not
try the write again; make the change once more.

While the lamp does not answer, the card shows **Controls are unavailable while the lamp isn’t
responding**. How the modes look on the lamp is in [Display modes](display-modes.md#multi-mode).

### Readings and air index

| Element | Content |
|---------|---------|
| Tiles | **Temperature**, **Humidity**, **eCO₂**, **TVOC** and **PM2.5**, each with its value, its unit and a square in the colour of the [colour scale](display-modes.md#colour-scale). **no data** replaces a value the lamp has not got. |
| Air index | **Good air**, **Moderate** or **Poor air**, from the worst colour of the tiles: green to blue, yellow, orange to red. The line below reads **All readings are normal** or **Worst:** and the name of the reading. **Sensor readings failed:** and the names replace it when a reading is missing. **No data** shows before the first readings. |
| Footer | The firmware version, the IP address and the uptime of the lamp. |

### Links and switches

| Control | Action |
|---------|--------------|
| Link **WiFi networks** | Opens the [WiFi networks page](#wifi-networks-page). |
| Link **Live log** | Opens the [log page](#log-page). |
| Link **Touch buttons** | Opens the [touch buttons page](#touch-buttons-page). |
| Links **User guide ↗** and **GitHub ↗** | Open sites on the internet; the notes **needs internet** mark them. |
| Red button **Restart lamp** | Restarts the lamp; see [Restart the lamp](#restart-lamp). |
| **EN**, **RU** and the round theme button | Switch the language and the theme. |

### Restart the lamp {#restart-lamp}

The red button **Restart lamp** is the last item of the links. It restarts the lamp without
unplugging it.

1. The page asks **Restart the lamp? It will be unavailable for up to a minute.** Confirm, or
   cancel to change nothing.
2. The lamp accepts the request and restarts about 1 second later. A lamp setting you changed just
   before is saved first. The button is locked while the request is out and while the restart is
   under way.
3. The status line reads **Lamp is restarting…**. The page asks the lamp every 2 seconds and
   reloads itself as soon as the lamp is back.
4. If the lamp does not answer for 90 seconds, a banner reads **The lamp has not answered for a
   minute and a half. Open this page again in a moment. In access point mode your phone may need to
   rejoin the lamp’s WiFi network first.**

When the lamp refuses, the banner shows its reply for about 10 seconds. The replies are in English:

| Reply | Cause |
|-------|-------|
| **The lamp is already restarting.** | A restart is already coming, for example after a passed WiFi check. |
| **The lamp is checking a WiFi network. Try again when the check is over.** | The lamp checks or switches to a new network, and a restart would break it. |
| **No answer from the lamp.** | The page got no answer to the request. |

While a restart is coming, the lamp refuses a new network with **The lamp is restarting. ...**; see
[Results of adding a network](#add-results).

---

## WiFi networks page {#wifi-networks-page}

Open it with the link **WiFi networks** or at `/wifi_manager`. The arrow in the header leads back
to the main page. In access point mode a banner at the top reads **You're on the lamp's own
network. Add your home WiFi — the lamp will restart and join it.**

### Add a network {#add-a-network}

| Field | Input |
|-------|---------------|
| **Network name (SSID)** | The name of your WiFi network, at most 31 bytes. The counter shows the bytes used. Emoji and non-Latin letters take 2 to 4 bytes each. |
| **Password** | The password of your network, at most 31 bytes. **Show** and **Hide** reveal and hide it. |
| **Save and restart** | Sends the network to the lamp. The button reads **Update password** when the name is already saved. It stays blocked while a field is over 31 bytes. |

The lamp works only with 2.4 GHz WiFi. It checks the network before it saves it, so a wrong
password or name costs you nothing: the lamp saves the network only after it has joined it. A
saved network goes to the first place of the list, and the lamp tries it first after a restart. How
the check runs depends on where the page is open.

#### In access point mode

1. The page shows **Checking “network”…**. The lamp keeps its access point and tries the network
   for up to 20 seconds. It fails early when the router refuses the password twice in a row, or
   when it does not find the network twice in a row.
2. If the lamp joins, it saves the network and the page shows **The lamp joined “network”**. Two
   addresses appear, **By IP address** and **By name** (`haquda.local`), each with a **Copy**
   button and a countdown such as **available in 12 s**.
3. The lamp restarts about 20 seconds after the check and joins the network.
4. Connect your phone or laptop to that network. The page tries both addresses every 3 seconds, up
   to 2 minutes, and opens the first that answers (the IP address first), once the lamp has
   restarted. If none answers, the page tells you to open `http://haquda.local/` by hand.

If the check fails, nothing is saved, the lamp keeps its access point, and the page shows the
reason with the form still filled in; see [Results of adding a network](#add-results).

#### While the lamp is in your network {#add-in-station-mode}

1. The lamp looks for the network without leaving its own. If it does not see it, the page shows
   the reason at once. Nothing changes and nothing restarts.
2. Otherwise the page warns that the lamp goes away: **The lamp leaves “network” to try “network”;
   this page loses it for about a minute.**
3. The lamp leaves its network and tries the new one for up to 20 seconds. The matrix can flash red
   and go dark meanwhile.
4. If the lamp joins, it saves the network as the first one and restarts. The page shows **The lamp
   joined “network”**. If the page stops answering, connect your phone to the new network and open
   `http://haquda.local/`.
5. If the lamp does not join, it saves nothing and restarts, then rejoins its saved networks. After
   the restart both pages show the red banner with the reason. The rejoin can take up to a few
   minutes when several saved networks are out of reach.

!!! note "The lamp goes away while it tries a network"
    From the warning until the restart the lamp does not answer on its old address. The page waits
    for it up to 3 minutes and then says the lamp has probably joined the new network.

#### Results of adding a network {#add-results}

| The page says | Cause | Action |
|---------------|-------|------------|
| **The password for “network” was not accepted. Check it and try again.** | The router refused the password. | Check the password and try again. |
| **The lamp cannot see “network”. The lamp works only with 2.4 GHz WiFi: ...** | The network was not found: wrong name, router off or out of range, or a 5 GHz network. | Choose the 2.4 GHz network of the router or turn 2.4 GHz on in the router; check the name; try again. |
| **“network” did not answer in time. Check the password and the signal, then try again.** | The lamp got no IP address within 20 seconds. | Check the password and the signal, try again. |
| **The lamp joined “network” but could not save it. ...** | The network works, but the lamp could not write its list. | Try again. If it repeats, delete all networks and add this one again. |
| **The lamp could not search for networks, so it did not try “network”. Try again.** | The scan for the network did not finish (station mode). | Try again. |
| **The lamp is back, but it did not report how the switch to “network” ended. ...** | The lamp restarted and has no outcome to report. It is probably in its previous network. | Try again. |
| **The lamp has probably joined “network” and restarted. ...** | The page lost the lamp and it does not answer any more. | Connect your phone to that network and open `http://haquda.local/`. If the page does not open, join the lamp's access point and try again. |
| **The lamp is already checking a network. Wait for the result.** | A check runs already. | Wait for the result. |
| **The network is saved. The lamp is restarting.** or **The lamp is restarting. Open this page again in a minute.** | The check has passed, or a switch has ended. | Wait for the restart. |

### Saved networks {#saved-networks}

The list **Saved networks** holds up to 10 networks, newest first. The row that matches the name in
the form shows **will update**. When 10 networks are saved, the form names the oldest one: **10
networks saved: adding this one removes “network”.** The lamp then saves the new network and drops
that one.

| Control | Action |
|---------|--------------|
| **Delete all networks** | Asks **Delete all N networks? After its next restart the lamp starts its own access point.** **Cancel** closes the question; **Delete all** deletes every saved network. The lamp stays connected until the next start; after it, it opens its access point. |

The lamp answers **Deleted all saved networks.**, or **There are no saved networks.** when the list
is empty. **Error deleting the saved networks.** means the lamp could not delete the file: try
again. The page cannot delete a single network.

---

## Touch buttons page {#touch-buttons-page}

Open it with the link **Touch buttons** on the main page or on the WiFi networks page, or at
`/buttons`. The arrow in the header leads back to the main page. The page asks the lamp every
5 seconds, and a change applies at once. While the lamp does not answer, the page dims, and its
footer reads **Settings are unavailable while the lamp isn’t responding**. How to use the buttons
on the lamp is in the [Quick guide](quick-guide.md#use-the-touch-buttons).

The page shows the lock card on top, a drawing of the four buttons **TOUCH 2**, **TOUCH 6**,
**TOUCH 8** and **TOUCH 9**, and the card of the chosen button. Each button in the drawing carries
the name of its action. Tap a button in the drawing, then choose its action in the card.

### Actions {#button-actions}

| Action | What the button does on a tap |
|--------|-------------------------------|
| **Nothing** | Nothing. |
| **Turn lamp on / off** | Switches the lamp on and off. |
| **Brighter** | Makes the lamp brighter by 5 %. A hold changes the brightness smoothly; see [System gestures](#system-gestures). |
| **Dimmer** | Makes the lamp dimmer by 5 %. A hold works as for **Brighter**. |
| **Next mode** | Shows the next mode: **Standard**, **Multi**, **Night**, **Lamp light**, then **Standard** again. It skips **Night** while the lamp has no time. |
| **Switch to mode** | Shows the mode you choose in the list: **Standard**, **Multi**, **Night** or **Lamp light**. The lamp refuses **Night** while it has no time. |
| **Show a reading** | Shows one reading on the whole matrix for 10 seconds, then the lamp returns to its own picture. Choose it in the list: **Overall**, **Temperature**, **Humidity**, **eCO₂**, **TVOC** or **PM2.5**. The lamp refuses it while it shows an error or its sensors are off. |

The defaults are **Turn lamp on / off** on TOUCH 2, **Brighter** on TOUCH 6, **Next mode** on
TOUCH 8 and **Dimmer** on TOUCH 9. The brightness steps are the steps of the Lamp card, 5 to
100 %. **Brighter**, **Dimmer**, **Next mode** and **Switch to mode** switch on a lamp that is off.
**Show a reading** does not change the power setting: the matrix goes dark again when the 10
seconds end. A mode that a button selects replaces the mode of the Lamp card, and the lamp keeps
it like any other change.

When no button has **Turn lamp on / off**, a warning reads **No button turns the lamp off — you’ll
only be able to switch it off from this page.**

When the lamp refuses an action, its matrix flashes red for half a second and the log names the
button; see [Running effects](troubleshooting.md#running-effects). When the lamp refuses a change
on this page, a red line shows its reply for about 10 seconds: **The lamp did not accept the
change.**, **No answer from the lamp.** or the text the lamp sent.

### Lock and defaults {#lock-and-defaults}

| Control | Action |
|---------|--------|
| Switch **Lock the buttons** | Locks and unlocks the touch buttons. The line next to it reads **Taps change the lamp.** or **Locked: taps do nothing until you unlock. Hold TOUCH 6 and 8 on the lamp for 3 s to unlock there.** While the buttons are locked, you can still edit their actions here; the footer reads **You can still edit while locked**. |
| **Defaults** | Restores the four default actions and keeps the lock as it is. |

The lamp keeps the actions and the lock in its flash and reads them at the next start. A missing or
damaged file gives the defaults and an unlocked state.

### System gestures {#system-gestures}

The card **System gestures** marks three gestures **always on**. They do not depend on the
actions you choose.

| Gesture | Effect |
|---------|--------|
| **Hold** | On a **Brighter** or a **Dimmer** button the brightness changes smoothly while your finger stays on the button. After 0.6 seconds the lamp makes one step every 0.2 seconds, and it saves the brightness when you release the button. |
| **6 + 8 · 3 s** | Hold TOUCH 6 and TOUCH 8 together for 3 seconds to lock or unlock the buttons. The matrix flashes amber for half a second when it locks and green when it unlocks. |
| **2 + 8 · 10 s** | Hold TOUCH 2 and TOUCH 8 together for 10 seconds: the matrix flashes aqua for half a second, and the lamp restarts and opens its own access point, without the red picture of a failed join. Your saved networks stay, and the next restart is a normal one. It works while the buttons are locked. The lamp refuses it with a red flash while it checks a WiFi network or a restart is already coming. |

A tap is a touch that you release within 0.6 seconds. Touching two buttons together does nothing
except these two holds. The lamp ignores a touch that began while it was starting.

### When the lamp cannot save {#buttons-save-failed}

If the lamp could not write a change of the actions or of the lock to its memory, it goes back to
the last saved settings and does not try the write again. The page shows the window **Couldn’t
save**, **The lamp couldn’t write the settings to its memory. It went back to the last saved
settings.** and the button **OK**. Make the change again.

---

## Log page

Open `/webserial` to read the lamp's log live. About one second after you open the page, the lamp
sends the lines it has stored since the last start; later lines arrive as they happen. The lamp
clears the stored log at every start.

!!! note "Access point mode"
    The log page also works in access point mode: connect to the lamp's own network and open
    `/webserial` there. Without a time server the lines carry the uptime stamp.

Each line has the form `time - level : message`.

| Part | Content |
|------|---------|
| `time` | In your network, the date and time `DD.MM.YYYY HH:MM:SS`, offset UTC+3. Before the lamp joins the network, in access point mode, and while the time server has not answered since the start, the stamp is the uptime: `up 12.345 s`. After the time server is lost, the clock keeps running from its last answer. |
| `level` | `Info` or `ERROR`. |
| `message` | The event. The messages are listed in [Log messages](troubleshooting.md#log-messages). |

→ [LED codes and troubleshooting](troubleshooting.md)
