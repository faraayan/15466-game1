#include "Sprites.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include <string>
#include <iostream>
#include <fstream>

// Copy, don't parse

// Represent assets as arrays of uniform structures

// std::vector < GameSprite::TileRef > all_refs;
// struct DeadSprite {
//     uint32_t begin_ref, end_ref;
// };
// std::vector< DeadSprite > sprites;

Sprites Sprites::load(std::string const &filename){
	std::ifstream in(filename, std::ios::binary);
	if (!in) throw std::runtime_error("Failed to open game.sprites");

	Sprites result;
	read_chunk(in, "TILE", &result.tile_table);
	read_chunk(in, "PALE", &result.palette_table);

	std::vector< GameSprite::TileRef > goldfish_tile_refs;
	std::vector< GameSprite::TileRef > seaweed_tile_refs;
	read_chunk(in, "GOLD", &goldfish_tile_refs);
	read_chunk(in, "SEAW", &seaweed_tile_refs);

	result.sprites["goldfish"] = GameSprite(goldfish_tile_refs);
	result.sprites["seaweed"] = GameSprite(seaweed_tile_refs);

	return result;
}

GameSprite const &Sprites::lookup(std::string const &name) const {
	auto it = sprites.find(name);
	if (it == sprites.end()) {
		throw std::runtime_error("Sprite '" + name + "' not found");
	}
	return it->second;
}
