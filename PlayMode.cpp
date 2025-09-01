#include "PlayMode.hpp"

// for the GL_ERRORS() macro:
#include "gl_errors.hpp"

// for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "Load.hpp"
#include "Sprites.hpp"
#include "data_path.hpp"

// Inspiration from lecture

// loads the whole sprites collection:
// data_path returns the path relative to the executable (so we always get the right path)
GameSprite const *goldfish = nullptr;
GameSprite const *seaweed = nullptr;
Load<Sprites> sprites(LoadTagDefault, []() {
	// Ensures sprites are loaded only AFTER openGL is initialized
	static Sprites ret = Sprites::load(data_path("game.sprites"));
	goldfish = &ret.lookup("goldfish");
	seaweed = &ret.lookup("seaweed");
	return &ret;
});

PlayMode::PlayMode() {
	// TODO:
	//  you *must* use an asset pipeline of some sort to generate tiles.
	//  don't hardcode them like this!
	//  or, at least, if you do hardcode them like this,
	//   make yourself a script that spits out the code that you paste in here
	//    and check that script into your repository.

	// Copy data to PPU
	for (size_t i = 0; i < sprites->palette_table.size() && i < ppu.palette_table.size(); ++i) {
		ppu.palette_table[i] = sprites->palette_table[i];
	}
	for (size_t i = 0; i < sprites->tile_table.size() && i < ppu.tile_table.size(); ++i) {
		ppu.tile_table[i] = sprites->tile_table[i];
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	
	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	// reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// background color will be some hsv-like fade:
	//  ppu.background_color = glm::u8vec4(
	//  	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI *
	//  (background_fade + 0.0f / 3.0f) ) ) ))), 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f
	//  + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
	//  	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI *
	//  (background_fade + 2.0f / 3.0f) ) ) ))), 	0xff
	//  );

	
	// Draw background as sequence of bubble tiles
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			uint16_t tile_index = (x + y) % 4;
			uint16_t palette_index = 0;
			ppu.background[x + y * PPU466::BackgroundWidth] = (palette_index << 8) | tile_index;
		}
	}

	// //background scroll:
	// ppu.background_position.x = int32_t(-0.5f * player_at.x);
	// ppu.background_position.y = int32_t(-0.5f * player_at.y);

	// ...existing code for background...

	// Draw goldfish
	int sprite_idx = 0;
	for (auto const &tile : sprites->lookup("goldfish").tiles) {
		ppu.sprites[sprite_idx].x = int(player_at.x) + tile.offset_x;
		ppu.sprites[sprite_idx].y = int(player_at.y) + tile.offset_y;
		ppu.sprites[sprite_idx].index = tile.tile_index;
		ppu.sprites[sprite_idx].attributes = tile.palette_index;
		++sprite_idx;
	}
	// goldfish->draw(player_at.x, player_at.y);

	// some other misc sprites:
	//  for (uint32_t i = 1; i < 63; ++i) {
	//  	float amt = (i + 2.0f * background_fade) / 62.0f;
	//  	ppu.sprites[i].x = int8_t(0.5f * float(PPU466::ScreenWidth) + std::cos( 2.0f * M_PI
	//  * amt * 5.0f + 0.01f * player_at.x) * 0.4f * float(PPU466::ScreenWidth)); 	ppu.sprites[i].y =
	//  int8_t(0.5f * float(PPU466::ScreenHeight) + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f *
	//  player_at.y) * 0.4f * float(PPU466::ScreenWidth)); 	ppu.sprites[i].index = 32;
	//  	ppu.sprites[i].attributes = 6;
	//  	if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	//  }

	//--- actually draw ---
	ppu.draw(drawable_size);
}
