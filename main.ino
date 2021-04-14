#include <ArduinoSTL.h>
#include <queue>
#include <string>
#include <system_configuration.h>
#include <unwind-cxx.h>

// We always have to include the library
#include "./Font.h"
#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
 We have only a single MAX72XX.
 */
#define DEVICES 4
LedControl lc = LedControl(51, 52, 53, DEVICES);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 80;
unsigned long offset = 0;

uint8_t data[32];
uint8_t local_offset = 0;

struct FontData {
  FontData(){};

public:
  uint8_t size;
  uint8_t data[8];
};

FontData font[255];

class Matrix {

public:
  Matrix() {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        values[i][j] = 0;
      }
    }
    size = 0;
  }

  void setAddr(int addr) { this->addr = addr; }

  void loadValue(FontData in) {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < in.size; col++) {
        uint8_t val = ((in.data[row] << (col)) & B11111111) >> (8 - 1);
        values[row][col] = val;
      }
    }
    size = in.size + 1;
  }

  void printValue() {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        Serial.print(values[row][col]);
      }
      Serial.print("\n");
    }
  }

  void writeToDisplay() {
    for (int row = 0; row < 8; row++) {
      uint8_t row_val = 0;
      for (int col = 0; col < 8; col++) {
        row_val |= values[row][col] << (col);
      }
      lc.setRow(this->addr, (7 - row), row_val);
    }
  }

  void shiftleft(bool row_in[8], bool row_out[8]) {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (col == 0) {
          row_out[row] = values[row][col];
        }

        if (col < 7) {
          values[row][col] = values[row][col + 1];
        } else {
          values[row][col] = row_in[row];
        }
      }
    }
    size--;
  }

  bool isEmpty() { return size == 0; }

  bool values[8][8];
  uint8_t size;
  uint8_t addr;
};

Matrix *matrices;

char str[] = "MSFT -10.2    ";
unsigned int letter_idx = 0;

std::queue<FontData> input_data;

void rotate(uint8_t *columns, uint8_t size, uint8_t idx, uint8_t *rows) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < 8; j++) {
      uint8_t val = (columns[i] << (8 - 1 - j) & B11111111) >> (8 - 1);
      rows[j] |= val << (7 - i);
    }
  }
}

void addToQueue(std::string input) {
  for (int i = 0; i < input.size(); i++) {
    input_data.push(font[input[i] - 1]);
  }
}

void addToQueue(char input) { input_data.push(font[input - 1]); }

void addToQueue(int input) { input_data.push(font[input - 1]); }

void setup() {
  Serial.begin(115200);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  for (int i = 0; i < 4; i++) {
    lc.shutdown(i, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(i, 0);
    /* and clear the display */
    lc.clearDisplay(i);
  }

  int idx = 0;
  for (int i = 0; i < 255; i++) {
    FontData new_font;
    new_font.size = _sysfont[idx++];
    uint8_t *columns = new uint8_t[new_font.size];

    uint8_t rows[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int j = 0; j < new_font.size; j++) {
      columns[j] = _sysfont[idx++];
    }
    rotate(columns, new_font.size, i, rows);
    for (int j = 0; j < 8; j++) {
      new_font.data[j] = rows[j];
    }
    font[i] = new_font;
  }

  matrices = new Matrix[DEVICES + 1];
  for (int i = 0; i < DEVICES + 1; i++) {
    matrices[i].setAddr(i);
  }

  addToQueue("I ");
  addToQueue(3);
  addToQueue(" Devan     ");
}

void writeArduinoOnMatrix() {
  for (int i = 0; i < DEVICES; i++) {
    matrices[i].writeToDisplay();
  }

  if (matrices[DEVICES].isEmpty()) {
    FontData front = input_data.front();
    matrices[DEVICES].loadValue(front);
    letter_idx = (letter_idx + 1) % strlen(str);
    input_data.pop();
    input_data.push(front);
  }

  bool next_in[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = DEVICES; i >= 0; i--) {
    matrices[i].shiftleft(next_in, out);
    for (int j = 0; j < 8; j++) {
      next_in[j] = out[j];
    }
  }

  delay(delaytime);
}

void loop() { writeArduinoOnMatrix(); }