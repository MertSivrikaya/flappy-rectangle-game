# flappy-rectangle-game
A game in which you control a flying rectangle between pipes, made with C and Raylib

Currently, this Makefile works both for Windows and Linux.

For Windows, this folder is enough to compile the project, the necessary include and static library files for Raylib are already inside the directory.
No additional files are required.
Make sure cpp compiler is in the PATH in Windows.

For Linux, the raylib library has to be installed under /usr/lib directory.

To compile the game, type "make" in the terminal in the 'space-shooter-game' directory.
To delete the executables, type "make clean" in the terminal in the 'space-shooter-game' directory.

To play the game:
  Use "space" key to elevate the flying rectangle.
  Use "escape" key or the cross button to close the game.

