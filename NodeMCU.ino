/*
 * WebSocketClientSocketIO.ino
 *
 *  Created on: 06.06.2016
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ArduinoJson.h>  // https://arduinojson.org/

#include <WebSocketsClient.h>   // download and install from https://github.com/Links2004/arduinoWebSockets (Library Manager: WebSockets by Markus Sattler)
#include <SocketIOclient.h>

#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial

void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      USE_SERIAL.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      USE_SERIAL.printf("[IOc] get event: %s\n", payload);
      messageHandler(payload);
      break;
    case sIOtype_ACK:
      USE_SERIAL.printf("[IOc] get ack: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      USE_SERIAL.printf("[IOc] get error: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      USE_SERIAL.printf("[IOc] get binary: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
      hexdump(payload, length);
      break;
  }
}

void messageHandler(uint8_t* payload) {
  StaticJsonDocument<64> doc;


  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println(error.f_str());
    return;
  }

  String messageKey = doc[0];
  String json = doc[1];
  Serial.println(json);

  //StaticJsonDocument<64> doc2;
  error = deserializeJson(doc, json);
  JsonObject object = doc.as<JsonObject>();
  int score = object["score"];
  //Serial.println(score);

  if (messageKey == "broadcast") {
    bool ledOff = score % 5 != 0;
    digitalWrite(LED_BUILTIN, ledOff);
    sendLedStatusToServer(ledOff);
  }
}

void sendLedStatusToServer(bool ledOff) {
  // creat JSON message for Socket.IO (event)
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // add evnet name
  // Hint: socket.on('event_name', ....
  array.add("led_status");

  StaticJsonDocument<100> ledStatusDoc;
  JsonObject ledStatusObject = ledStatusDoc.to<JsonObject>();
  ledStatusObject["ledOff"] = ledOff;
  String ledStatusStr;
  serializeJson(ledStatusDoc, ledStatusStr);
  array.add(ledStatusStr);

  // JSON to String (serializion)
  String output;
  serializeJson(doc, output);

  // Send event
  socketIO.sendEVENT(output);

  // Print JSON for debugging
  USE_SERIAL.println(output);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);

  //Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  // disable AP
  if (WiFi.getMode() & WIFI_AP) {
    WiFi.softAPdisconnect(true);
  }

  WiFiMulti.addAP("Malek", "Malek@27102018");

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  String ip = WiFi.localIP().toString();
  USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

  // server address, port and URL
  //socketIO.begin("192.168.1.7", 3000, "/socket.io/?EIO=4");   // Server URL (without https://www)
  socketIO.begin("game-node-js-5412cbf208fc.herokuapp.com", 80, "/socket.io/?EIO=4");   // Server URL (without https://www)

  // event handler
  socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;
void loop() {
  socketIO.loop();

  // uint64_t now = millis();

  // if (now - messageTimestamp > 2000) {
  //   messageTimestamp = now;

  //   // creat JSON message for Socket.IO (event)
  //   DynamicJsonDocument doc(1024);
  //   JsonArray array = doc.to<JsonArray>();

  //   // add evnet name
  //   // Hint: socket.on('event_name', ....
  //   array.add("event_name");

  //   // add payload (parameters) for the event
  //   JsonObject param1 = array.createNestedObject();
  //   param1["now"] = (uint32_t)now;

  //   // JSON to String (serializion)
  //   String output;
  //   serializeJson(doc, output);

  //   // Send event
  //   socketIO.sendEVENT(output);

  //   // Print JSON for debugging
  //   USE_SERIAL.println(output);
  // }
}
