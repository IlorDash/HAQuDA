# Web interface

The lamp runs a small web server as soon as it has joined your WiFi network or opened its own
access point. The pages are in English only.

!!! note "The pages have no password"
    Anyone who can reach the lamp in the browser can add or delete its WiFi network. The lamp
    sends your WiFi password over plain HTTP. Use the pages on a network you trust.

---

## Page addresses

| Lamp mode | Address |
|-----------|---------|
| Access point | `http://192.168.4.22/` after you join the lamp's own WiFi network. Type the address: opening the page automatically is <span class="wip">WIP</span>. |
| Joined to your network | `http://` and the address your router gave the lamp. The lamp does not report its address <span class="wip">WIP</span>; find it in the client list of your router, under the name of the lamp's access point. |

| Address | Content |
|---------|---------------|
| `/` | The main page. |
| `/wifi_manager` | The form that adds a WiFi network. |
| `/webserial` | The live log of the lamp. Not available in access point mode: the address opens the main page. |

In your network, any other address answers with an empty 404 page. In access point mode, every
address that is not one of the form routes returns the main page.

---

## Main page

The title bar reads **HAQuDA WEB SERVER**, the heading below it reads **Hello, world!**.

| Control | Action |
|---------|--------------|
| Link **Add WiFi credentials** | Opens the form [Add WiFi credentials](#add-wifi-credentials-page). |
| Button **Delete** under **Delete stored WiFi credentials** | Deletes all stored networks. The lamp stays connected until the next start; after the start it opens its access point. |
| Button **Show** under **Show stored WiFi credentials** | Do not press it. <span class="wip">WIP</span> |

The page answers **Delete** with **Deleted all WiFi credentials**. When no network is stored,
the answer is **Error while deleting WiFi credentials**.

!!! note "WIP"
    **Show** is not ready and can crash the lamp. Do not press it.

---

## Add WiFi credentials page

The title bar reads **ESP Wi-Fi Manager**.

| Field | Input |
|-------|---------------|
| **SSID** | The name of your WiFi network, at most 31 characters. |
| **Password** | The password of your network, at most 31 characters. |
| **IP Address** | Pre-filled with `192.168.1.200`. The lamp ignores this field. <span class="wip">WIP</span> |
| **Gateway Address** | Pre-filled with `192.168.1.1`. The lamp ignores this field. <span class="wip">WIP</span> |
| **Submit** | Saves the network. The lamp restarts about 3 seconds later and tries the network. |

!!! note "WIP"
    Storing more than one network is not ready: with a network stored, the lamp answers **Error
    reading stored WiFi credentials**. Press **Delete** on the main page first.

### Answers of the form

| Answer | Cause | Action |
|--------|-------|------------|
| **Error check WiFi creds** and **Go back** | The SSID or the password is empty. | Press **Go back**: it opens the main page. Press **Add WiFi credentials** again and fill in both fields. |
| **Error writing new WiFi creds in file system** and **Error reading stored WiFi credentials** | A network is already stored. | Press **Delete** on the main page and add your network again. |
| **Error writing new WiFi creds in file system** and **Error saving stored WiFi credentials** | The lamp could not write the file. | Try once more. If it repeats, unplug the lamp and plug it in again. |
| **Success** and **Saved new WiFi credentials**, then **Device will restart in 3 seconds** | The network is saved. | Wait for the restart. The lamp joins the network; see [Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi). |

---

## Log page

Open `/webserial` to read the lamp's log live. About one second after you open the page, the lamp
sends the lines it has stored since the last start; later lines arrive as they happen. The lamp
clears the stored log at every start.

!!! note "Access point mode"
    The log page is not available in access point mode: the address opens the main page. Read the
    log when the lamp runs in your network.

Each line has the form `time - level : message`.

| Part | Content |
|------|---------|
| `time` | `Fail get time!` The clock time in log lines is not ready. <span class="wip">WIP</span> |
| `level` | `Info` or `ERROR`. |
| `message` | The event. The messages are listed in [Log messages](troubleshooting.md#log-messages). |

→ [LED codes and troubleshooting](troubleshooting.md)
