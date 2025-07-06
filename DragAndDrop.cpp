//
// Created by konstantinos on 7/6/25.
//

#include <SDL3/SDL.h>
#include "Simulator.hpp"
#include "DragAndDrop.hpp"

bool ctrlPressed = false;
Uint64 clickTime = 0;
Object* clickedObject = nullptr;

/**
 * @brief Checks for Ctrl key presses to correctly handle multiple selections.
 * @param event Pointer to the SDL event to check for Ctrl press or release.
 */
void checkCtrlPress(const SDL_Event* event) {
    const SDL_Keymod mod = SDL_GetModState();

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (mod & SDL_KMOD_CTRL) {
            ctrlPressed = true;
            SDL_Log("Ctrl key pressed, multiple selection mode enabled.");
        }
    }
    else if (event->type == SDL_EVENT_KEY_UP) {
        if (!(mod & SDL_KMOD_CTRL)) {
            ctrlPressed = false;
            SDL_Log("Ctrl key released, multiple selection mode disabled.");
        }
    }
}

/**
 * @brief Handles drag and drop events for objects in the simulation.
 * @param event Pointer to the SDL event to handle drag and drop actions.
 */
void handleDragAndDrop(const SDL_Event* event) {
    switch (event->type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        checkCtrlPress(event);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        clickTime = SDL_GetTicks();
        SDL_Log("Mouse down at (%f, %f)", mouseX, mouseY);

        bool hit = false;
        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            Object* obj = *it;
            if (mouseX >= obj->x && mouseX <= obj->x + 50 * obj->scale &&
                mouseY >= obj->y && mouseY <= obj->y + 50 * obj->scale) {
                hit = true;
                clickedObject = obj;
                if (!ctrlPressed && selectedObjects.size() < 2) {
                    for (auto _obj : objects) {
                        _obj->selected = false;
                    }
                    selectedObjects.clear();
                }
                if (selectedObjects.size() < 2) {
                    obj->selected = !obj->selected;
                }
                else {
                    // If multiple objects are selected, keep the object selected
                    // Later we will check if the click was short, and thus we need to
                    // remove the selection.
                    obj->selected = true;
                }
                selectedObjects.clear();
                for (auto _obj : objects) {
                    if (_obj->selected) {
                        selectedObjects.push_back(_obj);
                        _obj->dragging = true;
                        _obj->offsetX = mouseX - _obj->x;
                        _obj->offsetY = mouseY - _obj->y;
                    }
                }
                break;
            }
        }
        if (!hit && !ctrlPressed) {
            for (const auto obj : objects) {
                obj->selected = false;
            }
            selectedObjects.clear();
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        for (const auto obj : selectedObjects) {
            if (obj->dragging) {
                obj->x = event->motion.x - obj->offsetX;
                obj->y = event->motion.y - obj->offsetY;
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (ctrlPressed && SDL_GetTicks() - clickTime < 60) {
            SDL_Log("Short click detected, toggling selection.");
            // If the click was short, toggle the selection of the clicked object
            if (clickedObject) {
                clickedObject->selected = false;
                clickedObject->dragging = false;
                for (auto _obj : selectedObjects) {
                    _obj->dragging = false;
                }
                // Remove the clicked object from the selected objects

                selectedObjects.clear();

                for (auto _obj : objects) {
                    if (_obj->selected) {
                        selectedObjects.push_back(_obj);
                    }
                }
                clickTime = 0;
                clickedObject = nullptr;
            }
        }

        for (const auto obj : selectedObjects) {
            obj->dragging = false;
            if (!ctrlPressed) {
                obj->selected = false;
            }
        }
        break;
    default:
        break;
    }
}
