#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

// the floor height in the globe coord space
const float FLOOR_HEIGHT = -0.4f;

// ~~~~~~~~~~~~~~ Loading 3D models

GLuint snowglobe_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > snowglobe_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("snowglobe.pnct"));
	snowglobe_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > snowglobe_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("snowglobe.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = snowglobe_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = snowglobe_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});


// Load Snowflake, for all the snow in the game
GLuint snowflake_program = 0;
Load< MeshBuffer > snowflake_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("snowflake.pnct"));
	snowflake_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

std::string snow_mesh_name;
Load< Scene > snowflake_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("snowflake.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = snowflake_meshes->lookup(mesh_name);

		// This scene was unused in the game.
		// Copied from above.
		// However, we are using it to load the snow mesh_name.
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();
		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = snowflake_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
		snow_mesh_name = mesh_name;
	});
});


// ~~~~~~~~~~~~~~~~~ Game state management

PlayMode::PlayMode() : scene(*snowglobe_scene) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// bind player
	for (auto iter = scene.drawables.begin(); iter != scene.drawables.end(); iter++) {
		if (iter->transform->name == "Ginger") {
			player = &*iter;
		} else if (iter->transform->name == "Globe") {
			globe = &*iter;
		} else if (iter->transform->name == "ShadySide") {
			shadow = &*iter;
		} else if (iter->transform->name == "Broom") {
			broom = &*iter;
		} else if (iter->transform->name == "Eyebrow") {
			eyebrow = &*iter;
		} else if (iter->transform->name == "Mouth") {
			mouth = &*iter;
		}
	}
	// Throw immediately if the expected object is not present
	if (player == nullptr) throw std::runtime_error("Found no player in the scene");
	if (globe == nullptr) throw std::runtime_error("Found no globe in the scene");
	if (shadow == nullptr) throw std::runtime_error("Found no ShadySide in the scene");
	if (broom == nullptr) throw std::runtime_error("Found no broom in the scene");
	if (eyebrow == nullptr) throw std::runtime_error("Found no eyebrow in the scene");
	if (mouth == nullptr) throw std::runtime_error("Found no mouth in the scene");
	player_pos = player->transform->position;
	// Debug message: print out the initial player position.
	std::cout << "player pos is (" << player_pos.x << ", "  << player_pos.y << ", "  << player_pos.x << "). \n";
	
	// Initialize snow pieces (200)
	for (size_t idx = 0; idx < 200; ++idx) {
		add_snow(snowflake_meshes.value, snow_mesh_name, snowflake_program);
	}

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_ESCAPE) {
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		} else if (evt.key.key == SDLK_A || evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D || evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W || evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S || evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_R) {
			reset.downs += 1;
			reset.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A || evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D || evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W || evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S || evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_R) {
			reset.pressed = false;
			return true;
		}
	} 

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	glm::vec2 move = glm::vec2(0.0f);
	if (game_stage == PLAY) { // player input

		//combine inputs into a move:
		constexpr float PlayerSpeed = 5.0f;
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		glm::vec2 norm_move =glm::vec2(0.0f);
		if (move != glm::vec2(0.0f)) norm_move = glm::normalize(move);
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		// Change player facing 
		if (move != glm::vec2(0.0f)) {
			player_target_facing = glm::atan(move.y, move.x);
		}
	}

	{   // Player movement
		float v_z = player_velocity.z;
		player_velocity = lerp(player_velocity, glm::vec3(move * 10.0f, 0.0f), elapsed * PLAYER_ACCEL_LERP_FACTOR);
		player_velocity.z = v_z - 1.88f * elapsed; // GRAVITY
		
		player_velocity.z = std::clamp(player_velocity.z, -0.75f, 1.2f);
		player->transform->position += player_velocity * elapsed;

		// If player is going out of bound, put it back
		glm::vec2 plane_pos = glm::vec2(player->transform->position);
		if (glm::length(plane_pos) > 0.8f) {
			plane_pos = plane_pos / (glm::length(plane_pos) + 1e-8f) * 0.8f;
			player->transform->position.x = plane_pos.x;
			player->transform->position.y = plane_pos.y;
		}

		// If player is on the ground
		if (player->transform->position.z < FLOOR_HEIGHT + 0.05f) {
			player->transform->position.z = FLOOR_HEIGHT + 0.05f;
			player_velocity.z = 0.0f;
		}
		if (game_stage == GAMEOVER) {
			player_anim_timer += elapsed;
		}

		player->transform->rotation = slerp(
			player->transform->rotation,
			glm::quat(glm::vec3(std::min(1.7f, player_anim_timer * player_anim_timer * 1.3f), 0.0f, player_target_facing + glm::pi<float>() * 0.5f)),
			elapsed * PLAYER_ACCEL_LERP_FACTOR);
		
		player_pos = player->transform->position;

		// Shadow drawing
		shadow->transform->position.x = player_pos.x;
		shadow->transform->position.y = plane_pos.y;
		shadow->transform->scale.x = std::max(0.3, 1.0 - (player_pos.z - FLOOR_HEIGHT));
		shadow->transform->scale.y = std::max(0.3, 1.0 - (player_pos.z - FLOOR_HEIGHT));

		// glm::mat4x3 frame = camera->transform->make_parent_from_local();
		// glm::vec3 frame_right = frame[0];
		// //glm::vec3 up = frame[1];
		// glm::vec3 frame_forward = -frame[2];

		// camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{   // Player animation
		// Animate facial expression based on game stage and timer
		if (shake_count < 3) {
			eyebrow->transform->position.z = -100.0f;
		} else {
			// put it where it should be
			eyebrow->transform->position.z = 0.96475;
			mouth->transform->rotation = glm::quat(glm::vec3(0.0f, 3.141593f, 0.0f));
		}

		// Broom
		if (game_stage == PLAY) {
			broom->transform->rotation = glm::quat(glm::vec3(-blow_timer * 2.0f, 0.0f, 0.0f));
		}
	}

	if (space.pressed && blow_timer <= 0.0f && game_stage == PLAY) {  // Blowing
		// enumerate: for every snow in front of the player, within a triangle region,
		// it gets applied an accel away from the player and into the sky
		// To do so, first we have to calculate the player direction vector and the triangle?
		// the player rotation quat is calculated with (facing+0.5pi) on XYZ,
		// So dir should be (cos(facing), -sin(facing)).
		blow_timer = 0.5f;
		glm::vec2 dir_vec{glm::cos(player_target_facing), glm::sin(player_target_facing)};
		glm::vec3 base_accel = glm::vec3{dir_vec * 0.4f, 0.3f};
		// todo: change to true orientation?

		// Calculate the other two points
		glm::vec2 player_vertex = glm::vec2{player_pos} + dir_vec * 0.02f;
		glm::vec2 left_vertex = player_vertex + glm::vec2{
			glm::cos(player_target_facing + 0.25 * glm::pi<float>()),
			glm::sin(player_target_facing + 0.25 * glm::pi<float>()),
		} * 0.45f;
		glm::vec2 right_vertex = player_vertex + glm::vec2{
			glm::cos(player_target_facing - 0.25 * glm::pi<float>()),
			glm::sin(player_target_facing - 0.25 * glm::pi<float>()),
		} * 0.45f;

		// Define a lambda for 2d cross
		auto cross = [](glm::vec2 a, glm::vec2 b) {
			return a.x * b.y - b.x * a.y;
		};

		// for every single piece of snow
		for (auto &snow : snow_pieces) {
			glm::vec2 snow_vertex = glm::vec2{snow.position};
			// sign(AP, AB), sign(BP, BC), sign(CP, CA)
			// not considering overflow issues
			if (snow.position.z <= -0.1f) {
				float c1 = cross(snow_vertex - player_vertex, left_vertex - player_vertex);
				float c2 = cross(snow_vertex - left_vertex, right_vertex - left_vertex);
				float c3 = cross(snow_vertex - right_vertex, player_vertex - right_vertex);
				if (c1 * c2 > 0 && c2 * c3 > 0) {
					// Inside the triangle, apply accel (with age discount)
					snow.velocity += base_accel * (1.0f - snow.age / 60.0f);
					// Add some randomness
					snow.velocity += glm::vec3{ (std::rand() % 20 - 10) * 0.01f, (std::rand() % 20 - 10) * 0.01f, (std::rand() % 20) * 0.1f};
					snow.age = std::min(std::max(0.0f, snow.age * 0.2f), 30.0f);
				}
			}			
		}
	}
	// count down blow timer
	blow_timer = std::max(0.0f, blow_timer - elapsed);

	{ // Snow movement and age update
		for (auto &snow : snow_pieces) {
			// apply velocity to position
			snow.position += snow.velocity * elapsed;
			// GRAVITY
			snow.velocity.z -= 0.8 * elapsed;
			snow.velocity.z = std::clamp(snow.velocity.z, -0.45f, 0.8f);
			if (snow.position.z <= FLOOR_HEIGHT) {
				snow.position.z = FLOOR_HEIGHT;
				snow.velocity.z = 0.0f;
				snow.velocity *= 0.8f;
				if (game_stage == PLAY) { // only age when playing
					snow.age += elapsed;
					snow.age = std::min(snow.age, 40.0f);
				}
			}
			glm::vec2 snow_xy = glm::vec2{snow.position};
			if (glm::length(snow_xy) > 0.8f) {
				// normalized x, y of snow.position
				snow_xy = glm::normalize(snow_xy) * 0.8f;
				snow.position.x = snow_xy.x;
				snow.position.y = snow_xy.y;
			}

			// synchronize position
			snow.drawable->transform->position = snow.position;

			float factor = 0.0f;
			// interpolate snow scale
			if (snow.age < 12.0f) {
				factor = 1.0f + snow.age / 24.0f;
				snow.drawable->transform->scale = glm::vec3{0.06f * factor, 0.06f * factor, 0.02f * factor};
			} else {
				factor = 0.5f + snow.age / 12.0f;
				snow.drawable->transform->scale = glm::vec3{0.06f * factor, 0.06f * factor, 0.03f};
			}

			// Calc overlap with player. Both use Globe coords
			if (glm::length(snow.position - player_pos) < 0.065f * factor) {
				freeze_meter += elapsed * (2.0f + snow.age / 24.0f);
			}
		}
	}
	if (game_stage == PLAY) { // Random shaking!
		shake_timer -= elapsed;
		survival_timer += elapsed;
		if (shake_timer <= 0.0f) {
			shake_timer = 8.0f - std::min(5.2f, shake_count * 0.35f) + std::rand() % (shake_count < 20 ? 5 : 2);
			shake_anim_timer = 0.7f;
			shake_count += 1;

			// send everything into the sky
			player_velocity.z += 0.75f + std::rand() % 40 / 80.0f;
			player_velocity.x += std::rand() % 20 / 40.0f - 0.25f;
			player_velocity.y += std::rand() % 20 / 40.0f -0.25f;

			// for all snow
			for (auto &snow: snow_pieces) {
				if (snow.position.z < FLOOR_HEIGHT + 0.1f) {
					snow.velocity.z += 0.55f + std::rand() % 80 / 80.0f;
					snow.velocity.x += std::rand() % 16 / 40.0f - 0.2f;
					snow.velocity.y += std::rand() % 16 / 40.0f - 0.2f;
			}
				}
		}
	}

	if (game_stage == SHAKE) { // Shaking stage
		shake_timer -= elapsed;
		if (shake_timer <= 0.0f && space.pressed) {
			shake_timer = 1.8f;
			shake_anim_timer = 0.7f;
			shake_count += 1;

			// send everything into the sky
			player_velocity.z += 0.75f + std::rand() % 40 / 80.0f;
			player_velocity.x += std::rand() % 20 / 40.0f - 0.25f;
			player_velocity.y += std::rand() % 20 / 40.0f -0.25f;

			// for all snow
			for (auto &snow: snow_pieces) {
				snow.velocity.z += 0.55f + std::rand() % 80 / 80.0f;
				snow.velocity.x += std::rand() % 16 / 40.0f - 0.2f;
				snow.velocity.y += std::rand() % 16 / 40.0f - 0.2f;
			}
		}
		if (shake_count > 3) {
			game_stage = PLAY;
		}
	}

	{ // Animate shake
		shake_anim_timer -= elapsed;
		shake_anim_timer = std::max(0.0f, shake_anim_timer);
		globe->transform->position.z = (0.25f - std::pow((shake_anim_timer / 0.7f - 0.5f), 2.0f)) * 0.8f;
	}

	{ // Camera positioning
		if (game_stage == SHAKE) {
			camera->transform->position = glm::vec3{0.0f, -6.0f, 1.0f};
		} else {
			camera->transform->position = lerp(
				camera->transform->position,
				glm::vec3{0.0f, -4.5, 3.5f},
				elapsed
			);
		}
	}

	{ // Handle freeze meter and camera
		if (game_stage != GAMEOVER) {
			freeze_meter = std::max(0.0f, freeze_meter - elapsed);
		}
		if (game_stage == SHAKE) {
			camera->fovy = lerp(camera->fovy, 0.471f - shake_count * 0.06f, elapsed * 2.0f);
		} else {
			camera->fovy = lerp(camera->fovy, std::max(0.1f, 0.471f - freeze_meter / 100.0f), elapsed * 3.0f);
		}

		// Camera direction
		// We want the camera to always point *almost* toward the player.
		// Not precisely the player, so the shaking is more visible.
		glm::vec3 camera_dir = player_pos - camera->transform->position;
		if (freeze_meter < 37.0f) {
			// trembling
			camera_dir += glm::vec3((std::rand()%20)/20.0f, (std::rand()%20)/20.0f, (std::rand()%20)/20.0f)
					   * 0.0001f * std::pow(freeze_meter, 1.25f);
		}
		
		// Inspired by https://gamedev.stackexchange.com/questions/149006/direction-vector-to-quaternion
		// But different
		camera->transform->rotation = glm::vec3{
			glm::atan(glm::length(glm::vec2(camera_dir)), game_stage == SHAKE ? 1.2f : -camera_dir.z + 0.2f), 0.0f, -glm::atan(camera_dir.x, camera_dir.y)
		};

		// Camera random shaking

		// If freeze_meter is over 37 (death)
		if (freeze_meter >= 37.0f) {
			// died
			game_stage = GAMEOVER;
		}

	}

	if (game_stage == GAMEOVER && reset.pressed) { // game reset
		player_pos.x = 0;
		player_pos.y = 0;
		player->transform->position.x = 0;
		player->transform->position.y = 0;
		for (SnowPiece &snow : snow_pieces) {
			snow.age = 0;
			float rand_theta = (std::rand() % 1024) / 1024.0f * 2.0f * 3.1415926f;
			float rand_gamma = (std::rand() % 1024) / 2048.0f + 0.5f;
			snow.position = glm::vec3(glm::cos(rand_theta) * rand_gamma, glm::sin(rand_theta) * rand_gamma, FLOOR_HEIGHT);
			snow.drawable->transform->position = snow.position;
			snow.drawable->transform->scale = glm::vec3(0.06f, 0.06f, 0.02f);
		}
		freeze_meter = 0.0f;
		player_anim_timer = 0.0f;
		shake_timer = 0.0f;
		shake_count = 0;
		shake_anim_timer = 0.0f;
		survival_timer = 0.0f;
		game_stage = SHAKE;

	}


	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	reset.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.04f, 0.25f, 0.28f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		std::string text = "";
		if (game_stage == SHAKE) {
			text = "Press [SPACE]";
		} else if (game_stage == PLAY) {
			text = "[WASD/SPACE] Survived  " + std::to_string(static_cast<int>(survival_timer)) + "s!";
		} else {
			text = "GAME OVER! Survived  " + std::to_string(static_cast<int>(survival_timer)) + "s!";
		}

		constexpr float H = 0.2f;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0xff));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0x33, 0xff));
	}
}

void PlayMode::add_snow(MeshBuffer const*  snowflake_meshes, std::string const mesh_name, GLuint snowflake_program) {
	Mesh const &mesh = snowflake_meshes->lookup(mesh_name);
	
	// add a new transform
	scene.transforms.emplace_back();
	Scene::Transform &xform = scene.transforms.back();

	// Complete the transform and drawable init
	int idx = snow_pieces.size();
	xform.name = "Snowflake" + std::to_string(idx);
	xform.parent = globe->transform;

	float rand_theta = (std::rand() % 1024) / 1024.0f * 2.0f * 3.1415926f;
	float rand_gamma = (std::rand() % 1024) / 2048.0f + 0.5f;
	xform.position = glm::vec3(glm::cos(rand_theta) * rand_gamma, glm::sin(rand_theta) * rand_gamma, FLOOR_HEIGHT);
	xform.scale = glm::vec3(0.06f, 0.06f, 0.02f);

	scene.drawables.emplace_back(&xform);
	Scene::Drawable &drawable = scene.drawables.back();
	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = snowflake_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;

	snow_pieces.emplace_back();
	snow_pieces.back().position = xform.position;
	snow_pieces.back().drawable = &drawable;
}