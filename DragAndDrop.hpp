//
// Created by konstantinos on 7/6/25.
//

#ifndef DRAGANDDROP_HPP
#define DRAGANDDROP_HPP

#include <SDL3/SDL.h>

extern void drawSelectionRect(SDL_Renderer* renderer);
extern void handleDragAndDrop(const SDL_Event* event, SDL_Renderer* renderer);

#endif //DRAGANDDROP_HPP
