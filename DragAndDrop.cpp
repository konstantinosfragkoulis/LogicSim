//
// Created by konstantinos on 7/6/25.
//

#include <SDL3/SDL.h>
#include "Simulator.hpp"
#include "DragAndDrop.hpp"

#include <cmath>

bool ctrlPressed = false;
Uint64 clickTime = 0;
Object* clickedObject = nullptr;
bool clickedObjectPrevState = false;
bool hit = false;

// Selection rectangle
bool selectionRectActive = false;
float selectionRectStartX = 0.0f, selectionRectStartY = 0.0f;
float selectionRectEndX = 0.0f, selectionRectEndY = 0.0f;

void drawSelectionRect(SDL_Renderer* renderer) {
    if (selectionRectActive) {
        SDL_GetMouseState(&selectionRectEndX, &selectionRectEndY);
        const float x = std::min(selectionRectStartX, selectionRectEndX);
        const float y = std::min(selectionRectStartY, selectionRectEndY);
        const float w = std::abs(selectionRectEndX - selectionRectStartX);
        const float h = std::abs(selectionRectEndY - selectionRectStartY);

        SDL_FRect selectionRect = {x, y, w, h};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 100); // semi-transparent blue
        SDL_RenderFillRect(renderer, &selectionRect);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        for (int i = 0; i < 2; ++i) {
            SDL_FRect borderRect = {x - static_cast<float>(i), y - static_cast<float>(i), w + 2 * static_cast<float>(i), h + 2 * static_cast<float>(i)};
            SDL_RenderRect(renderer, &borderRect);
        }

        SDL_Log("Selection rectangle from (%f, %f) to (%f, %f)", selectionRectStartX, selectionRectStartY,
                selectionRectEndX, selectionRectEndY);
    }
}

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

bool isWithinObject(const auto x, const auto y, const Object* obj) {
    return x >= obj->x && x <= obj->x + obj->width * obj->scale &&
           y >= obj->y && y <= obj->y + obj->height * obj->scale;
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
        hit = false;

        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            Object* obj = *it;
            // Bounds checking
            SDL_Log("Object size (%f, %f)", obj->width, obj->height);
            if (isWithinObject(mouseX, mouseY, obj)) {
                hit = true;
                clickedObject = obj;
                if (!ctrlPressed) {
                    clickedObjectPrevState = clickedObject->selected;
                    if (!clickedObject->selected) {
                        for (const auto _obj : objects) {
                            _obj->selected = false;
                            _obj->dragging = false;
                        }
                        clickedObject->selected = true;
                    }
                    clickedObject->selected = true;
                }
                else {
                    clickedObjectPrevState = clickedObject->selected;
                    clickedObject->selected = true;
                }

                // Drag
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
        if (!hit && event->button.button == SDL_BUTTON_LEFT) {
            SDL_Log("Start selection rectangle at (%f, %f)", mouseX, mouseY);
            selectionRectStartX = mouseX;
            selectionRectStartY = mouseY;
            selectionRectActive = true;
            selectionRectEndX = mouseX;
            selectionRectEndY = mouseY;
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        if (!selectionRectActive) {
            for (const auto obj : selectedObjects) {
                if (obj->dragging) {
                    obj->moved = true;
                    obj->x = event->motion.x - obj->offsetX;
                    obj->y = event->motion.y - obj->offsetY;
                }
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (selectionRectActive && event->button.button == SDL_BUTTON_LEFT) {
            SDL_Log("Stop selection rectangle");
            // selectionRectActive = false;

            // Select objects within the rectangle
            const float minX = std::min(selectionRectStartX, selectionRectEndX);
            const float maxX = std::max(selectionRectStartX, selectionRectEndX);
            const float minY = std::min(selectionRectStartY, selectionRectEndY);
            const float maxY = std::max(selectionRectStartY, selectionRectEndY);

            for (auto obj : objects) {
                SDL_Log("Object at (%f, %f) with scale %f", obj->x, obj->y, obj->scale);
                // Bounds checking
                if (obj->x >= minX && obj->x + obj->width * obj->scale <= maxX &&
                    obj->y >= minY && obj->y + obj->height * obj->scale <= maxY) {
                    SDL_Log("Selected object");
                    obj->selected = true;
                    selectedObjects.push_back(obj);
                }
            }
        }

        SDL_Log("Mouse up.");
        if (hit && clickedObject) {
            SDL_Log("Hit and clicked object.");
            if (SDL_GetTicks() - clickTime < 150) { // Short click
                SDL_Log("Short click detected.");
                if (!ctrlPressed) { // Ctrl is not pressed
                    SDL_Log("Ctrl was not pressed.");
                    if (selectedObjects.size() == 1) { // Only one object is selected
                        SDL_Log("Only one object is selected.");
                        if (auto *btn = dynamic_cast<Button*>(clickedObject)) {
                            SDL_Log("Clicked on a button.");
                            btn->isPressed = !btn->isPressed;
                            btn->selected = false;
                        } else {
                            clickedObject->selected = !clickedObjectPrevState;
                        }
                    }
                    else {
                        SDL_Log("Multiple objects are selected.");
                        for (const auto obj : selectedObjects) {
                            obj->selected = false;
                        }
                        clickedObject->selected = true;
                    }
                }
                else { // Ctrl is pressed
                    SDL_Log("Ctrl was pressed.");
                    clickedObject->selected = !clickedObjectPrevState;
                }
            }
            else { // Long click
                SDL_Log("Long click detected.");
                if (!ctrlPressed) { // Ctrl is not pressed
                    // Keep the selected objects selected
                }
                else { // Ctrl is pressed
                    // Keep the selected objects selected
                }
            }

            // Stop dragging
            for (const auto obj : selectedObjects) {
                obj->dragging = false;
            }
        }
        else if ((!hit && !ctrlPressed) && !selectionRectActive) {
            for (const auto obj : objects) {
                obj->selected = false;
                obj->dragging = false;
                obj->moved = false;
            }
        } else if (selectionRectActive) {
            selectionRectActive = false;
        }
        break;
    default:
        break;
    }
}
