# SD1-A8 Libra

## How to Use

### Controls

- P or Space or [Xbox Start or Xbox A]: start new game
- ESC or [Xbox Back]: pause & leave game, and again to quit
- W,A,S,D or [Xbox left joystick]: drive forward & turn toward direction
- I,J,K,L or [Xbox right joystick]: aim turret toward direction
- N: respawn player
- Space or [Xbox Right Trigger]: shoot bullets
- V or [Xbox Left Trigger]: shoot flames
- B or [Xbox B]: call reinforcements

### Developer Cheats

- P or [Xbox Start] toggle pause in-game
- F1: toggle debug draw overlays
- F2: invincible mode
- F3: toggle noclip (disable player physics)
- F4: toggle whole-map view
- F6: change map render mode (see debug tile heat maps)
  - F6: go to next mode
  - N: in entity path finding tile heat map mode, press N to switch entity
- F8: hard restart (delete/new game)
- F9: go to next map
- T: hold to slow time to 10%
- Y: hold to speed time to 4x (may break the game)
- T+Y: hold both to speed time to 8x (may break the game)
- O: step single update and pause
- ~: toggle developer console

### Changes Beyond Document

- Improved AI path finding ( but more time consuming )
  - It will not be more time comusing than tile heat map generation so I think it is acceptable
  - AI will turn around when there is no path
  - AI will not choose a slower path(ray cast from path's last point)
  - AI will not get stuck after squeezing by others
  - Thing do not solve: AI will crush together without making way for allies
- Leo & Capricorn stop in certain range (4 units) within target and shoot
- Player has allies; player can self-repair; player can shoot fires and has a yellow tip to represent remain fuel
- Stable target search system for all entities (Search for Nearest target)
- Damage from front: 0.5 * damage; from side: normal damage; from back: 2 * damage
- Map changes: most maps are fixed (not random) and designed by me; enemies are not randomly generated; they are on certain position at the beginning of the level or created by enemy reinforcements
- Tasks: each map has tasks include:
  - kill all enemies
  - go to exit point 
  - defense some building for some time
  - capture a banner and defense for some time
- Reinforcements: player can call reinforcements when player tank is on the edge of the map; reinforcements are limited
- Wondering destination choose: improved wondering destination choose strategy for different types of tasks
----------------------------------
##### ToDo:
- After win a level player can choose a enhancement for allies or reinforcements; all undead allies will add to next map's reinforcements
- Stretch Goals:
  - planes: diving bombers and fighters
  - artillery: call to bombard some place (even damage allies) (and enemy has artillery too)
  - enemy factories: spawn enemy tanks

## Known Issues

None.

## Deep Learning

The most important thing in libra is different maps and xmls. In starship we only have a game class with no different maps, and we hard code all game constants inside GameCommon.hpp. The essential part to make a game shippable is to let the game can be redefined by mods. Although this becomes harder for programmers, but with the support of programmers, other developers in team can have a easier work environment with xmls instead of codes.

In this prerequisites, one of the important thing for programmers is to decide which variable should be exposed to others, and what is the comfortable and safe way to expose them without confusing from other team members and then how to convert them back to game data in our own code.

Learn from SD and TGP: For team development, I have learned that as a programmer, I need to quickly give a version of code that can run by team members (especially LDs) to add there things. After a runnable (but may not be best) version is implemented, programmers can try to add new things or do optimization depending on the requirements. In my own work, I always like to implement the best code which solves every thing in one time. That can be less time consuming if I need to optimize later, but if I needn't to optimize or I am working for a team or the optimized version has bugs to fix, this way may be worse, and if I am in a team, this may even break our game and delay the game progress.

## Desgin Notes

In history, tanks were used to breakthrough enemy defences. Great plain is the best place for tanks to do its jobs since there is no block for tanks and tanks can go at full speed.

Different from star ship, tanks cannot always be very strong and fight against a cohort of enemies and tanks need support from infantry, artillery and aircrafts. Tanks also do not work well in complex terrains like cities or mountains.

In game, I plan to choose the world war II east front as the game's background and reference since this is a war which tanks played an very important role. I choose player to act as German tank (maybe there will be soviet union compaign in the future) since in base game player is born on the west south corner of the map.

I choose 8 famous and deceisive campaign in east front to be the 8 levels, they are: Moscow, 1941-1942 Winter Fight, 2ndKharkov, Caucasus, Stalingrad, 3rdKharkov, Budapest, Berlin. At first Germans have a lot of reinforcements and the enemy is weak, but when the game progresses, players will find that they are in quagmires. Reinforcement replenishment is slow and enemies are too many.

As I mentioned before, tanks are not good at fight at cities but there are a lot of city fights in the last few levels. Some testers commented that the enemy was too much and they did not get joy when playing this game, and this is what war is.

If you do not feel good when playing this game, just do not continue playing it and rest. Play star ship instead.
