#pragma once

#include <memory>
#include <stack>
#include "game_state.hpp"
#include <boost/optional.hpp>

class GameState;

class GameStateManager {
public:

    template<typename NewState>
    void changeState() {
        if (currentState) {
            discards.push_back(std::move(currentState));
        }

        stateCreator = [this]() {
            currentState = std::make_unique<NewState>();
            currentState->init(); 
            };
    }

    void on_key(int key, int scancode, int action, int mods);
    void on_mouse_move(const glm::vec2& position);
    void on_mouse_click(int button, int action, const glm::vec2& positoin, int mods);
    void update(float deltaTime);
    void render();

    // pause current state, go to another state, then come back to this state
    template<typename NewState>
    void pauseState() {
        if (currentState) {
            currentState->pause();
            pausedState.push(std::move(currentState));
        }
        stateCreator = [this]() {
            currentState = std::make_unique<NewState>();
            currentState->init();
            };
    }
    void resumeState();

    template<typename NewState> // clear all paused states, go to new state
    void resetPausedStates() {
        while (!pausedState.empty() && pausedState.top()) {
            discards.push_back(std::move(pausedState.top()));
            pausedState.pop();
        }
        pausedState = decltype(pausedState)();
        stateCreator = [this]() {
            currentState = std::make_unique<NewState>();
            currentState->init();
            };
    }

    void discard();
    void create();

    GameState* getCurrentState() const;

private:
    std::unique_ptr<GameState> currentState;
    std::stack<std::unique_ptr<GameState>> pausedState;
    std::vector<std::unique_ptr<GameState>> discards;
    boost::optional<std::function<void()>> stateCreator;
};
