# Countdown

How many days until Christmas, New Year's Eve or Easter, with a picture of the season beside the number.

**The tile** shows the number of days large and what they count to ("36 days until Christmas"), with the date under it
where there is room, and a picture of the season to the tile's edges. Under **Arrangement** you choose how they share the
tile: the picture as background with the number on a dark veil at the left, the number at the left and the picture to
its right, or the picture as a band on top with the number under it. The pictures: a Christmas tree, an Advent wreath,
fireworks, Easter eggs, spring flowers, a beach, autumn leaves, a pumpkin, a snowman, a birthday cake, a suitcase, a hammock or a
camper van.
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
4. Optional: **Count in** weeks, working days (Monday to Friday, public holidays counted as workdays) or Fridays
   instead of days, another **Arrangement**, a **Name** for the words after the number ("days until Holidays"), and another
   **Picture**. Automatic
   takes the holiday's picture, or the season of the date for a date of your own.

Options change without a build.

## Until retirement

Switch **Automatic** off, choose **A date of my own** and fill in the day, such as `2045-03-01`; then **Name**
`retirement`, **Count in** years and months, and the picture **Hammock**: "18 years and 3 months until retirement".

For holidays the same with their first day, the **Name** `holidays`, and the picture **Hammock** or **Camper van**.

## Good to know

- The tile counts on the screen's own clock and changes at midnight.
- Boards without pictures (the CYD, the 3.5- and 4-inch Hosyond and Waveshare) show the number and the words alone.
- The pictures are in `motifs/`, drawn by `motifs/draw.py`.

## Privacy

With a picture shown, the Tessera app in Home Assistant fetches it from this repository on GitHub
(`raw.githubusercontent.com`) and sends it to the screen; GitHub sees the address of your Home Assistant, as any visitor.
Nothing else leaves your home: the dates are counted on the screen, and a calendar or entity is read from Home Assistant.

## A hobby project

This plugin is a private, non-commercial hobby project. I build and maintain it in my spare time and share it
free of charge as open source under the [MIT License](LICENSE). It is free and will stay free: there is no paid
version, no paid feature and no paid support.

Bug reports and ideas are welcome, but please keep in mind:

- There is no support and no guaranteed response time. I answer when I find the time, which can take days or weeks.
- Not every request gets built. I add what fits the plugin and what I can keep working over time.
- Pull requests are welcome; for a larger change, open an issue first so we can agree on the way.

### No warranty, no liability

The plugin is provided "as is", without any warranty and without guaranteed support, updates or fixes. You use it at
your own risk. To the extent permitted by law, I accept no liability for any damage or data loss resulting from its
use. The full terms are in the [MIT License](LICENSE).

### Buy me a coffee

If the plugin is useful to you and you would like to say thanks, you are welcome to buy me a coffee. A donation is
entirely voluntary: it is a thank-you, not a payment. It does not buy support, features, a response time or any other
service, and the plugin stays exactly the same for everyone, whether they donate or not.

[![Buy me a coffee](https://img.buymeacoffee.com/button-api/?text=Buy%20me%20a%20coffee&emoji=&slug=hflocki&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff)](https://www.buymeacoffee.com/hflocki)
