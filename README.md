# (TODO: your game's title)

Author: Fara Yan (fayan)

Design: (TODO: In two sentences or fewer, describe what is new and interesting about your game.)

Screen Shot:

![Screen Shot](screenshot.png)

How Your Asset Pipeline Works:

(TODO: describe the steps in your asset pipeline, from source files to tiles/backgrounds/whatever you upload to the PPU466.)
* Run node Maekfile.js and ./dist/assets to load our assets into [game.sprites](./dist/game.sprites). This runs the main function in [load_assets.cpp](./load_assets.cpp), which reads from [goldfish.png](./goldfish.png), which includes our bubble tiles and the goldfish sprite. This uses load_chunk to store a table of palettes, tiles, and sprites.
* We then read this in with read_chunk.

(TODO: make sure the source files you drew are included. You can [link](your/file.png) to them to be a bit fancier.)

How To Play:

(TODO: describe the controls and (if needed) goals/strategy.)

This game was built with [NEST](NEST.md).

