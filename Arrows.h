#ifndef ARROWS_H_
#define ARROWS_H_
#include <stdint.h>


class Arrows {
private:
    uint8_t x_coord;
    uint8_t y_coord;
    const uint16_t *image; //pointer to bitmap array
    uint8_t h, w;          //height/width
    uint8_t button;        //corresponding input value
    uint8_t target;        //out of bounds = 0, bad = 1, good = 2, great = 3
    bool remove;

public:
    Arrows();
    Arrows(uint8_t x_coord, const uint16_t *image, uint8_t w, uint8_t h, uint8_t button);
    Arrows(Arrows &other);
    Arrows& operator=(const Arrows& other);
    bool move(void); //returns true if arrows is completely off screen, and false if still on screen
    void target_zone(void);
    bool pressed();
    void draw(void);
    int get_score(void);
    bool get_remove_state();
};





#endif /* ARROWS_H_ */
