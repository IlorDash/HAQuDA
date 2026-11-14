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
| `/webserial` | The live log of the lamp. Works in access point mode too. |

In your network, any other address answers with an empty 404 page. In access point mode, any other
address returns the main page.

---

## Main page

The page asks the lamp for new values every 5 seconds and loads nothing from the internet, so it
works in access point mode. From top to bottom it shows the header, the lamp's picture with the air
index, the tiles and the links.

### Header and banners

The header shows signal bars and one status line.

| Status line | Meaning |
|-------------|---------|
| **Connecting to the lamp…** | The page has had no answer yet. |
| **Connected to** *network* **· -60 dBm** | The lamp is in your network. Four bars above -55 dBm, three above -65, two above -75, one below. |
| **Access point · no internet** | The lamp runs its own access point. |
| **WiFi connection lost** | The lamp is in station mode and has no connection. |
| **Lamp not responding · retry in 5 s** | The page gets no answer from the lamp. |

A banner appears under the header when there is something to do.

| Banner | Cause | Action |
|--------|-------|------------|
| **The lamp couldn't find a saved network and started its own. Add your home WiFi.** and the button **Add WiFi network** | The lamp runs its access point. | Press the button; see [Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi). |
| **No response from the lamp. ...** | The lamp restarts, is off, or your phone changed networks. | Wait. The page keeps the last values, dims them and recovers by itself. |
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

While the grid shows an effect or an error, the legend beside it gives a caption instead of the
readings.

| Caption | The lamp is |
|---------|-------------|
| **Starting or connecting to WiFi…** | Running the lime snake: start or reconnect. |
| **Access point mode: add your WiFi** | Fading in access point mode. |
| **Connected to WiFi** | Flashing green after it joined your network. |
| **WiFi connection lost** | Flashing red after it lost the network. |
| **The lamp is showing an effect, not readings** | Showing any other effect, for example the random pixels. |
| **Can't connect to WiFi**, **Can't start the access point**, **Storage error**, **Sensor error**, **Can't get the time** | Signalling this error; see [LED codes](troubleshooting.md#start-up-effects). |

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
| Links **User guide ↗** and **GitHub ↗** | Open sites on the internet; the notes **needs internet** mark them. |
| **EN**, **RU** and the round theme button | Switch the language and the theme. |

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
