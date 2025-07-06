//
// Created by konstantinos on 7/5/25.
//

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <SDL3/SDL.h>
#include <vector>

class Object;

enum InputDeviceType { BUTTON, SWITCH };

enum GateType { NOT, AND, OR, NAND, NOR, XOR, XNOR };

extern std::vector<Object*> objects; // Global vector to hold all objects in the simulation
extern std::vector<Object*> selectedObjects; // Global vector to hold selected objects

class Object {
public:
    double x, y; // Position of the object
    double rotation; // Rotation angle in radians
    double scale; // Scale factor
    std::vector<Object*> inputPins; // Input pins for the object
    std::vector<Object*> outputPins; // Output pins for the object

    bool selected;
    bool dragging;
    bool moved;
    double offsetX, offsetY;

    explicit Object(const double x = 0.0, const double y = 0.0, const double rotation = 1.0, const double scale = 1.0) {
        this->x = x;
        this->y = y;
        this->rotation = rotation;
        this->scale = scale;
        selected = false;
        dragging = false;
        moved = false;
        offsetX = 0.0;
        offsetY = 0.0;
    }
    virtual ~Object() = default;

    virtual bool evaluate() = 0; // Pure virtual function to evaluate the object
    virtual void render(SDL_Renderer* renderer) = 0; // Pure virtual function to render the object

    static void connect(Object* src, Object* dest, const int outputPin = 0, const int inputPin = 0) {
        src->outputPins[outputPin] = dest;
        dest->inputPins[inputPin] = src;
    }

};

class Button : public Object {
public:
    bool isPressed = false;
    explicit Button(const double x = 0.0, const double y = 0.0): Object(x, y) {}

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

class Gate : public Object {
public:
    GateType type;
    explicit Gate(const GateType type, const double x = 0.0, const double y = 0.0): Object(x, y), type(type) {
        inputPins.resize(type == NOT ? 1 : 2);
        outputPins.resize(1);
    }

    bool evaluate() override;
    void render(SDL_Renderer* renderer) override;
};

#endif //SIMULATOR_HPP
