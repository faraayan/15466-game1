#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include "Sprites.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"

// Sets the color (0 to 3) for pixel (x, y) in tile
void set_tile_pixel(PPU466::Tile &tile, unsigned int x, unsigned int y, uint8_t palette_idx) {
    tile.bit0[y] &= ~(1 << x);  // clear bits first
    tile.bit1[y] &= ~(1 << x);
    tile.bit0[y] |= ((palette_idx) << x);
    tile.bit1[y] |= (((palette_idx >> 1) & 0x1) << x);
}

// Loads our assets
int main(int argc, char **argv) {
    glm::uvec2 size;
    std::vector<glm::u8vec4> data;

    load_png("goldfish.png", &size, &data, UpperLeftOrigin);

    std::vector<PPU466::Palette> palette_table;
    std::vector<PPU466::Tile> tile_table;
    std::vector<GameSprite::TileRef> goldfish_tile_refs;
    std::vector<GameSprite::TileRef> seaweed_tile_refs;

    unsigned int blocks_x = (size.x + 7) / 8;
    unsigned int blocks_y = (size.y + 7) / 8;
    for (unsigned int by = 0; by < blocks_y; ++by) {
        for (unsigned int bx = 0; bx < blocks_x; ++bx) {
            // Palette for this block
            unsigned int index = 0;
            if ((by == 0 && bx == 0) ||  // Background palette
                (by == 1 && bx == 0) ||  // Goldfish palette
                (by == 3 && bx == 0))    // Seaweed palette
            {
                PPU466::Palette block_palette = {};
                for (unsigned int y = 0; y < 8; ++y) {
                    for (unsigned int x = 0; x < 8; ++x) {
                        glm::u8vec4 color = data[(by * 8 + y) * size.x + bx * 8 + x];
                        if (std::find(block_palette.begin(), block_palette.end(), color) ==
                            block_palette.end()) {
                            block_palette[index] = color;
                            index++;
                        }
                    }
                }
                palette_table.push_back(block_palette);
            } else {
                PPU466::Palette block_palette = {};
                for (unsigned int y = 0; y < 8; ++y) {
                    for (unsigned int x = 0; x < 8; ++x) {
                        glm::u8vec4 color = data[(by * 8 + y) * size.x + bx * 8 + x];
                        if (std::find(block_palette.begin(), block_palette.end(), color) ==
                            block_palette.end()) {
                            block_palette[index] = color;
                            index++;
                        }
                    }
                }
                if (index <= 1) {
                    continue;  // Skip "empty" tiles
                }
            }

            // Get the actual palette
            PPU466::Palette block_palette = palette_table.back();

            PPU466::Tile tile;
            for (unsigned int y = 0; y < 8; ++y) {
                for (unsigned int x = 0; x < 8; ++x) {
                    glm::u8vec4 color = data[(by * 8 + y) * size.x + bx * 8 + x];
                    auto it = std::find(std::begin(block_palette), std::end(block_palette), color);
                    int idx = 0;
                    if (it == std::end(block_palette)) {
                        std::cout << "Error: Color not found in palette! in tile (" << bx << ", "
                                  << by << ")" << std::endl;
                    } else {
                        idx = std::distance(std::begin(block_palette), it);
                    }
                    set_tile_pixel(tile, x, 7 - y, idx);
                }
            }
            tile_table.push_back(tile);

            // DESIGN DEC: All non-top row blocks are sprite tiles
            if (by > 0) {
                GameSprite::TileRef tile_ref;
                tile_ref.tile_index = tile_table.size() - 1;
                tile_ref.offset_x = bx * 8;
                if (by == 1 || by == 2) {
                    // Goldfish: offset from second-to-last row
                    tile_ref.palette_index = 1;
                    tile_ref.offset_y = 16 - by * 8;
                    goldfish_tile_refs.push_back(tile_ref);
                } else if (by == 3) {
                    // Seaweed: offset from last row
                    tile_ref.palette_index = 2;
                    tile_ref.offset_y = 0;
                    seaweed_tile_refs.push_back(tile_ref);
                }
            }
        }
    }

    std::ofstream out("./dist/game.sprites", std::ios::binary);
    write_chunk("TILE", tile_table, &out);
    write_chunk("PALE", palette_table, &out);
    write_chunk("GOLD", goldfish_tile_refs, &out);
    write_chunk("SEAW", seaweed_tile_refs, &out);
    out.close();
    return 0;
}