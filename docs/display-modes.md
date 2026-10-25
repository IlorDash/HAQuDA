# Display modes

The lamp turns five measured values into colour. This page explains when the picture changes,
what the colours mean and how the three modes lay them out.

---

## Picture update

- The lamp reads its sensors every few seconds and adds each reading to a running sum.
- Once a minute it draws the average of that minute on the matrix and starts a new sum. The
  picture therefore changes at most once a minute.
- After a start, the first picture appears about one minute later. Until then the matrix shows
  the end of the start-up animation or stays dark. The standard and night modes are the
  exception: they draw once at the start with no reading yet, and that colour means nothing.
- A sensor reading that fails is not counted. When no reading of a value arrives during the
  minute, the multi mode keeps the previous picture on the matrix.
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

The limits are fixed in the firmware. To change them, edit the `dots` values in the constructor
of `HAQuDA_MeasHandler` in `sketches/HAQuDA_MeasHandler.cpp` and flash the lamp again.

---

## Multi mode

The lamp starts in this mode. It shows three values at once, each in its own horizontal zone of
three LED rows, with a dark row between the zones and one at the very bottom.

![Render of the multi mode: three horizontal colour zones in the tube](assets/Multi-mode.png)

| Zone, from the bottom | Value |
|-----------------------|-------|
| Lower | Temperature |
| Middle | eCO2 |
| Upper | PM2.5 |

The mode draws only when temperature, eCO2 and PM2.5 all have a reading from the last minute.

---

## Standard mode

The whole matrix has one colour: the colour of one value that you choose in the firmware
(`temp`, `humid`, `eCO2`, `TVOC` or `PM2_5`). The firmware has no default value, so the matrix
stays dark until you choose one.

Selecting this mode without changing the firmware is not ready <span class="wip">WIP</span>; see
[Mode selection](quick-guide.md#select-the-display-mode).

!!! note "A missing reading gives a meaningless colour"
    When the chosen sensor delivers no reading during the minute, the colour of that picture
    means nothing. Check the sensor and read the log.

---

## Night mode

The night mode shows one value that you choose in the firmware, as in the standard mode, plus
a progress bar of the night.

![Render of the night mode: three coloured columns and six white columns](assets/Night_1.png)

- Columns 1, 4 and 7 of the nine columns show the colour of the value over the full height.
- The other six columns are dim white bars. Each bar grows from the bottom by one LED per hour,
  from 21:00 to 09:00 — from empty at 21:00 to full height at 09:00.
- Outside the night hours the bars are full.
- The lamp takes the time from the internet and applies a fixed offset of UTC+3. It has no
  time when it runs as an access point, and the bars stay empty.

Selecting this mode without changing the firmware is not ready <span class="wip">WIP</span>; see
[Mode selection](quick-guide.md#select-the-display-mode).

Change the night hours with `SetNightTime` in `setup()`; the default is 21 for the start and
9 for the end.

→ [Mode selection](quick-guide.md#select-the-display-mode)
