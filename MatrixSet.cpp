#include "MatrixSet.h"
#include <SPI.h>

MatrixSet::MatrixSet(uint8_t num_matrices, uint8_t load_pin) {
  mNumMatrices = num_matrices;
  mLoadPin = load_pin;

  for (uint8_t i = 0; i < mNumMatrices + 1; i++) {
    Matrix mat;
    mat.setAddr(i);
    mMatrices.push_back(mat);
  }

  pinMode(load_pin, OUTPUT);

  digitalWrite(load_pin, HIGH);

  sendToAll(OP_DISPLAYTEST, 0);
  sendToAll(OP_SCANLIMIT, 7);
  sendToAll(OP_DECODEMODE, 0);
  // Display default to being shutdown.  writing 1 wakes it
  sendToAll(OP_SHUTDOWN, 1);
}

void MatrixSet::sendToAll(byte opcode, byte data) {
  digitalWrite(mLoadPin, LOW);
  for (uint8_t matrix_num = 0; matrix_num < mNumMatrices; matrix_num++) {
    SPI.transfer(opcode);
    SPI.transfer(data);
  }
  digitalWrite(mLoadPin, HIGH);
}

void MatrixSet::setIntensity(int intensity) {
  sendToAll(OP_INTENSITY, intensity);
}

void MatrixSet::loadValueEnd(FontData in) {
  Matrix &buffer = mMatrices.back();
  buffer.loadValue(in);
}

void MatrixSet::shiftLeft() {
  // Define input and output rows to pass data from one
  // matrix to the next
  bool next_in[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = mNumMatrices; i >= 0; i--) {
    mMatrices[i].shiftleft(next_in, out);
    for (int j = 0; j < 8; j++) {
      next_in[j] = out[j];
    }
  }
}

void MatrixSet::writeToDisplay() {

  // Populate row data to be written to the display
  std::vector<std::vector<byte>> rows_data;
  for (uint8_t row = 0; row < 8; row++) {
    std::vector<byte> row_data;
    for (uint8_t matrix_num = 0; matrix_num < mNumMatrices; matrix_num++) {
      row_data.push_back(mMatrices[matrix_num].getRow(row));
    }
    rows_data.push_back(row_data);
  }

  for (uint8_t row_num = 0; row_num < 8; row_num++) {
    auto row_data = rows_data[row_num];

    digitalWrite(mLoadPin, LOW);
    while (row_data.size() > 0) {
      SPI.transfer(uint8_t((7 - row_num) + 1));
      SPI.transfer(uint8_t(row_data.back()));
      row_data.pop_back();
    }
    digitalWrite(mLoadPin, HIGH);
  }
}

bool MatrixSet::isBufferEmpty() { return mMatrices[mNumMatrices].isEmpty(); }