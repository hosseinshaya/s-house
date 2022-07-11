#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define EEPROM_SIZE 16

ESP8266WebServer server;
char *ssid = "Beast";
char *wifiPassword = "13200079";

char *username = "Hossein_Shaya";
char *password = "Pw02mdEdmIe3J5GP*^6lnwI9c";

IPAddress local_IP(192, 168, 1, 195);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const int lightPinsCount = 11;
int lightPins[lightPinsCount] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10};

void setLightPinModes()
{
  for (int i = 0; i < lightPinsCount; i++)
  {
    pinMode(lightPins[i], OUTPUT);
    bool lightState;
    EEPROM.get(lightPins[i], lightState);
    digitalWrite(lightPins[i], LOW);
  }
}

void setup()
{
  EEPROM.begin(EEPROM_SIZE);

  setLightPinModes();

  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, wifiPassword);
  Serial.begin(115200);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/light", HTTP_POST, setLight);
  server.on("/light", HTTP_GET, getLights);
  server.begin();
}

void loop()
{
  server.handleClient();
}

bool authorize()
{
  if (!server.authenticate(username, password))
  {
    server.send(401, "text/plain", "401: Unauthorized");
    return false;
  }
  else
  {
    return true;
  }
}

void setLight()
{
  if (authorize())
  {
    String data = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);
    int pin = doc["key"];
    if (error || pin == NULL)
      return;
    bool power = doc["value"];
    digitalWrite(pin, power ? HIGH : LOW);
    EEPROM.put(pin, power);
    EEPROM.commit();
    server.send(204, "");
  }
}

void getLights()
{
  if (authorize())
  {
    DynamicJsonDocument docSend(200);
    for (int i = 0; i < lightPinsCount; i++)
    {
      docSend[lightPins[i]] = digitalRead(lightPins[i]);
    }
    String jsonString;
    serializeJson(docSend, jsonString);

    server.send(200, "text/plain", jsonString.c_str());
  }
}