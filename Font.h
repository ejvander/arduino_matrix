#ifndef FONT_H
#define FONT_H

#include <Arduino.h>
#include <ArduinoSTL.h>

void rotate90(uint8_t *columns, uint8_t size, uint8_t idx, uint8_t *rows);

struct FontData {
  FontData(){};

public:
  uint8_t size;
  uint8_t data[8];
};

extern const uint8_t _sysfont[];

void loadFont(FontData *font);

#endif