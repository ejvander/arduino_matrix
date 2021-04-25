// We always have to include the library
#include "Config.h"
#include "Font.h"
#include "MatrixSet.h"

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <queue>
#include <sstream>
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

/* we always wait a bit between updates of the display */
unsigned long delaytime = 100;

FontData font[255];

MatrixSet *matrixRow;

std::queue<FontData> input_data;

ESP8266WebServer server(80);

void addToQueue(const std::string &input) {
  for (int i = 0; i < input.size(); i++) {
    input_data.push(font[input[i] - 1]);
  }
}

void addToQueue(char input) { input_data.push(font[input - 1]); }

void addToQueue(int input) { input_data.push(font[input - 1]); }

void createMatrices() { matrixRow = new MatrixSet(DEVICES, LOAD); }

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

void pendingMessages() {
  std::string output;
  output += "{\"size\": ";
  output += String(input_data.size()).c_str();
  output += "}";
  server.send(200, "text/json", output.c_str());
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
  server.on("/PENDING_MSG", pendingMessages);
  server.begin();
}

void setup() {
  SPI.begin();
  Serial.begin(115200);
  Serial.println("Beginning");

  loadFont(font);

  createMatrices();
  matrixRow->setIntensity(1);

  instantiateServer();

  addToQueue("MSFT +12      ");

  Serial.println("End Setup");
}

void writeArduinoOnMatrix() {
  // Update the displays
  matrixRow->writeToDisplay();

  // The last matrix is a "Phantom" matrix.
  // It's purpose is to load in the data which will
  // be shifted into the real matrices
  // Once it's exmpty, we'll load in the new data
  if (matrixRow->isBufferEmpty()) {
    if (!input_data.empty()) {
      FontData front = input_data.front();
      matrixRow->loadValueEnd(front);
      input_data.pop();
    }
  }

  matrixRow->shiftLeft();

  // Add a delay to slow down scrolling
  delay(delaytime);
}

void loop() {
  writeArduinoOnMatrix();
  server.handleClient();
}