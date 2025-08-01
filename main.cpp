#define SDL_MAIN_USE_CALLBACKS 1

#include <chrono>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "Simulator.hpp"
#include "DragAndDrop.hpp"
#include "ShortcutManager.hpp"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

std::vector<Object*> objects;
std::vector<Object*> selectedObjects;
std::queue<Object*> eventQueue;
std::unordered_map<int, std::function<void()>> shortcuts;

Uint64 lastFrameTicks = 0;
constexpr Uint64 targetFrameTime = 1000 / 125; // Target frame time for 125 FPS

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("Logic Sim", "1.0", "com.kfragkoulis.logicsim");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Logic Sim", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const auto btn1 = new Button(renderer, 10, 10);
    const auto btn2 = new Button(renderer, 100, 10);
    const auto btn3 = new Button(renderer, 200, 10);
    const auto btn4 = new Button(renderer, 300, 10);
    const auto bufGate = new Gate(renderer, BUF, 10, 150);
    const auto notGate = new Gate(renderer, NOT, 100, 150);
    const auto andGate = new Gate(renderer, AND, 200, 150);
    const auto orGate = new Gate(renderer, OR, 300, 150);
    const auto nandGate = new Gate(renderer, NAND, 400, 150);
    const auto norGate = new Gate(renderer, NOR, 500, 150);
    const auto xorGate = new Gate(renderer, XOR, 600, 150);
    const auto xnorGate = new Gate(renderer, XNOR, 700, 150);
    const auto led1 = new Led(renderer, 400, 100);
    const auto led2 = new Led(renderer, 500, 100);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    handleShortcuts(event);
    handleDragAndDrop(event);
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    default:
        return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    int steps = 0;
    constexpr int MAX_STEPS = 1000;
    while (!eventQueue.empty() && steps < MAX_STEPS) {
        Object* obj = eventQueue.front();
        eventQueue.pop();

        bool changed = obj->eval();
        if (changed) {
            for (auto* outputObj : obj->outputPins) {
                if (outputObj == nullptr) {
                    continue; // Skip if there is no output object
                }
                if (!outputObj->queued) {
                    eventQueue.push(outputObj);
                    outputObj->queued = true;
                }
            }
        }

        obj->queued = false;
        steps++;
    }

    if (steps >= MAX_STEPS) {
        SDL_Log("Warning: Maximum steps reached in event processing loop.");
    }


    SDL_SetRenderDrawColorFloat(renderer, 66.0 / 255, 67.0 / 255, 68.0 / 255, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    // Draw all objects
    for (const auto obj : objects) {
        if (obj) obj->render(renderer);
    }
    drawSelectionRect(renderer);

    SDL_RenderPresent(renderer);

    // Limit frame rate to 120 FPS
    Uint64 now = SDL_GetTicks();
    if (lastFrameTicks != 0) {
        Uint64 elapsed = now - lastFrameTicks;
        if (elapsed < targetFrameTime) {
            SDL_Delay(targetFrameTime - elapsed);
        }
    }
    lastFrameTicks = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    std::vector<Object *> objCopy = objects;
    for (const auto obj : objCopy) {
        delete obj;
    }

    objCopy.clear();
    objects.clear();
}
