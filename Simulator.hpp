//
// Created by konstantinos on 7/5/25.
//

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <SDL3/SDL.h>
#include <vector>

class Object;

enum InputDeviceType { BUTTON, SWITCH };

enum GateType { BUF, NOT, AND, OR, NAND, NOR, XOR, XNOR };

extern std::vector<Object*> objects; // Global vector to hold all objects in the simulation
extern std::vector<Object*> selectedObjects; // Global vector to hold selected objects

class Object {
public:
    float x, y; // Position of the object
    float rotation; // Rotation angle in radians
    float scale; // Scale factor
    float width, height;
    std::vector<Object*> inputPins; // Input pins for the object
    std::vector<Object*> outputPins; // Output pins for the object

    bool selected;
    bool dragging;
    bool moved;
    float offsetX, offsetY;

    std::vector<SDL_Texture*> textures;

    explicit Object(const float x = 0.0, const float y = 0.0, const float rotation = 1.0, const float scale = 1.0) {
        this->x = x;
        this->y = y;
        this->rotation = rotation;
        this->scale = scale;
        width = 0.0;
        height = 0.0;
        selected = false;
        dragging = false;
        moved = false;
        offsetX = 0.0;
        offsetY = 0.0;
    }

    virtual ~Object() = default;

    virtual bool evaluate() = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    static void connect(Object* src, Object* dest, const int outputPin = 0, const int inputPin = 0) {
        src->outputPins[outputPin] = dest;
        dest->inputPins[inputPin] = src;
    }
};

class Button : public Object {
public:
    bool isPressed;
    explicit Button(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Button() override;

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

class Gate : public Object {
public:
    GateType type;
    explicit Gate(SDL_Renderer* renderer, GateType type, float x = 0.0, float y = 0.0);
    ~Gate() override;

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

class Wire : public Object {
public:
    explicit Wire(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Wire() override;

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

#endif //SIMULATOR_HPP
