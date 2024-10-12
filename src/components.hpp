#pragma once
#include "common.hpp"
#include <utility>
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

struct TransformComponent {
    glm::vec3 position;
    glm::vec3 scale;
    float rotation;
};

struct Sprite {
    GLuint textureID;
    float width;
    float height;
};

enum PlayerState {
    WALKING,
    ATTACKING,
    JUMPING,
};

/* Template Animation component for animated sprites
Each supports different animation states, such as walking, jumping, etc.
*/
template<typename State>
struct Animation {
    std::unordered_map<State, std::vector<Sprite>> frames;
    float frameDuration;
    float currentTime;
    size_t currentFrame;
    State currentState;

    Animation() : frameDuration(0.1f), currentTime(0.0f), currentFrame(0) {}

    // add a new animation state
    void addState(State newState, const std::vector<Sprite>& newSprites) {
        frames[newState] = newSprites;
    }

    // get the next frame for the current state
    void next(float deltaTime) {
        currentTime += deltaTime;
        if (currentTime >= frameDuration) {
            currentFrame = (currentFrame + 1) % frames[currentState].size();
            currentTime = 0.0f;
        }
    }

    void setState(State newState) {
        if (frames.find(newState) != frames.end()) {
            currentState = newState;
            currentFrame = 0;
            currentTime = 0.0f;
        }
    }

    const Sprite& getCurrentFrame() const {
        return frames.at(currentState)[currentFrame];
    }
};

struct Motion {
    vec2 position;
    float angle;
    vec2 velocity;
    vec2 scale;
    vec2 acceleration;
};

// Player component
struct Player
{

};

// Health component
struct Health
{
    int max_health;
    int current_health;
};

struct Damage
{
    int damage_dealt;
};

// A timer that will be associated to when the player can get damaged again 
struct InvincibilityTimer
{
    float counter_ms = 1000;
};

struct HealthFlask
{
    int num_uses = 3;
};


// anything that is deadly to the player
struct Deadly
{

};

// anything that can be damaged but the player
// enemies, bosses, 
struct Damagable
{
    int health;
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
    float counter_ms = 3000;
};

struct Collision
{
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
    Collision(Entity& other) { this->other = other; };
};

// Mesh data structure for storing vertex and index buffers
struct Mesh
{
   // static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
    vec2 original_size = {1,1};
    // std::vector<ColoredVertex> vertices;
    // std::vector<uint16_t> vertex_indices;
};

// Sets the brightness of the screen
struct ScreenState
{
    float darken_screen_factor = -1;
};

enum class TEXTURE_ASSET_ID {
    PLAYER_IDLE = 0,                      // idle.png
    PLAYER_WALK_1 = PLAYER_IDLE + 1,      // walk_1.png
    PLAYER_WALK_2 = PLAYER_WALK_1 + 1,    // walk_2.png
    PLAYER_WALK_3 = PLAYER_WALK_2 + 1,    // walk_3.png
    PLAYER_WALK_4 = PLAYER_WALK_3 + 1,    // walk_4.png
    PLAYER_JUMP_1 = PLAYER_WALK_4 + 1,    // jump_1.png
    PLAYER_JUMP_2 = PLAYER_JUMP_1 + 1,    // jump_2.png
    PLAYER_JUMP_3 = PLAYER_JUMP_2 + 1,    // jump_3.png
    PLAYER_JUMP_4 = PLAYER_JUMP_3 + 1,    // jump_4.png
    PLAYER_ATTACK_1 = PLAYER_JUMP_4 + 1,  // attack_1.png
    PLAYER_ATTACK_2 = PLAYER_ATTACK_1 + 1,// attack_2.png
    PLAYER_ATTACK_3 = PLAYER_ATTACK_2 + 1,// attack_3.png
    PLAYER_ATTACK_4 = PLAYER_ATTACK_3 + 1,// attack_4.png
    PLAYER_ATTACK_5 = PLAYER_ATTACK_4 + 1,// attack_5.png
    GOOMBA_WALK_ATTACK = PLAYER_ATTACK_5 + 1,  // goomba_walk_attack.PNG
    GOOMBA_WALK_HIT = GOOMBA_WALK_ATTACK + 1,  // goomba_walk_hit.PNG
    GOOMBA_WALK_IDLE = GOOMBA_WALK_HIT + 1,    // goomba_walk_idle.PNG
    GOOMBA_WALK_NOTICE = GOOMBA_WALK_IDLE + 1, // goomba_walk_notice.PNG
    GOOMBA_DEAD = GOOMBA_WALK_NOTICE + 1,      // goomba_dead.PNG
    CEILING_FALL = GOOMBA_DEAD + 1,            // ceiling_fall.png
    CEILING_HIT = CEILING_FALL + 1,            // ceiling_hit.png
    CEILING_IDLE = CEILING_HIT + 1,            // ceiling_idle.png
    SPLASH_SCREEN = CEILING_IDLE + 1,          // splash_screen.png
    DEMO_GROUND = SPLASH_SCREEN + 1,           // demo_ground.png
    TEXTURE_COUNT = DEMO_GROUND + 1          // Count of all textures
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    PLAYER_EFFECT = 0,
    EFFECT_COUNT = PLAYER_EFFECT + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    PLAYER_GEO = 0,
    SPRITE = PLAYER_GEO + 1,
    GEOMETRY_COUNT = SPRITE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

struct Gravity {
    float accleration = 0.0098f;
};
