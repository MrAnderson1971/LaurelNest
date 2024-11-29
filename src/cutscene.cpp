#include "cutscene.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "world_init.hpp"

OpeningCutscene::OpeningCutscene() : frameCount(0), seconds_passed(0.f), hasLoaded(false) {
    Sprite tutorialSprite(renderSystem.loadTexture("tutorial/box.PNG"));
    registry.sprites.emplace(tutorialEntity, tutorialSprite);
    registry.transforms.emplace(tutorialEntity, TransformComponent{
        vec3(renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f, 0.f),
        vec3(tutorialSprite.width * 0.35f, tutorialSprite.height * 0.43f, 1.f), 0.f
        });

    Sprite controlSprite(renderSystem.loadTexture("tutorial/control_keys.PNG"));
    registry.sprites.emplace(control_keys, controlSprite);
    registry.transforms.emplace(control_keys, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.18f, 0.f),
        vec3(controlSprite.width * 0.3f, controlSprite.height * 0.3f, 1.f), 0.f
        });

    Sprite mouseSprite(renderSystem.loadTexture("tutorial/mouse_click.PNG"));
    registry.sprites.emplace(mouse_click, mouseSprite);
    registry.transforms.emplace(mouse_click, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.36f, 0.f),
        vec3(mouseSprite.width * 0.3f, mouseSprite.height * 0.3f, 1.f), 0.f
        });

    Sprite hSprite(renderSystem.loadTexture("tutorial/H_key.PNG"));
    registry.sprites.emplace(h_key, hSprite);
    registry.transforms.emplace(h_key, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.51f, 0.f),
        vec3(hSprite.width * 0.4f, hSprite.height * 0.4f, 1.f), 0.f
        });

    Sprite eSprite(renderSystem.loadTexture("tutorial/E_key.PNG"));
    registry.sprites.emplace(e_key, eSprite);
    registry.transforms.emplace(e_key, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.15f, renderSystem.getWindowHeight() * 0.735f, 0.f),
        vec3(eSprite.width * 0.4f, eSprite.height * 0.4f, 1.f), 0.f
        });

    Sprite qSprite(renderSystem.loadTexture("tutorial/Q_key.PNG"));
    registry.sprites.emplace(q_key, qSprite);
    registry.transforms.emplace(q_key, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.66f, renderSystem.getWindowHeight() * 0.735f, 0.f),
        vec3(qSprite.width * 0.4f, qSprite.height * 0.4f, 1.f), 0.f
        });

    renderSystem.drawEntity(registry.sprites.get(tutorialEntity), registry.transforms.get(tutorialEntity));

    renderSystem.drawEntity(registry.sprites.get(control_keys), registry.transforms.get(control_keys));
    renderSystem.renderText("keys to control player movement", window_width_px * 0.3f, window_height_px * 0.8f, 0.8f, vec3(1), mat4(1));

    renderSystem.drawEntity(registry.sprites.get(mouse_click), registry.transforms.get(mouse_click));
    renderSystem.renderText("left click mouse to attack", window_width_px * 0.25f, window_height_px * 0.62f, 0.8f, vec3(1), mat4(1));

    renderSystem.drawEntity(registry.sprites.get(h_key), registry.transforms.get(h_key));
    renderSystem.renderText("hold to restore health up to 3 times", window_width_px * 0.25f, window_height_px * 0.48f, 0.8f, vec3(1), mat4(1));

    renderSystem.renderText("after defeating the Flame Chicken", window_width_px * 0.3f, window_height_px * 0.33f, 0.6f, vec3(1), mat4(1));

    renderSystem.drawEntity(registry.sprites.get(e_key), registry.transforms.get(e_key));
    renderSystem.renderText("key to equip flamethrower", window_width_px * 0.2f, window_height_px * 0.25f, 0.8f, vec3(1), mat4(1));

    renderSystem.drawEntity(registry.sprites.get(q_key), registry.transforms.get(q_key));
    renderSystem.renderText("key to unequip", window_width_px * 0.7f, window_height_px * 0.25f, 0.8f, vec3(1), mat4(1));

    renderSystem.renderText("Loading...", window_width_px * 0.9f, window_height_px * 0.05f, 0.5f, vec3(1), mat4(1));

    glfwSwapBuffers(renderSystem.getWindow());
    glfwPollEvents();

    std::array<std::future<Image>, LAST_OPENING_ANIMATION_FRAME> images;
    std::atomic<int> count;
    for (int i = 0; i < LAST_OPENING_ANIMATION_FRAME; i++) {
        images[i] = loadImageData("opening_animation/opening_" + std::to_string(i) + ".png", count);
    }

    for (int i = 0; i < LAST_OPENING_ANIMATION_FRAME; i++) {
        frames[i] = bindTexture(images[i].get());
    }
}

OpeningCutscene::~OpeningCutscene() {
    registry.remove_all_components_of(tutorialEntity);
    registry.remove_all_components_of(control_keys);
    registry.remove_all_components_of(mouse_click);
    registry.remove_all_components_of(h_key);
    registry.remove_all_components_of(e_key);
    registry.remove_all_components_of(q_key);
}

void OpeningCutscene::on_key(int, int, int action, int) {
    if (action == GLFW_PRESS && !hasLoaded) { // press any button to skip
        hasLoaded = true;
        renderSystem.getGameStateManager()->changeState<WorldSystem>();
    }
}

void OpeningCutscene::update(float deltaTime) {
    seconds_passed += deltaTime;
    if (seconds_passed > SECONDS_PER_FRAME) {
        seconds_passed = 0;
        if (++frameCount >= LAST_OPENING_ANIMATION_FRAME && !hasLoaded) {
            hasLoaded = true;
            renderSystem.getGameStateManager()->changeState<WorldSystem>();
        }
    }
}

void OpeningCutscene::render() {
    TransformComponent transform{ vec3(window_width_px / 2.f, window_height_px / 2.f, 0.f), vec3(window_width_px, window_height_px, 1.f), 0.f };
    renderSystem.drawEntity(frames[frameCount].get(), transform);
}

void OpeningCutscene::on_mouse_click(int, int, const vec2&, int) {}
void OpeningCutscene::on_mouse_move(const vec2&) {}

PickupCutscene::PickupCutscene() : frameCount(0), seconds_passed(0.f), transitionFrame(-0.5f), finishedCutscene(false) {
    static bool hasLoaded = false;
    if (!hasLoaded) {
        hasLoaded = true;
        std::array<std::future<Image>, LAST_PICKUP_ANIMATION_FRAME> images;
        std::atomic<int> count;
        for (int i = 0; i < LAST_PICKUP_ANIMATION_FRAME; i++) {
            images[i] = loadImageData("pickup_animation/pick_up_" + std::to_string(i) + ".png", count);
        }

        for (int i = 0; i < LAST_PICKUP_ANIMATION_FRAME; i++) {
            frames[i] = bindTexture(images[i].get());
        }
    }
}

void PickupCutscene::update(float deltaTime) {
    if (!finishedCutscene) {
        seconds_passed += deltaTime;
        if (seconds_passed > SECONDS_PER_FRAME) {
            seconds_passed = 0;
            if (++frameCount >= LAST_PICKUP_ANIMATION_FRAME) {
                finishedCutscene = true;
                renderSystem.captureScreen();
            }
        }
    }
    else {
        while (transitionFrame < 1.f) {
            transitionFrame += deltaTime * 2.f;
            renderSystem.doGlassBreakTransition(clamp(static_cast<int>(transitionFrame * 100), 0, 100), 100);
        }
        renderSystem.getGameStateManager()->resumeState();
    }
}

void PickupCutscene::render() {
    if (!finishedCutscene) {
        TransformComponent transform{ vec3(window_width_px / 2.f, window_height_px / 2.f, 0.f), vec3(window_width_px, window_height_px, 1.f), 0.f };
        renderSystem.drawEntity(frames[frameCount].get(), transform);
    }
}
