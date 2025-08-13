//
// Created by konstantinos on 7/30/25.
//

#ifndef SHORTCUTMANAGER_HPP
#define SHORTCUTMANAGER_HPP

#include <functional>
#include <unordered_map>
#include <SDL3/SDL.h>

struct Shortcut {
    SDL_Keycode key;
    SDL_Keymod mod;

    bool operator==(const Shortcut &other) const {
        return key == other.key && mod == other.mod;
    }
};

template<>
struct std::hash<Shortcut> {
    size_t operator()(const Shortcut &shortcut) const noexcept {
        return (static_cast<size_t>(shortcut.key) << 16) ^ static_cast<size_t>(shortcut.mod);
    }
};

class ShortcutManager {
public:
    using Callback = std::function<void()>;

    static ShortcutManager &instance();

    void registerShortcut(Shortcut shortcut, Callback callback);
    bool unregisterShortcut(Shortcut shortcut);

    bool processEvent(const SDL_Event &event);

private:
    ShortcutManager() = default;
    SDL_Keymod normalizeMods(SDL_Keymod mod) const;

    std::unordered_map<Shortcut, Callback> shortcuts;
};

#endif //SHORTCUTMANAGER_HPP
