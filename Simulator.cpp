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

Object::Object(const float x, const float y, const float rotation, const float scale) {
    this->state = false;
    this->queued = false;
    this->pos = {x, y};
    this->rot = rotation;
    this->scale = scale;
    this->w = 0.0;
    this->h = 0.0;
    this->selected = false;
    this->dragging = false;
    this->offsetX = 0.0;
    this->offsetY = 0.0;

    objects.push_back(this);
}

void Object::connect(Object* src, Object* dest, const int outputPin, const int inputPin) {
    // SDL_Log("Connecting 2 objects.");
    // If dest is a wire
    if (auto* wire = dynamic_cast<Wire*>(dest)) {
        // SDL_Log("Dest is a wire!");
        wire->outputPin = outputPin;
    }
    if (auto* wire = dynamic_cast<Wire*>(src)) {
        // SDL_Log("Src is a wire!");
        wire->inputPin = inputPin;
    }
    src->outputPins[outputPin] = dest;
    dest->inputPins[inputPin] = src;
    eventQueue.push(src); // Maybe it's not necessary, and we can just push dest.
    src->queued = true;
}


Button::Button(SDL_Renderer* renderer, const float x, const float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(0);
    outputPins.resize(1, nullptr);
    inputPinPos.resize(0);
    outputPinPos.resize(1);
    textures.resize(2);

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
    this->w = w;
    this->h = h;

    outputPinPos[0] = {this->w - 20, h / 2};
}

Button::~Button() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Button::eval() {
    return true; // Always consider the button's state as changed
}

void Button::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x - 4;
        border.y = pos.y - 4;
        border.w = w * scale + 8;
        border.h = h * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture* texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {pos.x, pos.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}

Gate::Gate(SDL_Renderer* renderer, const GateType type, const float x, const float y) : Object(x, y, 1.0, 0.05),
    type(type) {
    const bool isSingleInput = (type == NOT || type == BUF);
    inputPins.resize(isSingleInput ? 1 : 2);
    inputPinPos.resize(isSingleInput ? 1 : 2);
    outputPins.resize(1);
    outputPinPos.resize(1);
    textures.resize(1);

    const std::string path = "../Assets/" + GateTypeToString(type) + ".png";
    textures[0] = IMG_LoadTexture(renderer, path.c_str());
    if (!textures[0]) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", SDL_GetError());
        return;
    }
    float w, h;
    SDL_GetTextureSize(textures[0], &w, &h);
    this->w = w;
    this->h = h;
    if (isSingleInput) {
        inputPinPos[0] = {20, h / 2};
    }
    else {
        constexpr float k = 10.0f / 45.0f;
        constexpr float k2 = 35.0f / 45.0f;
        inputPinPos[0] = {20, k * h};
        inputPinPos[1] = {20, k2 * h};
    }
    outputPinPos[0] = {w - 20, h / 2};
}

Gate::~Gate() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Gate::eval() {
    const bool prevState = this->state;
    switch (type) {
    case BUF:
        state = inputPins[0]->state;
        break;
    case NOT:
        state = !inputPins[0]->state;
        break;
    case AND:
        state = inputPins[0]->state && inputPins[1]->state;
        break;
    case OR:
        state = inputPins[0]->state || inputPins[1]->state;
        break;
    case NAND:
        state = !(inputPins[0]->state && inputPins[1]->state);
        break;
    case NOR:
        state = !(inputPins[0]->state || inputPins[1]->state);
        break;
    case XOR:
        state = inputPins[0]->state != inputPins[1]->state;
        break;
    case XNOR:
        state = inputPins[0]->state == inputPins[1]->state;
        break;
    default:
        return false;
    }
    return (state != prevState);
}

void Gate::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x + 5;
        border.y = pos.y - 2;
        border.w = w * scale - 10;
        border.h = h * scale + 4;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    if (!textures[0]) {
        return;
    }

    SDL_FRect rect = {pos.x, pos.y, 0, 0};
    SDL_GetTextureSize(textures[0], &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, textures[0], nullptr, &rect);
}

Wire::Wire(SDL_Renderer* renderer, const float x, const float y) : Object(x, y, 1.0, 1.0) {
    inputPins.resize(1);
    inputPinPos.resize(1);
    outputPins.resize(1);
    outputPinPos.resize(1);
    w = 100;
    h = 5;
    rot = 0;
    inputPin = 0;
    outputPin = 0;

    inputPinPos[0] = {0, h / 2};
    outputPinPos[0] = {w, h / 2};
}

Wire::~Wire() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Wire::eval() {
    const bool prevState = this->state;
    state = inputPins[0]->state;
    return (state != prevState);
}

void Wire::render(SDL_Renderer* renderer) {
    if (state) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }

    SDL_RenderLine(renderer,
                   inputPins[0]->pos.x + inputPins[0]->outputPinPos[outputPin].x * inputPins[0]->scale,
                   inputPins[0]->pos.y + inputPins[0]->outputPinPos[outputPin].y * inputPins[0]->scale,
                   outputPins[0]->pos.x + outputPins[0]->inputPinPos[inputPin].x * outputPins[0]->scale,
                   outputPins[0]->pos.y + outputPins[0]->inputPinPos[inputPin].y * outputPins[0]->scale);
}


Led::Led(SDL_Renderer* renderer, const float x, float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(1, nullptr);
    inputPinPos.resize(1);
    outputPins.resize(0);
    outputPinPos.resize(0);
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
    this->w = w;
    this->h = h;

    inputPinPos[0] = {20, h / 2};
}

Led::~Led() {
    for (const auto texture : textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Led::eval() {
    state = inputPins[0]->state;
    return false; // LEDs don't have output pins, so there are no other objects to notify
}

void Led::render(SDL_Renderer* renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x - 4;
        border.y = pos.y - 4;
        border.w = w * scale + 8;
        border.h = h * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture* texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {pos.x, pos.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}
