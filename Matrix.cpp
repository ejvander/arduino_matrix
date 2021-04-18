#include "Matrix.h"
#include "Font.h"

Matrix::Matrix() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      mValues[i][j] = 0;
    }
  }
  mSize = 0;
}

void Matrix::loadValue(FontData in) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < in.size; col++) {
      uint8_t val = ((in.data[row] << (col)) & B11111111) >> (8 - 1);
      mValues[row][col] = val;
    }
  }
  mSize = in.size + 1;
}

void Matrix::printValue() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      Serial.print(mValues[row][col]);
    }
    Serial.print("\n");
  }
}

void Matrix::shiftleft(bool row_in[8], bool row_out[8]) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (col == 0) {
        row_out[row] = mValues[row][col];
      }

      if (col < 7) {
        mValues[row][col] = mValues[row][col + 1];
      } else {
        mValues[row][col] = row_in[row];
      }
    }
  }
  mSize = mSize == 0 ? 0 : mSize - 1;
}

void Matrix::writeToDisplay(LedControl *lc) {
  for (int row = 0; row < 8; row++) {
    uint8_t row_val = 0;
    for (int col = 0; col < 8; col++) {
      row_val |= mValues[row][col] << (col);
    }
    lc->setRow(this->mAddr, (7 - row), row_val);
  }
}