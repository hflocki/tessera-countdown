# Countdown

How many days until Christmas, New Year's Eve or Easter, with a picture of the season beside the number.

**The tile** shows the number of days large and what they count to ("36 days until Christmas"), with the date under it
where there is room, and a picture of the season to the tile's edges. Under **Arrangement** you choose how they share the
tile: the picture as background with the number on a dark veil at the left, the number at the left and the picture to
its right, or the picture as a band on top with the number under it. The pictures: a Christmas tree, an Advent wreath,
fireworks, Easter eggs, spring flowers, a beach, autumn leaves, a pumpkin, a snowman, a birthday cake or a suitcase.
On the day itself the tile wishes "Merry Christmas" instead of a number; a calendar's event shows its time.

**Automatic**, on by default, counts to the nearest of Christmas Eve, New Year's Eve and Easter Sunday and moves on by
itself: after Christmas to New Year's Eve, then to Easter, then to Christmas again, each with its own picture.

## Set up

1. In Tessera, open **Plugins**, **Add with a link**, and paste the link of this repository. Add the plugin to a
   screen; the screen builds once.
2. In **Layout**, place the **Countdown** tile. 2 x 1 suits every layout; on a tile of one column the picture is
   the background.
3. In the tile's options, leave **Automatic** on for the holidays one after the other, or switch it off and choose
   what it counts to under **Count down to (Automatic off)**:
   - One holiday: Christmas Eve, New Year's Eve, Easter Sunday, the first Sunday of Advent,
     Halloween or the start of summer.
   - **A date of my own**, and fill in **Date**: `24.12.` or `12-24` for every year, `2027-05-01` or `1.5.2027` once.
   - **A calendar or entity**, and choose it under **Calendar or entity**: a calendar counts to its next event (its
     name under the number), an `input_datetime` or a timestamp sensor to its date.
4. Optional: another **Arrangement**, a **Name** for the words after the number ("days until Holidays"), and another
   **Picture**. Automatic
   takes the holiday's picture, or the season of the date for a date of your own.

Options change without a build.

## Good to know

- The tile counts on the screen's own clock and changes at midnight.
- Boards without pictures (the CYD, the 3.5- and 4-inch Hosyond and Waveshare) show the number and the words alone.
- The pictures are in `motifs/`, drawn by `motifs/draw.py`.

## Privacy

With a picture shown, the Tessera app in Home Assistant fetches it from this repository on GitHub
(`raw.githubusercontent.com`) and sends it to the screen; GitHub sees the address of your Home Assistant, as any visitor.
Nothing else leaves your home: the dates are counted on the screen, and a calendar or entity is read from Home Assistant.
