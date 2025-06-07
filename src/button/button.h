#ifndef BUTTON_H
#define BUTTON_H

#include <LovyanGFX.hpp>
#include <Arduino.h>
#include "../conf_WT32SCO1-Plus.h"

class Button {
public:
    int x, y, w, h;
    String label;
    uint16_t color;

    Button(int x, int y, int w, int h, String label, uint16_t color = TFT_WHITE);

    void draw(LGFX& tft) const;
    bool isClicked(uint16_t touchX, uint16_t touchY) const;
};

#endif