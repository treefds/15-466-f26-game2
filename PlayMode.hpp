#include "Mode.hpp"

#include "Scene.hpp"
#include "Mesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, reset;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

	//================ Game states ==================

	// Game stage records game progress (intro sequence, main game, or game over.)
	enum {
		SHAKE = 0,
		PLAY = 1,
		GAMEOVER = 2
	} game_stage;

	/* PLAYER */
	// The gingerbread man character, controlled throughout the game
	Scene::Drawable *player = nullptr;
	// The globe
	Scene::Drawable *globe = nullptr;
	// The player shadow
	Scene::Drawable *shadow = nullptr;
	// The Broom
	Scene::Drawable *broom = nullptr;
	// Eyebrow
	Scene::Drawable *eyebrow = nullptr;
	// Mouth
	Scene::Drawable *mouth = nullptr;

	// Player position
	glm::vec3 player_pos;

	// Player veclocity
	glm::vec3 player_velocity { };
	float player_target_facing = 4.7f;

	// shaking
	float shake_timer = 0.0f;
	int shake_count = 0;
	float shake_anim_timer = 0.0f;
	float survival_timer = 0.0f;


	// Player animation flags:
	int player_anim_flag = 0;
	// Player animation timer, used for death animation
	float player_anim_timer = 0.0f;

	// buried timer (how cold)
	float freeze_meter = 0.0f;
	float blow_timer = 0.0f;

	// Snow(s)
	struct SnowPiece {
		Scene::Drawable *drawable;
		glm::vec3 position;
		glm::vec3 velocity { };
		float age = 0.0f;
	};

	std::vector<SnowPiece> snow_pieces;

	// Helper functions

	// Emplace a new SnowPiece object to snow_pieces.
	void add_snow(MeshBuffer const* snowflake_meshes, std::string const mesh_name, GLuint snowflake_program);
};

// ~~~~~~~~~~~~~ Helper methods

template <typename T>
inline T lerp(T a, T b, float t) {
	return (1.0f - t) * a + t * b;
}

// ~~~~~~~~~~~~~~ Helper constants
const float PLAYER_ACCEL_LERP_FACTOR = 5.0f;
