![Documentation logo consisting of the original game logo and a "Multiplayer" text next to it](./images/banner-logo.png)

# Using command line arguments

The Multiplayer mod adds a few useful command line arguments in addition to the Mod Loader ones to run the game directly in multiplayer mode.

## How to use

To launch the game via command lines you have two options:

* The first option is to navigate to your game folder using the Windows Explorer, then click on the navigation bar, type `cmd` and hit enter. In the freshly opened window, type `sonic` followed by the chosen commands. Once you're done, hit enter and the game will start.
* The second option is to create a shortcut to your game, then add commands in the target field after the quotes. Once you're done, run the shortcut.

## Mod Loader commands
See a complete list [here](https://github.com/X-Hax/SADXModdingGuide/wiki/Command-Lines).

**To spawn directly in a level:**
```
--level [id/name]
-l [id/name]
```
Note: the stage name should not have space, for example: `-l emeraldcoast`.

**To spawn in a specific act:**
```
--act [id]
-a [id]
```
Note: the ID starts at 0, so Act 2 would be `-a 1`.

**To choose a main character / stage layout:**
```
--character [id/name]
-c [id/name]
```

## Multiplayer commands
These commands are only useful if you're directly spawning in a level using `--level` or `-l`, otherwise they won't do anything.

**To spawn in multiplayer mode:**
```
--multi [count] [coop/battle]
```
Directly starts a multiplayer session with the specified amount of players in either coop or battle mode. The maximum amount is 4 players.

**To choose a character for a specific player:**
```
-p1 [id/name]
-p2 [id/name]
-p3 [id/name]
-p4 [id/name]
```
Note: if you don't specify a character, the mod will not load a player.

**To spawn in netplay**
```
--net [host/client] [ip] [port]
```
Directly look for server/clients.

## Example

```
sonic -l emeraldcoast --multi 2 battle -c sonic -p2 tails
```

This will launch the game in Emerald Coast act 1 with Sonic and Tails in Battle mode.

```
sonic -l twinklepark -a 1 --multi 4 coop -c sonic -p2 tails -p3 knuckles -p4 amy
```

This will launch the game in Twinkle Park act 2 with Sonic, Tails, Knuckles and Amy in Co-op mode.
