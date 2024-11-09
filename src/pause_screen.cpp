#include "pause_state.hpp"
#include "ecs_registry.hpp"
#include "splash_screen_state.hpp"

PauseState::PauseState(): timePassed(0) {}

PauseState::~PauseState() {
    PauseState::cleanup();
}

inline void PauseState::lerp(float start, float end, float t) const {
    t = clamp(t, 0.0f, 1.0f);
    registry.transforms.get(pauseScreenEntity).position[1] = start * (1 - t) + end * t;
}

void PauseState::init() {
    Sprite pauseSprite = renderSystem.loadTexture("pause_screen.png");

    registry.transforms.emplace(pauseScreenEntity, TransformComponent{
        vec3(renderSystem.getWindowWidth() / 2.0f, 0.0f, 0.0f),
        vec3(pauseSprite.width, pauseSprite.height, 1.0f), 0.f
    });
    registry.sprites.emplace(pauseScreenEntity, pauseSprite);

    MenuItem quitComponent{renderSystem.loadTexture("menu/quit_active.png"), renderSystem.loadTexture("menu/quit_inactive.png"),
    renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 200.f};
    registry.menuItems.emplace(quitEntity, quitComponent);
}

void PauseState::update(float deltaTime) {
    timePassed += deltaTime * 2.0f;
    lerp(0, renderSystem.getWindowHeight() / 2.0f, timePassed);
}

void PauseState::cleanup() {
    registry.remove_all_components_of(pauseScreenEntity);
    registry.remove_all_components_of(quitEntity);
}

void PauseState::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (registry.sprites.has(pauseScreenEntity) &&
        registry.transforms.has(pauseScreenEntity))
    {
        // Retrieve the Sprite and TransformComponent using the registry
        auto& sprite = registry.sprites.get(pauseScreenEntity);
        auto& transform = registry.transforms.get(pauseScreenEntity);

        // Use the render system to draw the entity
        renderSystem.drawEntity(sprite, transform);
    }
    for (const auto& component : registry.menuItems.components) {
        renderSystem.drawEntity(component.isPointWithin(mouse_pos) ? component.active : component.inactive,
            component.isPointWithin(mouse_pos) ? component.transformActive : component.transformInactive);
    }
}

void PauseState::on_mouse_click(int button, int action, const glm::vec2& position, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (registry.menuItems.get(quitEntity).isPointWithin(mouse_pos)) {
            renderSystem.getGameStateManager()->resetPausedStates<SplashScreenState>();
        }
    }
}
