# dweep-gen
Version of old Dweep shareware game on Sega Genesis (using SGDK)

![Screenshot of the game](./doc/screenshot.png "Game screenshot")

## About

This is a SGDK port of the old shareware game Dweep.
Not much to talk about for now, still in the beginning.
Next major milestones would be:
+ ~~A* navigation for Dweep~~
+ ~~Collision with walls and objs~~
+ Lasers (almost there)
+ Some items and inventory
+ Player States
+ Game states (victory, defeat)
+ Menu


Music was converted using libOPNMIDI (midi2vgm). Still needs some manual adjustment.
SFX was imported straight from the original game.
Art uses LibreSprite for tilesets and sprites, and Tiled for tilemaps.

The original Dweep was created and programmed by Steve Pavlina, with music by Michael Huang.

## Building

For the initial steps, you'll need Python 3 with the `lxml` library.

1. Init and update git submodules (`git submodule update --init`)
2. Run the map processor script beforehand (`./scripts/process_all_maps.sh`).
3. Also run the string processor script (`./scripts/process_strings.sh`).

After this, it should not be different to build than any other SGDK project. Using version 1.82.
After [installing the toolchain](https://github.com/Stephane-D/SGDK/wiki/SGDK-Installation),
check the [usage documentation](https://github.com/Stephane-D/SGDK/wiki/SGDK-Usage).
But in general, running `make -f ${SGDK_LOCATION}/makefile.gen build` should do it.



