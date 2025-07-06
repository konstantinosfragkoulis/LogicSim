#define SDL_MAIN_USE_CALLBACKS 1

#include <cstdio>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Simulator.hpp"
#include "DragAndDrop.hpp"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

std::vector<Object*> objects;
std::vector<Object*> selectedObjects;

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

    const auto btn1 = new Button();
    const auto btn2 = new Button();
    objects.push_back(btn1);
    objects.push_back(btn2);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    handleDragAndDrop(event);
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    default:
        return SDL_APP_CONTINUE;
    }
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
