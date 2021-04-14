// We always have to include the library
#include "Font.h"
#include "LedControl.h"
#include "Matrix.h"

#include <ArduinoSTL.h>
#include <queue>
#include <string>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <vector>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
 We have only a single MAX72XX.
 */
#define DATA_IN 51
#define CLK 52
#define LOAD 53
#define DEVICES 4
LedControl lc = LedControl(DATA_IN, CLK, LOAD, DEVICES);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 80;

FontData font[255];

std::vector<Matrix> matrices;

std::queue<FontData> input_data;

void addToQueue(std::string input) {
  for (int i = 0; i < input.size(); i++) {
    input_data.push(font[input[i] - 1]);
  }
}

void addToQueue(char input) { input_data.push(font[input - 1]); }

void addToQueue(int input) { input_data.push(font[input - 1]); }

void instantiateDisplays() {
  for (int i = 0; i < DEVICES; i++) {
    /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
    */
    lc.shutdown(i, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(i, 0);
    /* and clear the display */
    lc.clearDisplay(i);
  }
}

void createMatrices() {
  for (int i = 0; i < DEVICES + 1; i++) {
    Matrix mat;
    mat.setAddr(i);
    matrices.push_back(mat);
  }
}

void setup() {
  instantiateDisplays();

  loadFont(font);

  createMatrices();

  addToQueue("MSFT +12      ");
}

void writeArduinoOnMatrix() {
  // Update the displays
  for (int i = 0; i < DEVICES; i++) {
    matrices[i].writeToDisplay(&lc);
  }

  // The last matrix is a "Phantom" matrix.
  // It's purpose is to load in the data which will
  // be shifted into the real matrices
  // Once it's exmpty, we'll load in the new data
  if (matrices[DEVICES].isEmpty()) {
    FontData front = input_data.front();
    matrices[DEVICES].loadValue(front);
    input_data.pop();
    input_data.push(front);
  }

  // Define input and output rows to pass data from one
  // matrix to the next
  bool next_in[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = DEVICES; i >= 0; i--) {
    matrices[i].shiftleft(next_in, out);
    for (int j = 0; j < 8; j++) {
      next_in[j] = out[j];
    }
  }

  // Add a delay to slow down scrolling
  delay(delaytime);
}

void loop() { writeArduinoOnMatrix(); }