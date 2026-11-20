# Display modes

The lamp turns five measured values into colour. This page explains when the picture changes,
what the colours mean and how the three modes lay them out.

---

## Picture update

- The lamp reads its sensors every few seconds and adds each reading to a running sum.
- Once a minute it draws the average of that minute on the matrix and starts a new sum. The
  picture therefore changes at most once a minute.
- After a start, and after you change the mode on the web page, the lamp draws as soon as every
  value the mode shows has been read once. That takes a few seconds, and longer for PM2.5, because
  the ZH03B warms up for 12 seconds. Until then the matrix shows the end of the start-up animation
  or stays dark.
- A sensor reading that fails is not counted. When no reading of a shown value arrives, the
  matrix keeps the previous picture and the lamp draws again as soon as a reading arrives.
- The picture does not blink dark before a redraw: the new picture replaces the old one.
- One missed CCS811 reading discards the whole minute of eCO2 and TVOC, not only that reading.

---

## Colour scale

Every value has three limits. Blue means low, green means middle, red means high.

| Value | Lower limit (blue below) | Middle (green) | Upper limit (red above) |
|-------|--------------------------|----------------|-------------------------|
| Temperature | 20 °C | 26 °C | 30 °C |
| Humidity | 40 % | 60 % | 80 % |
| eCO2 | 400 ppm | 1000 ppm | 5000 ppm |
| TVOC | 220 ppb | 660 ppb | 1000 ppb |
| PM2.5 | 15 µg/m³ | 20 µg/m³ | 45 µg/m³ |

- Below the lower limit the colour is blue; above the upper limit it is red.
- Between the lower limit and the middle the colour blends from blue to green; between the middle
  and the upper limit it blends from green to red.

!!! note "eCO2 and TVOC change in steps"
    The lamp does not blend the colours of eCO2 and TVOC. For both, the colour is blue below the
    lower limit, cyan from the lower limit up to the middle, yellow from the middle up to the
    upper limit and red above it. Green does not appear for these two values.

The colours do not depend on the brightness: the scale is fixed, and the LEDs dim it.

The limits are fixed in the firmware. To change them, edit the `dots` values in the constructor
of `HAQuDA_MeasHandler` in `sketches/HAQuDA_MeasHandler.cpp` and flash the lamp again.

---

## Multi mode

The lamp starts in this mode when no settings are saved. It shows three values at once, each in
its own horizontal zone of three LED rows, with a dark row between the zones and one at the very
bottom.

![Render of the multi mode: three horizontal colour zones in the tube](assets/Multi-mode.png)

| Zone, from the bottom | Default value |
|-----------------------|---------------|
| Lower | Temperature |
| Middle | eCO2 |
| Upper | PM2.5 |

You choose the value of each zone on the web page; see [The Lamp card](web-interface.md#lamp-card).
The mode draws only when all three values have a reading.

---

## Standard mode

The whole matrix has one colour: the colour of the value you choose on the web page, or **Overall**.
Overall shows the colour of the first reading that is out of its range, in the order temperature,
humidity, eCO2, TVOC, PM2.5. When all readings are in range, it shows the average colour of them.

---

## Night mode

The night mode shows one value that you choose, plus a night light that fills up over the night.

![Render of the night mode: three coloured columns and six white columns](assets/Night_1.png)

- Columns 1, 4 and 7 of the nine columns show the colour of the value over the full height.
- The other six columns are the night light: 18 cells, six stripes of three. Each cell is two
  columns wide and two LED rows high. The stripes fill from the bottom up, a stripe from left to
  right.
- You set the start and the end of the night in the Lamp card; the default is 21:00 to 09:00. One
  more cell lights every `window / 18` minutes, and one cell is on as soon as the window starts.
- Outside the window all cells are on.
- The lamp takes the time from the internet and applies a fixed offset of UTC+3. The lamp refuses
  to select this mode without a time, for example in access point mode. If the time is lost later,
  the lamp keeps the mode and the cells stay off.

---

## Lamp light and power off

**Lamp light** fills the matrix with one of six colours: warm white, neutral, cool white, amber,
lavender or teal. It needs no readings.

When you switch the lamp off, the matrix goes dark. The measurements, the web pages and the log
keep running, and the picture you chose comes back when you switch the lamp on.

---

## Brightness

The brightness runs from 5 to 100 % in steps of 5, and the lamp starts at 35 %. The lamp keeps the
value with your other settings.

!!! note "Start-up and WiFi pictures use the default brightness"
    In {{ app_version }} the start-up snake and the WiFi pictures play at 35 %, not at the
    saved brightness. The saved brightness applies when your picture comes back.

---

## Saved settings {#saved-settings}

The lamp writes every change to its flash at once, before it shows the change, so a reset right
after a change keeps it. The settings are in the file `/lamp`. When the file is missing or damaged,
the lamp uses the defaults: on, 35 %, multi mode (PM2.5 on top, eCO2 in the middle, temperature at
the bottom), night window 21:00 to 09:00, the first lamp light colour.

→ [Mode selection](quick-guide.md#select-the-display-mode)
