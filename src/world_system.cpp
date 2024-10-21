#include <iostream>
#include "world_system.hpp"

#include <iomanip>

#include "pause_state.hpp"
#include "cesspit_map.hpp"
#include "collision_system.h"
#include "ai_system.h"
#include "region_factory.hpp"

WorldSystem::WorldSystem() {
    regionManager = std::make_unique<RegionManager>();
}

WorldSystem::~WorldSystem() {
	cleanup();
}

void WorldSystem::init() {
    // Create a new entity and register it in the ECSRegistry
    m_player = Entity();
    m_sword = Entity();
    regionManager->setRegion(makeRegion<Cesspit>);

    // Player

    // Add the Player component to the player entity
    registry.players.emplace(m_player, Player());

    // Create and initialize a Motion component for the player
    Motion playerMotion;
    playerMotion.position = glm::vec2(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f);
    playerMotion.velocity = glm::vec2(0, 0);
    playerMotion.scale = { WALKING_BB_WIDTH, WALKING_BB_HEIGHT };
    registry.motions.emplace(m_player, playerMotion);

    // Add the Weapon component to the sword entity
    registry.weapons.emplace(m_sword, Weapon());

    // Create and initialize a damage component for the sword
    Damage swordDamage;
    swordDamage.damage_dealt = 1;
    registry.damages.emplace(m_sword, swordDamage);

    // Create and initialize a Health component for the player
    Health playerHealth;
    playerHealth.max_health = 3;
    playerHealth.current_health = 3;
    registry.healths.emplace(m_player, playerHealth);

    // Create the HealthFlask for the player to heal with
    HealthFlask healthFlask;
    registry.healthFlasks.emplace(m_player, healthFlask);

    // Add gravity to the Player
    registry.gravity.emplace(m_player, Gravity());

    // Add Combat to Player
    registry.combat.emplace(m_player, Combat());

    // Create and initialize the Animation component

    Animation<PlayerState> playerAnimations(IDLE);
    std::vector<Sprite> idleSprite;
    std::vector<Sprite> walkingSprites;
    std::vector<Sprite> jumpingSprites;
    std::vector<Sprite> attackingSprites;

    registry.bounding_box.emplace(m_player);

    for (unsigned i = 1; i <= 4; i++) {
        int playerWidth, playerHeight;
        GLuint playerTextureID = renderSystem.loadTexture("walk_" + std::to_string(i) + ".png", playerWidth, playerHeight);
        Sprite sprite(playerTextureID);
        walkingSprites.push_back(sprite);
        if (i == 3) {
            idleSprite.push_back(sprite);
        }

        //Adding Bounding Box to the entities
        BoundingBox x = registry.bounding_box.get(m_player);
        x.height = sprite.height;
        x.width = sprite.width;
    }

    for (unsigned i = 1; i <= 4; i++) {
        int playerWidth, playerHeight;
        GLuint jumpTextureID = renderSystem.loadTexture("jump_" + std::to_string(i) + ".png", playerWidth, playerHeight);
        Sprite jumpSprite(jumpTextureID);
        jumpingSprites.push_back(jumpSprite);
    }

    for (unsigned i = 1; i <= 5; i++) {
        int playerWidth, playerHeight;
        GLuint attackTextureID = renderSystem.loadTexture("attack_" + std::to_string(i) + ".png", playerWidth, playerHeight);
        Sprite attackSprite(attackTextureID);
        attackingSprites.push_back(attackSprite);
    }

    playerAnimations.addState(PlayerState::WALKING, std::move(walkingSprites));
    playerAnimations.addState(PlayerState::IDLE, std::move(idleSprite));
    playerAnimations.addState(PlayerState::JUMPING, std::move(jumpingSprites));
    playerAnimations.addState(PlayerState::ATTACKING, std::move(attackingSprites));
    registry.playerAnimations.emplace(m_player, std::move(playerAnimations));


    // Create and initialize a TransformComponent for the player
    TransformComponent playerTransform;
    playerTransform.position = glm::vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f, 0.0f);
    playerTransform.scale = glm::vec3(WALKING_BB_WIDTH, WALKING_BB_HEIGHT, 1.0f);
    playerTransform.rotation = 0.0f;
    registry.transforms.emplace(m_player, playerTransform);

    // MANDY LOOK
    // Ground:
    // sprite for ground, move this elsewhere for optimization. It is here for testing
    regionManager->init();

    // Create and initialize the Heart sprites

    std::vector<Sprite> heartSprites;
    for (unsigned i = 0; i <= 3; i++) {
        int heartWidth, heartHeight;
        GLuint heartTextureID = renderSystem.loadTexture("heart_" + std::to_string(i) + ".png", heartWidth, heartHeight);
        Sprite heartSprite(heartTextureID);
        heartSprites.push_back(heartSprite);
    }
    registry.heartSprites.emplace(m_hearts, std::move(heartSprites));

    // Create and initialize the a Transform component for the Heart sprites
    TransformComponent heartSpriteTransform;
    heartSpriteTransform.position = glm::vec3(250.0f, 120.0f, 0.0);
    heartSpriteTransform.scale = glm::vec3(HEARTS_WIDTH, HEARTS_HEIGHT, 1.0);
    heartSpriteTransform.rotation = 0.0f;
    registry.transforms.emplace(m_hearts, heartSpriteTransform);

    int goombaWidth, goombaHeight;
    Sprite goombaSprite(renderSystem.loadTexture("goomba_walk_idle.PNG", goombaWidth, goombaHeight));
    goombaWidth /= 4; goombaHeight /= 4;
    registry.sprites.emplace(m_goomba, goombaSprite);

    TransformComponent goombaTransform;
    registry.transforms.emplace(m_goomba, goombaTransform);
    Motion goombaMotion;
    goombaMotion.position = vec2(renderSystem.getWindowWidth() - 50, 0);
    goombaMotion.scale = vec2(goombaWidth, goombaHeight);
    registry.motions.emplace(m_goomba, goombaMotion);
    registry.gravity.emplace(m_goomba, Gravity());
    registry.patrol_ais.emplace(m_goomba, Patrol_AI());
    registry.damages.emplace(m_goomba, Damage{ 1 });
    registry.healths.emplace(m_goomba, Health{ 1,1 });
}

void WorldSystem::update(float deltaTime) {
    static PlayerState lastState = PlayerState::WALKING; // Track the player's last state
    // Loop through all entities that have motion components
    for (auto entity : registry.motions.entities) {
        if (registry.transforms.has(entity) && registry.motions.has(entity)) {
            auto& t = registry.transforms.get(entity);
            auto& m = registry.motions.get(entity);

            // Step 1: Apply gravity if not grounded
            if (registry.gravity.has(entity)) {
                auto& g = registry.gravity.get(entity);
                m.velocity.y += g.accleration;
            }

            // Step 2: Update position based on velocity
            if (registry.players.has(entity)) {
                // Make the player's position stop once its head reaches the top of the window
                if ((m.position[1] + m.velocity[1]) > 150) {
                    m.position += m.velocity;
                }
                else {
                    // Makes sure the player starts to drop immiediately cuz of gravity
                    m.velocity[1] = 0;
                }
            }
            else {
                m.position += m.velocity;
            }
            

            // If this is the player, reset canJump before handling collisions
            if (entity == m_player) {
                canJump = false;  // Only the player can jump
            }

            // Step 3: Prevent falling out of the screen for all entities
            if (m.position[1] > window_height_px) {
                m.position[1] = window_height_px;
                m.velocity.y = 0;

                // Only the player can be grounded and jump
                if (entity == m_player) {
                    isGrounded = true;
                    canJump = true;
                }
            }

            m.position[0] = clamp(m.position[0], 0, window_width_px);

            // Step 4: Flip the texture based on movement direction for all entities
            if (m.velocity[0] < 0) {
                m.scale.x = -std::abs(m.scale.x);
            }
            else if (m.velocity[0] > 0) {
                m.scale.x = std::abs(m.scale.x);
            }

            // Player-specific logic
            if (entity == m_player && registry.playerAnimations.has(m_player) && registry.combat.has(m_player)) {
                auto& a = registry.playerAnimations.get(m_player);
                auto& c = registry.combat.get(m_player);

                // Attack handling
                if (c.frames > 0 && !canAttack) {
                    c.frames -= 1;
                }
                else {
                    canAttack = true;
                }

                // Step 6: Handle player state (JUMPING, WALKING, ATTACKING)
                PlayerState currentState = a.getState();
                if (c.frames > 0 && !canAttack) {
                    currentState = PlayerState::ATTACKING;
                } else if (m.velocity[0] != 0) {
                    currentState = PlayerState::WALKING;
                } else if (!isGrounded) {
                    currentState = PlayerState::JUMPING;
                } else {
                    currentState = PlayerState::IDLE;
                }

                // Step 7: Update bounding box size based on state
                switch (currentState) {
                case WALKING:
                case IDLE:
                    m.scale = vec2(WALKING_BB_WIDTH * signof(m.scale.x), WALKING_BB_HEIGHT);
                    break;
                case JUMPING:
                    m.scale = vec2(JUMPING_BB_WIDTH * signof(m.scale.x), JUMPING_BB_HEIGHT);
                    break;
                case ATTACKING:
                    m.scale = vec2(ATTACKING_BB_WIDTH * signof(m.scale.x), ATTACKING_BB_HEIGHT);
                    break;
                }

                // Step 8: Update the player animation state if it has changed
                if (currentState != lastState) {
                    a.setState(currentState);
                    lastState = currentState;
                }
                else {
                    if ((a.currentState == JUMPING && !isGrounded) ||
                        (a.currentState == WALKING && m.velocity[0] != 0) ||
                        (a.currentState == IDLE && m.velocity[0] == 0) ||
                        (a.currentState == ATTACKING)) {
                        a.next(deltaTime);  // Advance the animation frame
                    }
                }

                if (currentState == PlayerState::ATTACKING && a.isAnimationComplete()) {
                    // Reset attack state and set the player back to IDLE or WALKING
                    c.frames = 0;  // Reset attack frames
                    canAttack = true;  // Allow another attack
                    currentState = isGrounded ? PlayerState::IDLE : PlayerState::WALKING;  // Switch back to IDLE or WALKING
                    a.setState(currentState);  // Update animation state
                    registry.players.get(m_player).attacking = false;
                }
            }
            // Step 5: Update the transform component for all entities
            t = m;
        }
    }
    // Handle collisions
    handle_collisions();

    std::vector<Entity> to_remove;
    for (auto& e : registry.invinciblityTimers.entities) {
        auto& i = registry.invinciblityTimers.get(e);
        i.counter_ms -= deltaTime * 1000;
        if (i.counter_ms <= 0) {
            to_remove.push_back(e);
        }
    }

    for (auto& e : to_remove) {
        registry.invinciblityTimers.remove(e);
    }

    AISystem::step(m_player);
    for (auto& e : registry.patrol_ais.entities) {
        auto& p = registry.patrol_ais.get(e);
        if (registry.motions.has(e)) {
            auto& m = registry.motions.get(e);
            if (std::abs(m.position.x - renderSystem.getWindowWidth()) < 10) {
                p.movingRight = false;
            } else if (std::abs(m.position.x - 0) < 10) {
                p.movingRight = true;
            }
            if (p.movingRight) {
                if(p.chasing){
                    m.velocity.x = 3;
                }else{
                    m.velocity.x = 1;
                }
            } else {
                if(p.chasing){
                    m.velocity.x = -3;
                }else{
                    m.velocity.x = -1;
                }
            }
        }
    }

    //Update bounding boxes for all the entities
    auto & bounding_boxes = registry.bounding_box;
    for(int i = 0; i < bounding_boxes.size(); i++){
        Entity e1 = bounding_boxes.entities[i];
        updateBoundingBox(e1);
    }
}

void WorldSystem::handle_collisions() {
    auto& collisionsRegistry = registry.collisions;
    for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = collisionsRegistry.components[i].other;
        vec2 direction = collisionsRegistry.components[i].direction;
        vec2 overlap = collisionsRegistry.components[i].overlap;

        if (registry.grounds.has(entity_other)) {
            Motion& thisMotion = registry.motions.get(entity);
            Motion& otherMotion = registry.motions.get(entity_other);

            if (direction.x != 0) {
                thisMotion.position.x -= overlap.x * direction.x;
            }
            else if (direction.y > 0 && thisMotion.velocity.y > 0) {
                thisMotion.position.y -= overlap.y;
                thisMotion.velocity.y = 0;
                if (registry.players.has(entity)) {
                    canJump = true;  // Allow player to jump again
                    isGrounded = true;  // Player is grounded
                }
            }
            else if (direction.y < 0 && thisMotion.velocity.y < 0) {
                thisMotion.position.y += overlap.y;
                thisMotion.velocity.y = 0;
            }

        }
        if (registry.players.has(entity) && registry.damages.has(entity_other) && !registry.invinciblityTimers.has(entity)) {
            if(registry.players.get(m_player).attacking){
                hostile_get_damaged(m_goomba);
                registry.players.get(m_player).attacking = false;
            }else{
                player_get_damaged(entity_other);
            }
        }
        if (registry.weapons.has(entity) && registry.healths.has(entity_other)) {
            if (registry.players.get(m_player).attacking) {
                hostile_get_damaged(entity_other);
            }
        }
    }

    registry.collisions.clear();
}


bool WorldSystem::checkPlayerGroundCollision() {
    auto& playerMotion = registry.motions.get(m_player);
    for (auto& groundEntity : registry.collisions.entities) {
        auto& groundMotion = registry.motions.get(groundEntity);
        // Assuming Y direction is down, check if player is at or below ground level
        if (playerMotion.position[1] >= groundMotion.position[1]) {
            isGrounded = true;
            canJump = true;  // Allow player to jump after landing
            return true;
        }
    }
    isGrounded = false;
    canJump = false;  // Prevent jumping mid-air
    return false;
}

void WorldSystem::render() {
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the ground entity if it exists and has the required components
    for (auto& obj : registry.envObject.entities) {
        if (registry.transforms.has(obj) && registry.sprites.has(obj))
        {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
        }
    }

    // Draw the player entity if it exists and has the required components
    if (registry.playerAnimations.has(m_player) &&
        registry.transforms.has(m_player))
    {
        auto& animation = registry.playerAnimations.get(m_player);
        auto& transform = registry.transforms.get(m_player);
        renderSystem.drawEntity(animation.getCurrentFrame(), transform);
    }

    // Draw health
    if (registry.transforms.has(m_hearts) && registry.sprites.has(m_hearts))
    {
        auto& transform = registry.transforms.get(m_hearts);
        auto& sprite = registry.sprites.get(m_hearts);
        renderSystem.drawEntity(sprite, transform);
    }


    if (registry.transforms.has(m_hearts) && registry.heartSprites.has(m_hearts))
    {
        auto& health = registry.healths.get(m_player);
        update_heartSprite(health.current_health);
    }


    if (registry.sprites.has(m_goomba) && registry.transforms.has(m_goomba)) {
        auto& s = registry.sprites.get(m_goomba);
        auto& t = registry.transforms.get(m_goomba);
        renderSystem.drawEntity(s, t);
    }
}

void WorldSystem::processPlayerInput(int key, int action) {
    // Escape key to close the window
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        renderSystem.getGameStateManager()->pauseState(std::make_unique<PauseState>());
    }

    // Move left (A key)
    if (action == GLFW_PRESS && key == GLFW_KEY_A) {
        if (registry.motions.has(m_player)) {
            registry.motions.get(m_player).velocity[0] = -player_speed;

        }
    }

    // Move right (D key)
    if (action == GLFW_PRESS && key == GLFW_KEY_D) {
        if (registry.motions.has(m_player)) {
            registry.motions.get(m_player).velocity[0] = player_speed;
        }
    }

    // Stop leftward movement (release A key)
    if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
        if (registry.motions.has(m_player)) {
            auto& motion = registry.motions.get(m_player);
            if (motion.velocity[0] < 0) {  // Only stop leftward movement
                motion.velocity[0] = 0;
            }
        }
    }

    // Stop rightward movement (release D key)
    if (action == GLFW_RELEASE && key == GLFW_KEY_D) {
        if (registry.motions.has(m_player)) {
            auto& motion = registry.motions.get(m_player);
            if (motion.velocity[0] > 0) {  // Only stop rightward movement
                motion.velocity[0] = 0;
            }
        }
    }

    // Jump (Space key)
    if (action == GLFW_PRESS && (key == GLFW_KEY_SPACE || key == GLFW_KEY_W)) {
        if (registry.motions.has(m_player)) {
            auto& playerMotion = registry.motions.get(m_player);
            if (canJump) {  // Ensure the player can only jump if grounded
                playerMotion.velocity[1] = -player_jump_velocity;  // Apply jump velocity
                canJump = false;  // Prevent further jumps mid-air
                isGrounded = false;
            }
        }
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_S) {
        if (registry.motions.has(m_player)) {
            auto& playerMotion = registry.motions.get(m_player);
            playerMotion.velocity[1] += player_speed * 2.0f; // Increase downward velocity
        }
    }

    // Press H to heal the player
    if (action == GLFW_PRESS && key == GLFW_KEY_H) {
        player_get_healed();
    }

    // Press P to respawn the goomba
    if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        respawnGoomba();
    }
}

void WorldSystem::on_key(int key, int, int action, int) {
    processPlayerInput(key, action);
}

void WorldSystem::on_mouse_move(const glm::vec2&) {
}

void WorldSystem::on_mouse_click(int button, int action, const glm::vec2&, int) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (registry.combat.has(m_player)) {
            if (canAttack) {  // Ensure the player can attack
                // make a call to bounding boxes here
                std::cout << "is attacking" << std::endl;
                canAttack = false;  // Prevent further attacks for a time
                auto& c = registry.combat.get(m_player);
                c.frames = c.max_frames;
                registry.players.get(m_player).attacking = true;
            }
        }
    }
}

void WorldSystem::cleanup() {
    // Remove all components of the player entity from the registry
    registry.remove_all_components_of(m_player);
}

void WorldSystem::player_get_damaged(Entity hostile) {
    Health& player_health = registry.healths.get(m_player);
    Damage hostile_damage = registry.damages.get(hostile);
    // Make sure to give the player i-frames so that they dont just die from walking into a goomba
    registry.invinciblityTimers.emplace(m_player);

    if (player_health.current_health > 0) {
        player_health.current_health -= hostile_damage.damage_dealt;
        update_heartSprite(player_health.current_health);
    }
}

void WorldSystem::player_get_healed() {
    Health& player_health = registry.healths.get(m_player);
    HealthFlask& health_flask = registry.healthFlasks.get(m_player);

    if (health_flask.num_uses > 0 && player_health.max_health > player_health.current_health) {
        player_health.current_health++;
        health_flask.num_uses--;
        update_heartSprite(player_health.current_health);
        printf("You have %d uses of your health flask left \n", health_flask.num_uses);
    }
    else if (player_health.max_health == player_health.current_health){
        printf("You have full health \n");
    }
    else {
        printf("You have no more uses of your health flask \n");
    }
}

void WorldSystem::hostile_get_damaged(Entity hostile) {
    if (registry.healths.has(hostile)) {
        Health& hostile_health = registry.healths.get(hostile);
        Damage sword_damage = registry.damages.get(m_sword);
        if (hostile_health.current_health > 0) {
            hostile_health.current_health--;
            if (hostile_health.current_health == 0) {
                registry.sprites.remove(hostile);
                registry.bounding_box.remove(hostile);
                Motion& hostile_motion = registry.motions.get(hostile);
                hostile_motion.velocity = {0,0};
                hostile_motion.position = hostile_motion.position + vec2(0, 50);
                registry.gravity.remove(hostile);
                registry.patrol_ais.remove(hostile);               
                registry.damages.remove(hostile);
                registry.healths.remove(hostile);
                int goombaWidth, goombaHeight;
                Sprite goombaSprite(renderSystem.loadTexture("goomba_dead.PNG", goombaWidth, goombaHeight));
                goombaWidth /= 4; goombaHeight /= 4;
                registry.sprites.emplace(hostile, goombaSprite);
            }
        }
    }
 
}

void WorldSystem::respawnGoomba() {
    // Check if the Goomba has been killed
    if (!registry.healths.has(m_goomba)) {

        registry.healths.emplace(m_goomba, Health{ 1, 1 }); // Goomba has 1 health

        int goombaWidth, goombaHeight;
        Sprite goombaSprite(renderSystem.loadTexture("goomba_walk_idle.PNG", goombaWidth, goombaHeight));
        goombaWidth /= 4; goombaHeight /= 4;
        registry.sprites.get(m_goomba) = goombaSprite;

        auto& goombaMotion = registry.motions.get(m_goomba);
        goombaMotion.position = glm::vec2(renderSystem.getWindowWidth() - 50, 0);
        goombaMotion.velocity = glm::vec2(0, 0);

        if (!registry.patrol_ais.has(m_goomba)) {
            registry.patrol_ais.emplace(m_goomba, Patrol_AI());
        }

        if (!registry.damages.has(m_goomba)) {
            registry.damages.emplace(m_goomba, Damage{ 1 });
        }

        if (!registry.gravity.has(m_goomba)) {
            registry.gravity.emplace(m_goomba, Gravity());
        }

        if (!registry.bounding_box.has(m_goomba)) {
            BoundingBox goombaBoundingBox;
            goombaBoundingBox.width = static_cast<float>(goombaWidth);
            goombaBoundingBox.height = static_cast<float>(goombaHeight);
            registry.bounding_box.emplace(m_goomba, goombaBoundingBox);
        }

        std::cout << "Goomba has respawned" << std::endl;
    }
}


void WorldSystem::update_heartSprite(int num_hearts) {
    auto& transform = registry.transforms.get(m_hearts);
    auto& heartSprites = registry.heartSprites.get(m_hearts);
    num_hearts = clamp(num_hearts, 0, static_cast<int>(heartSprites.size()));
    Sprite heartSprite = heartSprites[num_hearts];
    renderSystem.drawEntity(heartSprite, transform);
}

void WorldSystem::updateBoundingBox(Entity e1) {
    Motion& player_motion = registry.motions.get(e1);
    float box_height = player_motion.scale.y * registry.bounding_box.get(e1).height;
    float y_value_min = player_motion.position.y - box_height/2;
    float y_value_max = player_motion.position.y + box_height/2;
    float box_width = player_motion.scale.x * registry.bounding_box.get(e1).width;
    float x_value_min = player_motion.position.x - box_width/2;
    float x_value_max = player_motion.position.x + box_width/2;
    BoundingBox bounding_box = registry.bounding_box.get(e1);

    //Top Left
    bounding_box.p1.x = x_value_min;
    bounding_box.p1.y = y_value_max;

    //Bottom Left
    bounding_box.p2.x = x_value_min;
    bounding_box.p2.y = y_value_min;

    //Bottom Right
    bounding_box.p3.x = x_value_max;
    bounding_box.p3.y = y_value_min;

    //Top Right
    bounding_box.p4.x = x_value_max;
    bounding_box.p4.y = y_value_max;

}
