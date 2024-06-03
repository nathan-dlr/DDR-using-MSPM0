#include "Arrows.h"
#include "../inc/ST7735.h"
#include "Switch.h"
#include "images/images.h"
#include "arrow_list.h"
#include "common.h"



Arrows::Arrows() {
    x_coord = 0;
    y_coord = 0;
    image = nullptr;
    h = 0;
    w = 0;
    button = 0;
    target = 0;
    remove = false;
}


Arrows::Arrows(uint8_t x_coord, const uint16_t *image, uint8_t w, uint8_t h, uint8_t button) {
    this->x_coord = x_coord;
    this->image = image;
    this->h = h;
    this->w = w;
    this->button = button;
    y_coord = 160;
    target = 0;  //0 = out of bounds, 1 = okay, 2= good, 3 = great
    remove = false;
}


Arrows::Arrows(Arrows &other) {
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    image = other.image;
    h = other.h;
    w = other.w;
    button = other.button;
    target = other.target;
    remove = other.remove;
}


Arrows& Arrows::operator=(const Arrows& other) {
    if (this != &other) {
        this->x_coord = other.x_coord;
        this->image = other.image;
        this->h = other.h;
        this->w = other.w;
        this->button = other.button;
        this->y_coord = other.y_coord;
        this->target = other.target;
        this->remove = other.remove;
    }
    return *this;
}


bool Arrows::move() { //returns true if arrows is completely off screen, and false if still on screen
    if (!remove) {
        y_coord -= 5;
        if (y_coord < 6) {
            remove = true;
            return false;
        }
        else {
            target_zone();
            return true;
        }
    }
    return false;
}


void Arrows::target_zone() {
    if ((y_coord >= 65) || (y_coord < 15)) {
        target = 0;
    }
    else if (((55 < y_coord) && (y_coord <= 65)) || ((15 < y_coord) && (y_coord <= 20))) {
        target = 1;
    }
    else if (((40 < y_coord) && (y_coord <= 55)) || ((20 < y_coord) && (y_coord <= 35))) {
        target = 2;
    }
    else if ((35 < y_coord) && (y_coord <= 40)) {
        target = 3;
    }
}


bool Arrows::pressed() {
    if ((Key_In() & button) && (target > 0)) {
        remove = true;
        remove_arrow = true;
        return true;
    }
    else {
        return false;
    }
}


void Arrows::draw() {
    ST7735_DrawBitmap(x_coord, y_coord, image, w, h);
}


int Arrows::get_score() {
    return this->target;
}

bool Arrows::get_remove_state() {
    return this->remove;
}







