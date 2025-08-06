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
bool clickedPin = false;
FakeObject* tmpFakeObject = nullptr;

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
            SDL_FRect borderRect = {
                x - static_cast<float>(i), y - static_cast<float>(i), w + 2 * static_cast<float>(i),
                h + 2 * static_cast<float>(i)
            };
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
        if (ctrlPressed && !(mod & SDL_KMOD_CTRL)) {
            ctrlPressed = false;
            SDL_Log("Ctrl key released, multiple selection mode disabled.");
        }
    }
}

bool isWithinObject(const auto x, const auto y, const Object* obj) {
    return x >= obj->pos.x && x <= obj->pos.x + obj->w * obj->scale &&
        y >= obj->pos.y && y <= obj->pos.y + obj->h * obj->scale;
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
        SDL_Log("\n\n\n\n\n\nMouse down at (%f, %f)", mouseX, mouseY);
        hit = false;

        std::vector<Object*> objectsCopy = objects;

        for (auto it = objectsCopy.rbegin(); it != objectsCopy.rend(); ++it) {
            Object* obj = *it;

            if (std::find(objects.begin(), objects.end(), obj) == objects.end()) {
                continue;
            }

            if (obj == nullptr) continue;
            // Bounds checking
            SDL_Log("Object size (%f, %f)", obj->w, obj->h);
            // Check for pins
            // This assumes that the object is not rotated.
            if (!clickedPin) {
                for (int pin = 0; pin < obj->outputPinPos.size(); ++pin) {
                    SDL_Log("Checking pin at (%f, %f)", obj->outputPinPos[pin].x, obj->outputPinPos[pin].y);
                    if (mouseX >= obj->pos.x + (obj->outputPinPos[pin].x * obj->scale) - 20 &&
                        mouseX <= obj->pos.x + (obj->outputPinPos[pin].x * obj->scale) + 20 &&
                        mouseY >= obj->pos.y + (obj->outputPinPos[pin].y * obj->scale) - 20 &&
                        mouseY <= obj->pos.y + (obj->outputPinPos[pin].y * obj->scale) + 20) {
                        SDL_Log("Grabbed pin\n\n");
                        clickedPin = true;
                        // // Check if pin is already connected to something.
                        // // For now, we limit connections to just one per pin.
                        // if (obj->outputPins[pin] != nullptr) {
                        //     SDL_Log("Pin is already connected to something, not creating a new wire.");
                        //     clickedPin = false;
                        //     break;
                        // }
                        const auto tmpWire = new Wire(nullptr);
                        const auto tmpObj = new FakeObject(nullptr);
                        tmpFakeObject = tmpObj;
                        Object::connect(obj, tmpWire, pin, 0);
                        Object::connect(tmpWire, tmpObj, 0, 0);
                        selectedObjects.push_back(tmpObj);
                        tmpObj->dragging = true;
                        clickedObject = tmpWire; // The wire is not actually clicked, but we need to keep track of it
                    }
                }
            }
            if (!clickedPin && isWithinObject(mouseX, mouseY, obj)) {
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
                        _obj->offsetX = mouseX - _obj->pos.x;
                        _obj->offsetY = mouseY - _obj->pos.y;
                    }
                }
                break;
            }
        }
        if (!clickedPin && !hit && !ctrlPressed) {
            for (const auto obj : objects) {
                obj->selected = false;
            }
            selectedObjects.clear();
        }
        if (!hit && !clickedPin && event->button.button == SDL_BUTTON_LEFT) {
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
                    // obj->moved = true;
                    obj->pos.x = event->motion.x - obj->offsetX;
                    obj->pos.y = event->motion.y - obj->offsetY;
                }
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (selectionRectActive && event->button.button == SDL_BUTTON_LEFT) {
            SDL_Log("Stop selection rectangle");

            // Select objects within the rectangle
            const float minX = std::min(selectionRectStartX, selectionRectEndX);
            const float maxX = std::max(selectionRectStartX, selectionRectEndX);
            const float minY = std::min(selectionRectStartY, selectionRectEndY);
            const float maxY = std::max(selectionRectStartY, selectionRectEndY);

            for (auto obj : objects) {
                SDL_Log("Object at (%f, %f) with scale %f", obj->pos.x, obj->pos.y, obj->scale);
                // Bounds checking
                if (obj->pos.x >= minX && obj->pos.x + obj->w * obj->scale <= maxX &&
                    obj->pos.y >= minY && obj->pos.y + obj->h * obj->scale <= maxY) {
                    SDL_Log("Selected object");
                    obj->selected = true;
                    selectedObjects.push_back(obj);
                }
            }
        }

        if (clickedPin) {
            SDL_Log("Pin was clicked, checking for snapping.");
            bool snapped = false;
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            // If possible, snap to pin
            for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
                Object* obj = *it;
                if (obj == nullptr) continue;

                // Skip FakeObjects
                if (dynamic_cast<FakeObject*>(obj)) continue;

                // Bounds checking
                SDL_Log("Object size (%f, %f)", obj->w, obj->h);
                // Check for pins
                // This assumes that the object is not rotated.

                for (int pin = 0; pin < obj->inputPinPos.size(); ++pin) {
                    SDL_Log("Checking pin at (%f, %f)", obj->inputPinPos[pin].x, obj->inputPinPos[pin].y);


                    if (mouseX >= obj->pos.x + (obj->inputPinPos[pin].x * obj->scale) - 20 &&
                        mouseX <= obj->pos.x + (obj->inputPinPos[pin].x * obj->scale) + 20 &&
                        mouseY >= obj->pos.y + (obj->inputPinPos[pin].y * obj->scale) - 20 &&
                        mouseY <= obj->pos.y + (obj->inputPinPos[pin].y * obj->scale) + 20) {
                        SDL_Log("Snapped to pin\n\n");

                        // // Check if pin is already connected to something.
                        // // For now, we limit connections to just one per pin.
                        // if (obj->inputPins[pin] != nullptr) {
                        //     SDL_Log("Pin is already connected to something, not creating a new connection.");
                        //     clickedPin = false;
                        //     selectedObjects.clear();
                        //     break;
                        // }

                        // Check if the user is trying to connect to the same object
                        // clickedObject is the wire, so the actual object is the first input pin
                        if (std::find(clickedObject->inputPins[0].begin(), clickedObject->inputPins[0].end(), obj) != clickedObject->inputPins[0].end()) {
                            SDL_Log("Clicked object is the same as the object being snapped to, not creating a new connection.");
                            clickedPin = false;
                            selectedObjects.clear();
                            break;
                        }

                        if (tmpFakeObject != nullptr) {
                            delete tmpFakeObject;
                            tmpFakeObject = nullptr;
                        }

                        Object::connect(clickedObject, obj, 0, pin);

                        clickedPin = false;
                        selectedObjects.clear();
                        snapped = true;
                    }
                }
            }

            if (!snapped) {
                // We assume that the wire's input and output pins are only connected to one object
                for (auto &outputPins : clickedObject->inputPins[0][0]->outputPins) {
                    std::erase(outputPins, clickedObject);
                }
                Object *tmpObj = clickedObject->outputPins[0][0];
                delete clickedObject->outputPins[0][0];
                std::erase(clickedObject->outputPins[0], tmpObj);
                delete clickedObject;
                clickedObject = nullptr;
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
                        if (auto* btn = dynamic_cast<Button*>(clickedObject)) {
                            SDL_Log("Clicked on a button.");
                            btn->state = !btn->state;
                            eventQueue.push(clickedObject);
                            clickedObject->queued = true;
                            btn->selected = false;
                        }
                        else {
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
                // obj->moved = false;
            }
        }
        else if (selectionRectActive) {
            selectionRectActive = false;
        }

        clickedPin = false;
        break;
    default:
        break;
    }
}
