//
// Created by konstantinos on 7/30/25.
//

#ifndef SHORTCUTMANAGER_HPP
#define SHORTCUTMANAGER_HPP

#include <functional>
#include <unordered_map>
#include <SDL3/SDL.h>

extern std::unordered_map<int, std::function<void()>> shortcuts;

extern void registerShortcut(int key, std::function<void()>& callback);
extern void handleShortcuts(const SDL_Event* event);

#endif //SHORTCUTMANAGER_HPP
