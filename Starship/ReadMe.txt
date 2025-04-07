------------Starship 1.0---------------
How to Use
1.play:
run Starship_Release_x64.exe in ./Run folder
2.control
There are tutorials in game.
W to thrust; A to turn left; D to turn right; for XBox controller, left joystick is used to move
space or XBox controller A to shoot bullet
following weapons will unlock through levels:
J or XBox controller B to give a cone attack 
K or XBox controller Y to shoot rocket
L or XBox controller X to turn on light saber
F8 to restart game
Esc to exit game
3.change to original control
Make accerleration speed a big number and clamp the velocity of player ship. Then add a big fraction to stop playership when not thrusting.
4.Others
Player can choose upgrade something when finishing a level.
Shooting needs ammo and ammo is growing automaticly.
A Boss which has three stages in level4.(There may be another later)
Player can pick up power ups including add health, enhance weapon and gain ammo.
If player ship gets damage, it can be invincible for 2s and health point -1.
Enemy beetle will stop and shoot player at certain range.
Enemy wasp now can bounce off edges and shoot bullet to player.
Almost every weapon can destroy bullet.
5.code structures
A level system which is convenient for LDs to design levels.
A level is seperated to several waves. When time from level begin reaches wave begin time, a wave starts.
Wave includes enemy type and number can be easily changed in GameCommon.hpp(or a text file out of program and read when program starts)
A text print system.
When in a level, disigner can decide when to print a text on the top of the screen(for tutorial or more juicy sense of reality). There are two ways to give a text: time from level begin or something related to game happens(A function pointer parameter to solve this).
An Upgrade system(which is the most connected to this game and cannot be used individually) which manages upgrading after levels.

Known Issues
As far as I know, I haven't encountered unsolved issues when testing.

Deep Learning
Coding is not for ourselves(though SD projects are completed individually), coding is for others. So as a programmer, we need to achieve three goals:
1.Make our code no bugs and stable, if situation allows, make our code as fast as possible.
2.Make other cooperative programmers easy to read our code(naming of varibles, comments).
3.Make others who do not understand code can use our code easier.
As for the third point, I think the task for programmer is to consturct something that is capable for more situation. In star ship coding, when I was coding, I always condsider how to make add new entity more easy; how to deal with future different entities(like boss effects or powerups); how to make level modifying and testing more simple and can be tested by level designers. After building a strong construction, it is easy to add something and avoid the whole construction collapsing. But something bad happened in the ending of this project too. To persue speed and sometimes too falling deep into coding(to implement something), I made some hard code and built some unstable codes which may only understood by myself and easy to produce bugs in future addition of codes. To solve this problem, in the future I need to do termly code revision to reconstruct some of my codes and make them more robust.


Notes when programming:
-----9.1.2023-----
SD1A2 finished
build: open Starship.sln and build and run
problems:
fixes:
1.The game keeps paused when pressing P continuously. After releasing P key, the game will operate if a new P key is pressed.

-----9.25.2023-----
SD1A4
features:
all enemies(beetles) faces target player ship needs time to do so(uses GetTurnedTowardDegrees() and gives a turn speed)
entities health bar
level system which is easy for LDs to design level; level ends if killed all enemy(except asteroids) and all waves are loaded;
wave is load if time from level start is exceeded the setting time
the wasp will bounce off the edge and can shoot

four attack type:
space or A: bullet
j or B: sector attack
K or Y: laser beam(may be change to rocket)
L or X: light saber

a boss(first explorer)

ammo system
dead system
control system
splash rocket
acquire weapon when finish levels

text in levels
other power ups(ammo)

need to do: another boss? last priority
juicy up


do not forget to add cone attack to pool when defeat the first boss

