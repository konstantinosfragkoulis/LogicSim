//
// Created by konstantinos on 7/30/25.
//

#include <SDL3/SDL.h>
#include "ShortcutManager.hpp"

// void handleShortcuts(const SDL_Event* event) {
//     switch (event->type) {
//     case SDL_EVENT_KEY_DOWN: {
//         const SDL_Keycode key = event->key.key;
//         const SDL_Scancode scancode = event->key.scancode;
//         if (!event->key.repeat) SDL_Log("Key down: %s (%d)", SDL_GetKeyName(key), scancode);
//         break;
//     }
//     default:
//         break;
//     }
// }

ShortcutManager &ShortcutManager::instance() {
    static ShortcutManager instance;
    return instance;
}

SDL_Keymod ShortcutManager::normalizeMods(SDL_Keymod mod) const {
    SDL_Keymod mask = SDL_KMOD_NONE;
    if (mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL))   mask = static_cast<SDL_Keymod>(mask | SDL_KMOD_CTRL);
    if (mod & (SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT)) mask = static_cast<SDL_Keymod>(mask | SDL_KMOD_SHIFT);
    if (mod & (SDL_KMOD_LALT | SDL_KMOD_RALT))     mask = static_cast<SDL_Keymod>(mask | SDL_KMOD_ALT);
    if (mod & (SDL_KMOD_LGUI | SDL_KMOD_RGUI))     mask = static_cast<SDL_Keymod>(mask | SDL_KMOD_GUI);
    return mask;
}

void ShortcutManager::registerShortcut(Shortcut shortcut, Callback callback) {
    shortcut.mod = normalizeMods(shortcut.mod);
    shortcuts[shortcut] = std::move(callback);
}

bool ShortcutManager::unregisterShortcut(Shortcut shortcut) {
    shortcut.mod = normalizeMods(shortcut.mod);
    return shortcuts.erase(shortcut) > 0;
}

bool ShortcutManager::processEvent(const SDL_Event &event) {
    if (event.type != SDL_EVENT_KEY_DOWN) return false;
    if (event.key.repeat) return false;

    const SDL_Keycode key = event.key.key;
    const SDL_Keymod mod = event.key.mod;

    Shortcut _shortcut{key, normalizeMods(mod)};
    auto it = shortcuts.find(_shortcut);
    if (it != shortcuts.end()) {
        if (it->second) it->second();
        return true;
    }

    if (mod != SDL_KMOD_NONE) {
        Shortcut _withoutMod{key, SDL_KMOD_NONE};
        it = shortcuts.find(_withoutMod);
        if (it != shortcuts.end()) {
            if (it->second) it->second();
            return true;
        }
    }

    return false;
}
