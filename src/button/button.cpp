#include "button.h"

Button::Button(int x, int y, int w, int h, String label, uint16_t color)
    : x(x), y(y), w(w), h(h), label(label), color(color) {}

void Button::draw(LGFX& tft) const {
    tft.fillRoundRect(x, y, w, h, 5, color);
    tft.setTextColor(TFT_WHITE);
    tft.drawCenterString(label, x + (w / 2), y + (h / 2), &fonts::DejaVu12);
}

bool Button::isClicked(uint16_t touchX, uint16_t touchY) const {
    return (touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h);
}