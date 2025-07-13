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

typedef struct Coords {
    float x, y;
} Coords;

extern std::vector<Object*> objects; // Global vector to hold all objects in the simulation
extern std::vector<Object*> selectedObjects; // Global vector to hold selected objects

class Object {
public:
    bool state;
    Coords position;
    float rotation; // Rotation angle in radians
    float scale; // Scale factor
    float width, height;
    std::vector<Object*> inputPins; // Input pins for the object
    std::vector<Object*> outputPins; // Output pins for the object
    // Relative coordinates, not adjusted for scale or rotation
    std::vector<Coords> inputPinPositions;
    // Relative coordinates, not adjusted for scale or rotation
    std::vector<Coords> outputPinPositions;

    bool selected;
    bool dragging;
    bool moved;
    float offsetX, offsetY;

    std::vector<SDL_Texture*> textures;

    explicit Object(const float x = 0.0, const float y = 0.0, const float rotation = 1.0, const float scale = 1.0) {
        this->state = false;
        this->position = {x, y};
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

class Led : public Object {
public:
    explicit Led(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Led() override;

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

#endif //SIMULATOR_HPP
