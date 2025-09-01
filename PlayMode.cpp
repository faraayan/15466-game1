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
static constexpr int goldfish_width = 48;
static constexpr int goldfish_height = 16;
GameSprite const *seaweed = nullptr;
unsigned int points = 0;
Load<Sprites> sprites(LoadTagDefault, []() {
	// Ensures sprites are loaded only AFTER openGL is initialized
	static Sprites ret = Sprites::load(data_path("game.sprites"));
	goldfish = &ret.lookup("goldfish");
	seaweed = &ret.lookup("seaweed");
	return &ret;
});

PlayMode::PlayMode() {
	// Copy data to PPU
	for (size_t i = 0; i < sprites->palette_table.size(); i++) {
		ppu.palette_table[i] = sprites->palette_table[i];
	}
	for (size_t i = 0; i < sprites->tile_table.size(); i++) {
		ppu.tile_table[i] = sprites->tile_table[i];
	}
	
	player_at = glm::vec2(110.0f, 50.0f);

	// seaweed positions
	seaweed_positions = {{20, 40},   {60, 120}, {100, 80}, {140, 200}, {180, 30},
						 {220, 160}, {50, 200}, {90, 60},  {170, 100}, {210, 180}};
}

PlayMode::~PlayMode() {}

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
	float goldfish_speed = 30.0f;
	if (left.pressed) player_at.x -= goldfish_speed * elapsed;
	if (right.pressed) player_at.x += goldfish_speed * elapsed;
	if (down.pressed) player_at.y -= goldfish_speed * elapsed;
	if (up.pressed) player_at.y += goldfish_speed * elapsed;

	if (player_at.x < 0) {
		player_at.x += PPU466::ScreenWidth;
	} else if (player_at.x >= PPU466::ScreenWidth) {
		player_at.x -= PPU466::ScreenWidth;
	}
	if (player_at.y < 0) {
		player_at.y += PPU466::ScreenHeight;
	} else if (player_at.y >= PPU466::ScreenHeight) {
		player_at.y -= PPU466::ScreenHeight;
	}

	float seaweed_speed = 30.0f;
	for (auto &pos : seaweed_positions) {
		pos.x += seaweed_speed * elapsed;
		if (pos.x < 0) {
			pos.x += PPU466::ScreenWidth;
		} else if (pos.x >= PPU466::ScreenWidth) {
			pos.x -= PPU466::ScreenWidth;
		}
		if (pos.y < 0) {
			pos.y += PPU466::ScreenHeight;
		} else if (pos.y >= PPU466::ScreenHeight) {
			pos.y -= PPU466::ScreenHeight;
		}
	}

	// Check collision goldfish and seaweed
	seaweed_positions.erase(std::remove_if(seaweed_positions.begin(), seaweed_positions.end(),
										   [&](const glm::vec2 &pos) {
											   float fish_left = player_at.x;
											   float fish_right = player_at.x + goldfish_width;
											   float fish_top = player_at.y;
											   float fish_bottom = player_at.y + goldfish_height;

											   float seaweed_left = pos.x;
											   float seaweed_right = pos.x + 8.0f;
											   float seaweed_top = pos.y;
											   float seaweed_bottom = pos.y + 16.0f;

											   bool collision = seaweed_left <= fish_right &&
																seaweed_right >= fish_left &&
																seaweed_top <= fish_bottom &&
																seaweed_bottom >= fish_top;
											   if (collision) {
												   points++;
												   std::string title;
												   if (points == 10) {
													   title =
														   "serene waters [goldfish is full! 💗]";
												   } else {
													   title = "serene waters [";
													   for (int i = 0; i < points; i++)
														   title += "★";
													   for (int i = points; i < 10; i++)
														   title += "☆";
													   title += "]";
												   }
												   SDL_SetWindowTitle(Mode::window, title.c_str());
											   }
											   return collision;
										   }),
							seaweed_positions.end());

	// reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// draw background
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			uint16_t tile_index = (x + y) % 4;
			uint16_t palette_index = 0;
			ppu.background[x + y * PPU466::BackgroundWidth] = (palette_index << 8) | tile_index;
		}
	}

	// background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);

	// draw goldfish
	int sprite_idx = 0;
	const auto &sprite = sprites->lookup("goldfish");
	for (auto const &tile : sprite.tiles) {
		ppu.sprites[sprite_idx].x = int(player_at.x) + tile.offset_x;
		ppu.sprites[sprite_idx].y = int(player_at.y) + tile.offset_y;
		ppu.sprites[sprite_idx].index = tile.tile_index;
		ppu.sprites[sprite_idx].attributes = 1;
		++sprite_idx;
	}

	// draw seaweed
	int seaweed_drawn = 0;
	for (const auto &pos : seaweed_positions) {
		for (auto const &tile : sprites->lookup("seaweed").tiles) {
			int x_pos = int(pos.x) + tile.offset_x;
			int y_pos = int(pos.y) + tile.offset_y;
			ppu.sprites[sprite_idx].x = x_pos;
			ppu.sprites[sprite_idx].y = y_pos;
			ppu.sprites[sprite_idx].index = tile.tile_index;
			ppu.sprites[sprite_idx].attributes = tile.palette_index;
			sprite_idx++;
		}
		seaweed_drawn++;
	}

	// draw hidden sprites off bounds
	for (int i = seaweed_drawn; i < 10; i++) {
		for (auto const &tile : sprites->lookup("seaweed").tiles) {
			ppu.sprites[sprite_idx].x = 250;
			ppu.sprites[sprite_idx].y = 250;
			ppu.sprites[sprite_idx].index = tile.tile_index;
			ppu.sprites[sprite_idx].attributes = tile.palette_index;
			++sprite_idx;
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
