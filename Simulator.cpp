//
// Created by konstantinos on 7/5/25.
//

#include "Simulator.hpp"

bool Button::evaluate() {
    return isPressed;
}

void Button::render(SDL_Renderer* renderer) {
    // TODO: Add object rotation. Switch to textures.
    if (selected) {
        SDL_FRect border;
        border.x = static_cast<float>(x) - 4;
        border.y = static_cast<float>(y) - 4;
        border.w = static_cast<float>(50 * scale) + 8;
        border.h = static_cast<float>(50 * scale) + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }
    SDL_FRect buttonRect;
    buttonRect.x = static_cast<float>(x);
    buttonRect.y = static_cast<float>(y);
    buttonRect.w = static_cast<float>(50 * scale);
    buttonRect.h = static_cast<float>(50 * scale);

    SDL_FRect buttonInner;
    buttonInner.x = buttonRect.x + buttonRect.w / 4;
    buttonInner.y = buttonRect.y + buttonRect.h / 4;
    buttonInner.w = buttonRect.w / 2;
    buttonInner.h = buttonRect.h / 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &buttonRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &buttonInner);
}

bool Gate::evaluate() {
    switch (type) {
    case NOT:
        return !inputPins[0];
    case AND:
        return inputPins[0] && inputPins[1];
    case OR:
        return inputPins[0] || inputPins[1];
    case NAND:
        return !(inputPins[0] && inputPins[1]);
    case NOR:
        return !(inputPins[0] || inputPins[1]);
    case XOR:
        return inputPins[0] != inputPins[1];
    case XNOR:
        return inputPins[0] == inputPins[1];
    default:
        return false;
    }
}

void Gate::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        // TODO: Implement render function
    }
}