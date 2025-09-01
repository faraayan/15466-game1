#pragma once

#include "PPU466.hpp"

#include <map>
#include <string>

struct GameSprite {
    struct TileRef {
        uint16_t tile_index = 0;
        uint16_t palette_index = 0;
        int16_t offset_x = 0;
        int16_t offset_y = 0;
    };
    // Since we have 16 bits = 2 bytes, and 4 of these bytes
    static_assert(sizeof(TileRef) == 8, "TileRef must be 8 bytes");

    GameSprite() = default;
    GameSprite(const std::vector<TileRef>& tiles_) : tiles(tiles_) {}

    std::vector< TileRef > tiles;

    // draw a sprite with origin at x, y:
    void draw(int32_t x, int32_t y) const;
};

class Sprites {
public:
    // Lookup a specific sprite by name and return a reference to it (or throw on failure)
    GameSprite const &lookup(std::string const &name) const;

    // We can tell a function is static if we use Sprites::load()
    // and not Sprites.load(), And means you don't need an instance of Sprites to call it
    // which is good because this function gives us our sprites.
    static Sprites load(std::string const &filename);

    std::map< std::string, GameSprite > sprites;
    std::vector< PPU466::Tile > tile_table;
    std::vector< PPU466::Palette > palette_table;
};