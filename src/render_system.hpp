#pragma once

#include <string>
#include "common.hpp"
#include "ecs.hpp"
#include "components.hpp"
#include "game_state_manager.hpp"
#include <array>
#include "physics_system.hpp"
//#include "world_system.hpp"

class RenderSystem
{
    std::array<GLuint, texture_count> texture_gl_handles;
    std::array<ivec2, texture_count> texture_dimensions;

    const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
            {
                    std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_IDLE_MESH, mesh_path("mesh_walk_3.obj")),
                    std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_WALK_MESH, mesh_path("mesh_walk_3.obj")),
                    std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_JUMP_MESH, mesh_path("mesh_jump_3.obj")),
                    std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_ATTACK_MESH, mesh_path("mesh_attack_3.obj"))
            };

    // Make sure these paths remain in sync with the associated enumerators.
    const std::array<std::string, texture_count> texture_paths = {
            textures_path("idle.png"),                // PLAYER_IDLE
            textures_path("walk_1.png"),              // PLAYER_WALK_1
            textures_path("walk_2.png"),              // PLAYER_WALK_2
            textures_path("walk_3.png"),              // PLAYER_WALK_3
            textures_path("walk_4.png"),              // PLAYER_WALK_4
            textures_path("jump_1.png"),              // PLAYER_JUMP_1
            textures_path("jump_2.png"),              // PLAYER_JUMP_2
            textures_path("jump_3.png"),              // PLAYER_JUMP_3
            textures_path("jump_4.png"),              // PLAYER_JUMP_4
            textures_path("attack_1.png"),            // PLAYER_ATTACK_1
            textures_path("attack_2.png"),            // PLAYER_ATTACK_2
            textures_path("attack_3.png"),            // PLAYER_ATTACK_3
            textures_path("attack_4.png"),            // PLAYER_ATTACK_4
            textures_path("attack_5.png"),            // PLAYER_ATTACK_5
            textures_path("goomba_walk_attack.PNG"),  // GOOMBA_WALK_ATTACK
            textures_path("goomba_walk_hit.PNG"),     // GOOMBA_WALK_HIT
            textures_path("goomba_walk_idle.PNG"),    // GOOMBA_WALK_IDLE
            textures_path("goomba_walk_notice.PNG"),  // GOOMBA_WALK_NOTICE
            textures_path("goomba_dead.PNG"),         // GOOMBA_DEAD
            textures_path("ceiling_fall.png"),        // CEILING_FALL
            textures_path("ceiling_hit.png"),         // CEILING_HIT
            textures_path("ceiling_idle.png"),        // CEILING_IDLE
            textures_path("splash_screen.png"),       // SPLASH_SCREEN
            textures_path("demo_ground.png"),         // DEMO_GROUND
            textures_path("demo_ceiling.png"),        // DEMO_CEILING
            textures_path("heart_3.png"),             // HEART_3
            textures_path("heart_2.png"),             // HEART_2
            textures_path("heart_1.png"),             // HEART_1
            textures_path("heart_0.png"),             // HEART_0
            textures_path("entrance_bg.PNG"),         // ENTRANCE_BG
            textures_path("spaceship.PNG"),           // SPACESHIP
            textures_path("pipes.PNG"),               // PIPES
            textures_path("cesspit_boss_bg.PNG")      // CESSPIT_BOSS_BG
    };

    std::array<GLuint, geometry_count> vertex_buffers;
    std::array<GLuint, geometry_count> index_buffers;
    std::array<Mesh, geometry_count> meshes;

public:

    static RenderSystem& instance() {
        static RenderSystem instance;
        return instance;
    }

    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void setGameStateManager(GameStateManager* gsm);
    GameStateManager* getGameStateManager() const;
    bool initOpenGL(int width, int height, const std::string& title);
    void renderLoop();
    void closeWindow();
    GLFWwindow* getWindow() const;
    int getWindowWidth() const;
    int getWindowHeight() const;
    void loadPlayerMeshes(Entity playerEntity);
    const Mesh& getPlayerMesh(Entity playerEntity, PlayerState state);

    void cleanup();
    GLuint loadTexture(const std::string& filePath, int& outWidth, int& outHeight);
    void drawEntity(const Sprite& sprite, const TransformComponent& transform);

private:
    RenderSystem();

    void loadShaders();
    void setupVertices();
    std::string readShaderFile(const std::string& filePath);

    GLFWwindow* window;
    GLuint shaderProgram;
    GLuint VAO, VBO, EBO;
    glm::mat4 projection;
    GLuint projectionLoc;

    int windowWidth;
    int windowHeight;

    static void keyCallbackRedirect(GLFWwindow* wnd, int key, int scancode, int action, int mods);
    static void mouseMoveCallbackRedirect(GLFWwindow* wnd, double xpos, double ypos);
    static void mouseClickCallbackRedirect(GLFWwindow* wnd, int button, int action, int mods);

    GameStateManager* gameStateManager;
    PhysicsSystem physics; // remove when physics is move to GameState
};

extern RenderSystem& renderSystem;
