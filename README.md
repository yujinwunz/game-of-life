game-of-life
============

by yujinwunz@hotmail.com

Conway's game of life simulator.
This cute thing was supposed to become a multiplayer turn based game where players fight to destroy the opponent's flag (an oscillator) by buying and placing ships, cells and guns on squares that they control. But I stopped halfway and decided to just make this into a regular game of life simulator.

# Build

This was a Visual C++ application. I haven't tried compiling it with anything else so far.

Dependencies:

SDL 1.2.15 (not SDL 2)
https://www.libsdl.org/download-1.2.php

SDL_ttf 2.0
https://www.libsdl.org/projects/SDL_ttf/



# Use

Click on the "Sandbox" button from the menu, and select a size for the world.

Click and drag on the grid to draw live cells/kill cells.

You can use the navigation to pan around and zoom. The "Home" key sets the view to the origin (0, 0).

Use the speed slider to control the speed.


## Colours

You can add coloured cells. Coloured cells leave behind coloured dead cells after they die. Choose a colour to set the current pen colour.

A new live cell's colour depends on a majority vote of the colours of it's live neighbours. If there is a tie, a random choice is made.

## Enforcing rules:

Checking this option forces you pen to only be able to affect cells of the pen's own colour. The master colour is an exception: it can draw anywhere.


# Code

The algorithm keeps track of live cells in one set, and changing cells in a separate set. This implies that the time and space complexity of each iteration is independent of the grid size, which allows "infinite" sized grids. In fact they are wrapped at +/- 32767 due to coordinates being stored as shorts.

The GUI you see is actually a nice and reusable SDL gui library that I wrote. One word to describe it is that it functions. I will _maybe_ create a separate library for this some time.

