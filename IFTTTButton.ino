/*
 * IFTTT Button for ESP8266.
 * Sketch will wake up and send an ifttt message, then go back to sleep.
 */

#include <ESPWebConfig.h> // https://github.com/mnomn/ESPWebConfig

const char* KEY = "Maker key";
const char* EVENT = "Event name";

ESP8266WebServer server(80);
// int resetPin = 4; // Wemos D2, Arduino 4
int resetPin = 0; // Good for ESP01
String parameters[] = {KEY, EVENT};
ESPWebConfig espConfig(resetPin, NULL, parameters, 2);
unsigned long lastButtonTime = 0;
#define MODE_CALLING 0
#define MODE_RESETTING 1
#define MODE_CONFIG 2
#define MODE_CALL_COMPLETED 2
int mode = -1;

void callIFTTT() {
  WiFiClient client;
  const char *host = "maker.ifttt.com";
  Serial.println(F("Connect to host"));
  if (client.connect(host, 80)) {
    char *key = espConfig.getParameter(KEY);
    char *event = espConfig.getParameter(EVENT);
    String header = "POST /trigger/";
    header += event;
    header += "/with/key/";
    header += key;
    header += " HTTP/1.1\r\nHost: ";
    header += host;
    header += "\r\nConnection: close\r\n\r\n" +

    Serial.println(F("Sending a request"));
    Serial.println(header);
    client.println(header);

    Serial.println(F("Read response"));
    // Read response
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    Serial.println(F("Done!"));
    client.stop();
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(1);
  }
  // espWebConfig is configuring the button as input
  Serial.println(F("Starting ..."));
  espConfig.setHelpText("Parameters for IFTTT Maker <i>https://ifttt.com/maker</i>");
  if (espConfig.setup(server)) {
    Serial.println(F("Normal boot"));
    Serial.println(WiFi.localIP());
    mode = MODE_CALLING;
  } else {
    Serial.println(F("Config mode"));
    mode = MODE_CONFIG;
  }
  server.begin();
}

void loop() {
  espConfig.checkReset();
  server.handleClient();
  if (LOW == digitalRead(resetPin)) {
    mode = MODE_RESETTING;
  }
  if (mode == MODE_CALLING) {
    mode = MODE_CALL_COMPLETED;
    callIFTTT();
    delay(10);
    ESP.deepSleep(0);
  }
}
