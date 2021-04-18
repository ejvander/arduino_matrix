// We always have to include the library
#include "Config.h"
#include "Font.h"
#include "LedControl.h"
#include "Matrix.h"

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <queue>
#include <string>
#include <vector>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
 We have only a single MAX72XX.
 */
#define DATA_IN 13
#define CLK 14
#define LOAD 15
#define DEVICES 8
LedControl lc = LedControl(DATA_IN, CLK, LOAD, DEVICES);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 70;

FontData font[255];

std::vector<Matrix> matrices;

std::queue<FontData> input_data;

ESP8266WebServer server(80);

void addToQueue(const std::string &input) {
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
    lc.setIntensity(i, 15);
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

void handleRoot() {
  server.send(200, "text/html",
              "<body>"
              "<div>"
              "<form action='SEND_MSG' method='get' >"
              "<label for='message' style='font-size: 5vw'>Message</label><br>"
              "<input type='text' id='message' name='message' "
              "style='font-size: 5vw'><br>"
              "<button type='submit' style='font-size: 5vw'>Send</button>"
              "</form>"
              "</div>"
              "</body>");
}

void msgReceived() {
  if (server.arg("message") != "") {
    const std::string message(server.arg("message").c_str());
    addToQueue(message);
  }
  handleRoot();
}

void instantiateServer() {
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP Address: ");
  Serial.print(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/SEND_MSG", msgReceived);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  instantiateDisplays();

  loadFont(font);

  createMatrices();

  instantiateServer();

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
    if (!input_data.empty()) {
      FontData front = input_data.front();
      matrices[DEVICES].loadValue(front);
      input_data.pop();
    }
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

void loop() {
  writeArduinoOnMatrix();
  server.handleClient();
}