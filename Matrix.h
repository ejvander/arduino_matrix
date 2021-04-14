#ifndef MATRIX_H
#define MATRIX_H

#include "Font.h"
#include "LedControl.h"

class Matrix {

public:
  Matrix();

  void setAddr(int addr) { mAddr = addr; }

  void loadValue(FontData in);

  void printValue();

  void writeToDisplay(LedControl *lc);

  void shiftleft(bool row_in[8], bool row_out[8]);

  bool isEmpty() { return mSize == 0; }

private:
  bool mValues[8][8];
  uint8_t mSize;
  uint8_t mAddr;
};

#endif