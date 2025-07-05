//
// Created by konstantinos on 7/5/25.
//

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <SDL3/SDL.h>
#include <vector>

class Object;

enum InputDeviceType {
    BUTTON,
    SWITCH
};

enum GateType {
    NOT,
    AND,
    OR,
    NAND,
    NOR,
    XOR,
    XNOR
};

extern std::vector<Object *> objects; // Global vector to hold all objects in the simulation

class Object {
public:
    double x, y; // Position of the object
    double rotation; // Rotation angle in radians
    double scale; // Scale factor
    std::vector<Object*> inputPins; // Input pins for the object
    std::vector<Object*> outputPins; // Output pins for the object

    bool dragging;
    double offsetX, offsetY;

    virtual bool evaluate() = 0; // Pure virtual function to evaluate the object
    virtual void render(SDL_Renderer* renderer) = 0; // Pure virtual function to render the object
    static void connect(Object *src, Object *dest, const int outputPin = 0, const int inputPin = 0) {
        src->outputPins[outputPin] = dest;
        dest->inputPins[inputPin] = src;
    }

    explicit Object(const double x = 0.0, const double y = 0.0, const double rotation = 1.0, const double scale = 1.0) {
        this->x = x;
        this->y = y;
        this->rotation = rotation;
        this->scale = scale;
        dragging = false;
        offsetX = 0.0;
        offsetY = 0.0;
    }

    virtual ~Object() = default;
};

class Button : public Object {
public:
    bool isPressed = false;

    bool evaluate() override {
        return isPressed;
    }
    void render(SDL_Renderer* renderer) override {
        // TODO: Add object rotation. Switch to textures.
        if (dragging) {
            SDL_FRect buttonHighlightRect;
            buttonHighlightRect.x = static_cast<float>(x)-4;
            buttonHighlightRect.y = static_cast<float>(y)-4;
            buttonHighlightRect.w = static_cast<float>(50 * scale)+8;
            buttonHighlightRect.h = static_cast<float>(50 * scale)+8;

            SDL_SetRenderDrawColor(renderer, 85, 136, 255, 255);
            SDL_RenderFillRect(renderer, &buttonHighlightRect);

        }
        SDL_FRect buttonRect;
        buttonRect.x = static_cast<float>(x);
        buttonRect.y = static_cast<float>(y);
        buttonRect.w = static_cast<float>(50 * scale);
        buttonRect.h = static_cast<float>(50 * scale);

        SDL_FRect buttonClickRect;
        buttonClickRect.x = buttonRect.x + buttonRect.w / 4;
        buttonClickRect.y = buttonRect.y + buttonRect.h / 4;
        buttonClickRect.w = buttonRect.w / 2;
        buttonClickRect.h = buttonRect.h / 2;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &buttonRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &buttonClickRect);
    }
};

class Gate : public Object {
public:
    GateType type;

    bool evaluate() override {
        switch (type) {
        case NOT:
            return !inputPins[0];
            break;
        case AND:
            return inputPins[0] && inputPins[1];
            break;
        case OR:
            return inputPins[0] || inputPins[1];
            break;
        case NAND:
            return !(inputPins[0] && inputPins[1]);
            break;
        case NOR:
            return !(inputPins[0] || inputPins[1]);
            break;
        case XOR:
            return inputPins[0] != inputPins[1];
            break;
        case XNOR:
            return inputPins[0] == inputPins[1];
            break;
        default:
            return false;
            break;
        }
    }

    explicit Gate(GateType type, const double x = 0.0, const double y = 0.0, const double rotation = 0.0,
                  const double scale = 1.0)
        : Object(x, y, rotation, scale), type(type) {
        // Initialize input and output pins based on gate type
        switch (type) {
        case NOT:
            inputPins.resize(1);
            break;
        case AND:
        case OR:
        case NAND:
        case NOR:
        case XOR:
        case XNOR:
            inputPins.resize(2);
            break;
        }
        outputPins.resize(1);
    }
};

#endif //SIMULATOR_HPP
