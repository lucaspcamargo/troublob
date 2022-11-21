# dweep-gen
Version of old Dweep shareware game on Sega Genesis (using SGDK)

![Screenshot of the game](screenshot.png "Game screenshot")

## About

This is a SGDK port of the old shareware game Dweep.
Not much to talk about for now, still in the beginning.
Next major milestones would be:
- A* navigation for Dweep
- Collision with walls etc
- Lasers
- Some items and inventory
- Player States
- Game states (victory, defeat)
- Menu


Music was converted using libOPNMIDI (midi2vgm).
SFX was converted by rescomp.
Art uses LibreSprite for tilesets and sprites, and Tiled for tilemaps.

The original Dweep was created and programmed by Steve Pavlina, with music by Michael Huang.

## Building

The current makefile came from the SGDK marsdev example. If using another toolchain you may have to change/replace the makefile.
Should not be different to build than any other SGDK project.
