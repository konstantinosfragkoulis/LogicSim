//
// Created by konstantinos on 7/5/25.
//

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Simulator.hpp"

#include <cmath>
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

Button::Button(SDL_Renderer* renderer, const float x, const float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(0);
    outputPins.resize(1, nullptr);
    inputPinPositions.resize(0);
    outputPinPositions.resize(1);
    textures.resize(2);
    state = false;

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

    outputPinPositions[0] = {20, height / 2};
}

Button::~Button() {
    for (auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Button::evaluate() {
    return state;
}

void Button::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = position.x - 4;
        border.y = position.y - 4;
        border.w = width * scale + 8;
        border.h = height * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture* texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {position.x, position.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}

Gate::Gate(SDL_Renderer* renderer, const GateType type, const float x, const float y) : Object(x, y, 1.0, 0.05),
    type(type) {
    const bool isSingleInput = (type == NOT || type == BUF);
    inputPins.resize(isSingleInput ? 1 : 2);
    inputPinPositions.resize(isSingleInput ? 1 : 2);
    outputPins.resize(1);
    outputPinPositions.resize(1);
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
    if (isSingleInput) {
        inputPinPositions[0] = {20, height / 2};
    } else {
        inputPinPositions[0] = {20, height / 3};
        inputPinPositions[1] = {20, 2* height / 3};
    }
    outputPinPositions[0] = {width - 20, height / 2};

}

Gate::~Gate() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Gate::evaluate() {
    switch (type) {
    case BUF:
        state = inputPins[0]->evaluate();
        break;
    case NOT:
        state = !inputPins[0]->evaluate();
        break;
    case AND:
        state = inputPins[0]->evaluate() && inputPins[1]->evaluate();
        break;
    case OR:
        state = inputPins[0]->evaluate() || inputPins[1]->evaluate();
        break;
    case NAND:
        state = !(inputPins[0]->evaluate() && inputPins[1]->evaluate());
        break;
    case NOR:
        state = !(inputPins[0]->evaluate() || inputPins[1]->evaluate());
        break;
    case XOR:
        state = inputPins[0]->evaluate() != inputPins[1]->evaluate();
        break;
    case XNOR:
        state = inputPins[0]->evaluate() == inputPins[1]->evaluate();
        break;
    default:
        return false;
    }
    return state;
}

void Gate::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = position.x + 5;
        border.y = position.y - 2;
        border.w = width * scale - 10;
        border.h = height * scale + 4;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    if (!textures[0]) {
        return;
    }

    SDL_FRect rect = {position.x, position.y, 0, 0};
    SDL_GetTextureSize(textures[0], &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, textures[0], nullptr, &rect);
}

Wire::Wire(SDL_Renderer* renderer, const float x, const float y) : Object(x, y, 1.0, 1.0) {
    inputPins.resize(1);
    inputPinPositions.resize(1);
    outputPins.resize(1);
    outputPinPositions.resize(1);
    width = 100;
    height = 5;
    rotation = 0;

    inputPinPositions[0] = {0, height / 2};
    outputPinPositions[0] = {width, height / 2};
}

Wire::~Wire() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Wire::evaluate() {
    state = inputPins[0]->evaluate();
    return state;
}

void Wire::render(SDL_Renderer* renderer) {
    if (state) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }
    SDL_RenderLine(renderer, position.x, position.y,
                   width * std::cos(rotation) + position.x,
                   width * std::sin(rotation) + position.y);
}


Led::Led(SDL_Renderer* renderer, const float x, float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(1, nullptr);
    inputPinPositions.resize(1);
    outputPins.resize(0);
    outputPinPositions.resize(0);
    textures.resize(2);
    state = false;

    const std::string path0 = "../Assets/Led0.png";
    const std::string path1 = "../Assets/Led1.png";
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

    inputPinPositions[0] = {20, height / 2};
}

Led::~Led() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Led::evaluate() {
    state = inputPins[0]->evaluate();
    return state;
}

void Led::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = position.x - 4;
        border.y = position.y - 4;
        border.w = width * scale + 8;
        border.h = height * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture* texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {position.x, position.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}
