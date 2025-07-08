//
// Created by konstantinos on 7/5/25.
//

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Simulator.hpp"
#include <string>

std::string GateTypeToString(const GateType type) {
    switch (type) {
    case BUF: return "BUF";
    case NOT: return "NOT";
    case AND: return "AND";
    case OR: return "OR";
    case NAND: return "NAND";
    case NOR: return "NOR";
    case XOR: return "XOR";
    case XNOR: return "XNOR";
    default: return "UNKNOWN";
    }
}

Button::Button(SDL_Renderer* renderer, const double x, const double y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(0);
    outputPins.resize(1);
    textures.resize(2);
    isPressed = false;

    const std::string path0 = "../Assets/Button0.png";
    const std::string path1 = "../Assets/Button1.png";
    textures[0] = IMG_LoadTexture(renderer, path0.c_str());
    textures[1] = IMG_LoadTexture(renderer, path1.c_str());
    if (!textures[0] || !textures[1]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", SDL_GetError());
        return;
    }
    float w, h;
    SDL_GetTextureSize(textures[0], &w, &h);
    this->width = w;
    this->height = h;
}

Button::~Button() {
    for (auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Button::evaluate() {
    return isPressed;
}

void Button::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = static_cast<float>(x) - 4;
        border.y = static_cast<float>(y) - 4;
        border.w = static_cast<float>(width * scale) + 8;
        border.h = static_cast<float>(height * scale) + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture* texture = isPressed ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {static_cast<float>(x), static_cast<float>(y), 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= static_cast<float>(scale);
    rect.h *= static_cast<float>(scale);

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}

Gate::Gate(SDL_Renderer* renderer, const GateType type, const double x, const double y) : Object(x, y, 1.0, 0.05), type(type) {
    inputPins.resize((type == NOT || type == BUF) ? 1 : 2);
    outputPins.resize(1);
    textures.resize(1);

    const std::string path = "../Assets/" + GateTypeToString(type) + ".png";
    textures[0] = IMG_LoadTexture(renderer, path.c_str());
    if (!textures[0]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", SDL_GetError());
        return;
    }
    float w, h;
    SDL_GetTextureSize(textures[0], &w, &h);
    this->width = w;
    this->height = h;
}

Gate::~Gate() {
    for (auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Gate::evaluate() {
    switch (type) {
    case BUF:
        return inputPins[0]->evaluate();
    case NOT:
        return !inputPins[0]->evaluate();
    case AND:
        return inputPins[0]->evaluate() && inputPins[1]->evaluate();
    case OR:
        return inputPins[0]->evaluate() || inputPins[1]->evaluate();
    case NAND:
        return !(inputPins[0]->evaluate() && inputPins[1]->evaluate());
    case NOR:
        return !(inputPins[0]->evaluate() || inputPins[1]->evaluate());
    case XOR:
        return inputPins[0]->evaluate() != inputPins[1]->evaluate();
    case XNOR:
        return inputPins[0]->evaluate() == inputPins[1]->evaluate();
    default:
        return false;
    }
}

void Gate::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = static_cast<float>(x) + 5;
        border.y = static_cast<float>(y) - 2;
        border.w = static_cast<float>(width * scale) - 10;
        border.h = static_cast<float>(height * scale) + 4;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    if (!textures[0]) {
        return;
    }

    SDL_FRect rect = {static_cast<float>(x), static_cast<float>(y), 0, 0};
    SDL_GetTextureSize(textures[0], &rect.w, &rect.h);
    rect.w *= static_cast<float>(scale);
    rect.h *= static_cast<float>(scale);

    SDL_RenderTexture(renderer, textures[0], nullptr, &rect);

}