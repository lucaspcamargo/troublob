# Dweep Genesis
A demake of [Dweep](https://en.wikipedia.org/wiki/Dweep) for the Sega MegaDrive.

![Screenshot of the game](./doc/screenshot.png "Game screenshot")

## About

This is a work-in-progress SGDK port of the old shareware game Dweep.
The main objective of the project is to be a good C programming exercise,
as well as a way tor realize a long-standing desire to code for the Genesis.

Most of the game subsystems are in place, and we have a good base for the game.
Now the focus is on getting the gameplay mechanics and objects working.

Next major milestones would be:
+ ~~A* navigation for Dweep~~
+ ~~Collision with walls and objs~~
+ Lasers (almost there)
+ Some items and inventory
+ Player States
+ Game states (victory, defeat)
+ Menu


A couple of songs were rearranged using the [Furnace](https://github.com/tildearrow/furnace/) tracker
Some music was converted using libOPNMIDI (midi2vgm), and still needs some manual adjustment or arrangement.
SFX was imported straight from the original game.
Art uses [LibreSprite](https://github.com/LibreSprite/LibreSprite) for tilesets and sprites, and [Tiled](https://github.com/mapeditor/tiled) for tilemaps.

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



