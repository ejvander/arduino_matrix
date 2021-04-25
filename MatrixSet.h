#ifndef MATRIX_SET_H
#define MATRIX_SET_H

#include "Matrix.h"

#define OP_DECODEMODE 9
#define OP_INTENSITY 10
#define OP_SCANLIMIT 11
#define OP_SHUTDOWN 12
#define OP_DISPLAYTEST 15

class MatrixSet {
public:
  MatrixSet(uint8_t num_matrices, uint8_t load_pin);

  void sendToAll(byte opcode, byte data);

  void setIntensity(int intensity);

  void loadValueEnd(FontData in);

  void shiftLeft();

  void writeToDisplay();

  bool isBufferEmpty();

private:
  uint8_t mNumMatrices;
  std::vector<Matrix> mMatrices;
  uint8_t mLoadPin;
};

#endif