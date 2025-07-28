//
// Created by konstantinos on 7/5/25.
//

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <SDL3/SDL.h>
#include <queue>
#include <vector>

class Object;

enum InputDeviceType { BUTTON, SWITCH };

enum GateType { BUF, NOT, AND, OR, NAND, NOR, XOR, XNOR };

typedef struct Coords {
    float x, y;
} Coords;

extern std::vector<Object*> objects; // Global vector to hold all objects in the simulation
extern std::vector<Object*> selectedObjects; // Global vector to hold selected objects
extern std::queue<Object*> eventQueue;

class Object {
public:
    bool state;
    bool queued;

    Coords pos{};
    float rot; // Rotation angle in radians, ONLY for wires
    float scale;
    float w, h;

    std::vector<Object*> inputPins; // Input pins for the object
    std::vector<Object*> outputPins; // Output pins for the object
    // Relative coordinates, not adjusted for scale or rotation
    std::vector<Coords> inputPinPos;
    // Relative coordinates, not adjusted for scale or rotation
    std::vector<Coords> outputPinPos;

    bool selected;
    bool dragging;
    float offsetX, offsetY;

    std::vector<SDL_Texture*> textures;

    explicit Object(float x = 0.0, float y = 0.0, float rotation = 0.0, float scale = 1.0);
    virtual ~Object() = default;

    virtual bool eval() = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    static void connect(Object* src, Object* dest, int outputPin = 0, int inputPin = 0);
};

class Button final : public Object {
public:
    explicit Button(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Button() override;

    bool eval() override;
    void render(SDL_Renderer* renderer) override;
};

class Gate final : public Object {
public:
    GateType type;
    explicit Gate(SDL_Renderer* renderer, GateType type, float x = 0.0, float y = 0.0);
    ~Gate() override;

    bool eval() override;
    void render(SDL_Renderer* renderer) override;
};

class Wire final : public Object {
public:
    int inputPin, outputPin;
    // inputPin refers to the input pin of the object that the wire's output pin is connected to.
    // outputPin refers to the output pin of the object that the wire's input pin is connected to.

    explicit Wire(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Wire() override;

    bool eval() override;
    void render(SDL_Renderer* renderer) override;
};

class Led final : public Object {
public:
    explicit Led(SDL_Renderer* renderer, float x = 0.0, float y = 0.0);
    ~Led() override;

    bool eval() override;
    void render(SDL_Renderer* renderer) override;
};

#endif //SIMULATOR_HPP
