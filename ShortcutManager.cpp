//
// Created by konstantinos on 7/30/25.
//

#include <SDL3/SDL.h>
#include "ShortcutManager.hpp"

void registerShortcut(const int key, const std::function<void()>& callback) {
    shortcuts[key] = callback;
}

void handleShortcuts(const SDL_Event* event) {
    switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
        const SDL_Keycode key = event->key.key;
        const SDL_Scancode scancode = event->key.scancode;

        SDL_Log("Key down: %s (%d)", SDL_GetKeyName(key), scancode);
        break;
    }
    default:
        break;
    }
}
