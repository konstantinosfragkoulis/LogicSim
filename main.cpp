#define SDL_MAIN_USE_CALLBACKS 1

#include <algorithm>
#include <cstdio>
#include <ranges>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Simulator.hpp"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

std::vector<Object*> objects;
std::vector<Object*> draggingObjects;
bool ctrlPressed = false;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("Logic Sim", "1.0", "com.kfragkoulis.logicsim");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Logic Sim", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        printf("SDL_CreateWindowAndRenderer() Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto* btn1 = new Button();
    auto* btn2 = new Button();
    objects.push_back(btn1);
    objects.push_back(btn2);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    SDL_Keymod mod;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        mod = SDL_GetModState();
        if (mod & SDL_KMOD_CTRL) {
            ctrlPressed = true;
            SDL_Log("Ctrl key pressed, multiple selection mode enabled.");
        }
        break;
    case SDL_EVENT_KEY_UP:
        mod = SDL_GetModState();
        if (!(mod & SDL_KMOD_CTRL)) {
            ctrlPressed = false;
            SDL_Log("Ctrl key released, multiple selection mode disabled.");
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!ctrlPressed) {
            SDL_Log("Mouse button pressed and ctrl is not down, clearing draggin objects.");
            draggingObjects.clear();
        }
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Log("Mouse down at (%f, %f)", mouseX, mouseY);

        for (const auto obj : std::ranges::reverse_view(objects)) {
            if (mouseX >= obj->x && mouseX <= obj->x + 50 * obj->scale &&
                mouseY >= obj->y && mouseY <= obj->y + 50 * obj->scale) {
                if (obj->dragging) break; // Already dragging this object
                obj->dragging = true;
                obj->offsetX = mouseX - obj->x;
                obj->offsetY = mouseY - obj->y;
                draggingObjects.push_back(obj);
                SDL_Log("Dragging object at (%f, %f)", obj->x, obj->y);
                break;
            }
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        for (const auto obj : draggingObjects) {
            if (obj->dragging) {
                obj->x = event->motion.x - obj->offsetX;
                obj->y = event->motion.y - obj->offsetY;
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        for (const auto obj : draggingObjects) {
            if (obj->dragging) { // Should always be true
                obj->dragging = false;
            }
        }
        break;
    default:
        return SDL_APP_CONTINUE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_SetRenderDrawColorFloat(renderer, 76.0 / 255, 78.0 / 255, 82.0 / 255, SDL_ALPHA_OPAQUE_FLOAT);
    /* new color, full alpha. */

    SDL_RenderClear(renderer);

    for (const auto obj : objects) {
        obj->render(renderer);
    }

    SDL_RenderPresent(renderer);


    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
