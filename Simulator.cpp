//
// Created by konstantinos on 7/5/25.
//

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Simulator.hpp"
#include "Assets/AND.hpp"
#include "Assets/BUF.hpp"
#include "Assets/NAND.hpp"
#include "Assets/NOR.hpp"
#include "Assets/NOT.hpp"
#include "Assets/OR.hpp"
#include "Assets/XOR.hpp"
#include "Assets/XNOR.hpp"
#include "Assets/Button0.hpp"
#include "Assets/Button1.hpp"
#include "Assets/Led0.hpp"
#include "Assets/Led1.hpp"
#include "Assets/CLK.hpp"

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

Object::~Object() {
    std::erase(objects, this);

    for (auto &inputPin: inputPins) {
        for (auto *connectedObj: inputPin) {
            disconnect(connectedObj);
        }
    }

    for (auto &outputPin: outputPins) {
        for (auto *connectedObj: outputPin) {
            disconnect(connectedObj);
        }
    }
}


void Object::connect(Object *src, Object *dest, const int outputPin, const int inputPin) {
    if (auto *wire = dynamic_cast<Wire *>(dest)) {
        wire->outputPin = outputPin;
    }
    if (auto *wire = dynamic_cast<Wire *>(src)) {
        wire->inputPin = inputPin;
    }
    src->outputPins[outputPin].push_back(dest);
    dest->inputPins[inputPin].push_back(src);
    eventQueue.push(src);
    eventQueue.push(dest);
    src->queued = true;
    dest->queued = true;
}

// Disconnect this from another object.
void Object::disconnect(Object *obj) {
    if (!obj) return;

    for (auto &inputPin: obj->inputPins) {
        std::erase(inputPin, this);
    }
    for (auto &outputPin: obj->outputPins) {
        std::erase(outputPin, this);
    }
}


Button::Button(SDL_Renderer *renderer, const float x, const float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(0);
    outputPins.resize(1);
    inputPinPos.resize(0);
    outputPinPos.resize(1);
    textures.resize(2);

    SDL_IOStream* btn0 = SDL_IOFromConstMem(Button0_png, Button0_png_len);
    SDL_IOStream* btn1 = SDL_IOFromConstMem(Button1_png, Button1_png_len);
    if (!btn0 || !btn1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create IOStream from memory: %s", SDL_GetError());
    }

    SDL_Surface* surface0 = IMG_Load_IO(btn0, 1);
    SDL_Surface* surface1 = IMG_Load_IO(btn1, 1);
    if (!surface0 || !surface1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image from IOStream: %s", SDL_GetError());
    }
    textures[0] = SDL_CreateTextureFromSurface(renderer, surface0);
    textures[1] = SDL_CreateTextureFromSurface(renderer, surface1);

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
    for (const auto texture: textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Button::eval() {
    return true; // Always consider the button's state as changed
}

void Button::render(SDL_Renderer *renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x - 4;
        border.y = pos.y - 4;
        border.w = w * scale + 8;
        border.h = h * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture *texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {pos.x, pos.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}


Clock::Clock(SDL_Renderer *renderer, const float x, float y, const float freq) : Object(x, y, 0, 0.05), freq(freq) {
    inputPins.resize(0);
    outputPins.resize(1);
    inputPinPos.resize(0);
    outputPinPos.resize(1);
    textures.resize(1);

    SDL_IOStream* rw = SDL_IOFromConstMem(CLK_png, CLK_png_len);
    if (!rw) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create IOStream from memory: %s", SDL_GetError());
    }

    SDL_Surface* surface = IMG_Load_IO(rw, 1);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image from IOStream: %s", SDL_GetError());
    }
    textures[0] = SDL_CreateTextureFromSurface(renderer, surface);

    float w, h;
    SDL_GetTextureSize(textures[0], &w, &h);
    this->w = w;
    this->h = h;

    outputPinPos[0] = {w - 20, h / 2};
}

Clock::~Clock() {
    for (const auto texture: textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Clock::eval() {
    static Uint64 last = 0;
    static bool lastState = false;
    Uint64 now = SDL_GetTicks();
    if (last == 0) last = now;
    float T = 1000.0f / freq / 2.0f;
    if (now - last >= T) {
        lastState = !lastState;
        last = now;
    }
    bool prevState = state;
    state = lastState;
    return state != prevState;
}

void Clock::render(SDL_Renderer *renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x - 2;
        border.y = pos.y - 2;
        border.w = w * scale - 3;
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



Gate::Gate(SDL_Renderer *renderer, const GateType type, const float x, const float y) : Object(x, y, 0, 0.05),
    type(type) {
    const bool isSingleInput = (type == NOT || type == BUF);
    inputPins.resize(isSingleInput ? 1 : 2);
    inputPinPos.resize(isSingleInput ? 1 : 2);
    outputPins.resize(1);
    outputPinPos.resize(1);
    textures.resize(1);


    SDL_IOStream* rw = nullptr;
    switch (type) {
        case BUF:
            rw = SDL_IOFromConstMem(BUF_png, BUF_png_len);
            break;
        case NOT:
            rw = SDL_IOFromConstMem(NOT_png, NOT_png_len);
            break;
        case AND:
            rw = SDL_IOFromConstMem(AND_png, AND_png_len);
            break;
        case OR:
            rw = SDL_IOFromConstMem(OR_png, OR_png_len);
            break;
        case NAND:
            rw = SDL_IOFromConstMem(NAND_png, NAND_png_len);
            break;
        case NOR:
            rw = SDL_IOFromConstMem(NOR_png, NOR_png_len);
            break;
        case XOR:
            rw = SDL_IOFromConstMem(XOR_png, XOR_png_len);
            break;
        case XNOR:
            rw = SDL_IOFromConstMem(XNOR_png, XNOR_png_len);
            break;
    }
    
    if (!rw) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create IOStream from memory: %s", SDL_GetError());
    }

    SDL_Surface* surface = IMG_Load_IO(rw, 1);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image from IOStream: %s", SDL_GetError());
    }
    textures[0] = SDL_CreateTextureFromSurface(renderer, surface);

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
    } else {
        constexpr float k = 10.0f / 45.0f;
        constexpr float k2 = 35.0f / 45.0f;
        inputPinPos[0] = {20, k * h};
        inputPinPos[1] = {20, k2 * h};
    }
    outputPinPos[0] = {w - 20, h / 2};
}

Gate::~Gate() {
    for (const auto texture: textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

static bool evalPin(const std::vector<Object*>& pins) {
    bool ret = false;
    for (const auto pin: pins) {
        ret |= pin->state;
    }
    return ret;
}

bool Gate::eval() {
    const bool prevState = this->state;
    // This assumes only two input pins
    // For custom gates this code needs to change
    if (inputPins[0].empty()) return false;
    if (!(type == NOT || type == BUF) && inputPins[1].empty()) return false;

    switch (type) {
        case BUF:
            state = evalPin(inputPins[0]);
            break;
        case NOT:
            state = !evalPin(inputPins[0]);
            break;
        case AND:
            state = evalPin(inputPins[0]) && evalPin(inputPins[1]);
            break;
        case OR:
            state = evalPin(inputPins[0]) || evalPin(inputPins[1]);
            break;
        case NAND:
            state = !(evalPin(inputPins[0]) && evalPin(inputPins[1]));
            break;
        case NOR:
            state = !(evalPin(inputPins[0]) || evalPin(inputPins[1]));
            break;
        case XOR:
            state = evalPin(inputPins[0]) != evalPin(inputPins[1]);
            break;
        case XNOR:
            state = evalPin(inputPins[0]) == evalPin(inputPins[1]);
            break;
        default:
            return false;
    }
    return (state != prevState);
}

void Gate::render(SDL_Renderer *renderer) {
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

Wire::Wire(SDL_Renderer *renderer, const float x, const float y) : Object(x, y, 1.0, 1.0) {
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
    for (const auto texture: textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Wire::eval() {
    const bool prevState = this->state;
    state = evalPin(inputPins[0]);
    return (state != prevState);
}

void Wire::render(SDL_Renderer *renderer) {
    if (selected) {
        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
    } else if (state) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }

    // if (outputPins[0] == nullptr || inputPins[0] == nullptr) {
    //     return; // No connection, nothing to render
    // }

    if (inputPins[0].empty() || outputPins[0].empty()) return;

    // Technically wires are Objects so they could have multiple other Objects
    // connected to their input and output pins. This, of course, is not intended
    // and should not happen. We assume that wires are only connected to one object.
    SDL_RenderLine(renderer,
                   inputPins[0][0]->pos.x + inputPins[0][0]->outputPinPos[outputPin].x * inputPins[0][0]->scale,
                   inputPins[0][0]->pos.y + inputPins[0][0]->outputPinPos[outputPin].y * inputPins[0][0]->scale,
                   outputPins[0][0]->pos.x + outputPins[0][0]->inputPinPos[inputPin].x * outputPins[0][0]->scale,
                   outputPins[0][0]->pos.y + outputPins[0][0]->inputPinPos[inputPin].y * outputPins[0][0]->scale);
}


Led::Led(SDL_Renderer *renderer, const float x, float y) : Object(x, y, 1.0, 0.05) {
    inputPins.resize(1);
    inputPinPos.resize(1);
    outputPins.resize(0);
    outputPinPos.resize(0);
    textures.resize(2);
    state = false;

    SDL_IOStream* led0 = SDL_IOFromConstMem(Led0_png, Led0_png_len);
    SDL_IOStream* led1 = SDL_IOFromConstMem(Led1_png, Led1_png_len);
    if (!led0 || !led1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create IOStream from memory: %s", SDL_GetError());
    }

    SDL_Surface* surface0 = IMG_Load_IO(led0, 1);
    SDL_Surface* surface1 = IMG_Load_IO(led1, 1);
    if (!surface0 || !surface1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image from IOStream: %s", SDL_GetError());
    }
    textures[0] = SDL_CreateTextureFromSurface(renderer, surface0);
    textures[1] = SDL_CreateTextureFromSurface(renderer, surface1);

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
    for (const auto texture: textures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool Led::eval() {
    state = evalPin(inputPins[0]);
    return false; // LEDs don't have output pins, so there are no other objects to notify
}

void Led::render(SDL_Renderer *renderer) {
    if (selected) {
        SDL_FRect border;
        border.x = pos.x - 4;
        border.y = pos.y - 4;
        border.w = w * scale + 8;
        border.h = h * scale + 8;

        SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
        SDL_RenderFillRect(renderer, &border);
    }

    SDL_Texture *texture = state ? textures[1] : textures[0];

    if (!texture) {
        return;
    }

    SDL_FRect rect = {pos.x, pos.y, 0, 0};
    SDL_GetTextureSize(texture, &rect.w, &rect.h);
    rect.w *= scale;
    rect.h *= scale;

    SDL_RenderTexture(renderer, texture, nullptr, &rect);
}


FakeObject::FakeObject(SDL_Renderer *renderer, float x, float y) {
    inputPins.resize(1);
    outputPins.resize(1);
    inputPinPos.resize(1);
    outputPinPos.resize(1);
    inputPinPos[0] = {0, 0};
    outputPinPos[0] = {0, 0};
}

bool FakeObject::eval() {
    return false;
}

void FakeObject::render(SDL_Renderer *renderer) {
    // No rendering logic for fake objects
}
