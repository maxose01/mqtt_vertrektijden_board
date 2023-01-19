#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "MyFont.h"
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <ESPAsyncWebServer.h>
#include "index_html.h"
#include "success_html.h"
#include "secrets.h"

#define BIGFONT &FreeMonoBold24pt7b
#define MIDBIGFONT &FreeMonoBold18pt7b
#define MIDFONT &FreeMonoBold12pt7b
#define SMALLFONT &FreeMonoBold9pt7b
#define TINYFONT &TomThumb

// Replace the next variables with your SSID/Password combination
// const char* ssid = WIFI_SSID;
// const char* password = WIFI_PASSWORD;

const char* apName = "Vertrektijdenbord_001";
const char* apPassword = AP_PASSWORD;

AsyncWebServer server(80);

const char delimiter = ',';
String dir_one;
String dir_two;

const char* mqtt_server = MQTT_SERVER_HOST;
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI display = TFT_eSPI(135, 240);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, apPassword);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
    String ssid = request->getParam("ssid")->value();
    String password = request->getParam("password")->value();

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
  });

  server.begin();
  while (WiFi.status() != WL_CONNECTED) {
    display.begin();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setCursor(0, 20);
    display.setTextColor(TFT_YELLOW, TFT_YELLOW);
    display.setFreeFont(SMALLFONT);
    display.println("Waiting for wifi...");
    delay(1000);
    Serial.println("Waiting for wifi credentials...");
  }
  Serial.print("Connected with wifi! IP = ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  timeClient.setTimeOffset(3600); // time offset for GMT+1
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("departures/valkenboslaan");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  print_on_builtin_display(messageTemp);
  Serial.println();
}

void print_on_builtin_display(String departure)
{
  display.fillScreen(TFT_BLACK);
  display.setCursor(0, 20);
  display.setTextColor(TFT_YELLOW, TFT_YELLOW);
  display.setFreeFont(SMALLFONT);
  display.println("L Bestemming       V");
  display.setFreeFont(MIDFONT);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  // Split the string into an array of substrings
  int index = departure.indexOf(delimiter);
  if (index != -1) {
    dir_one = departure.substring(0, index);
    dir_two = departure.substring(index + 1);
  } else {
    // If the delimiter is not found, the original string is assigned to the first variable
    dir_one = departure;
    dir_two = "";
  }
  display.println(dir_one);
  display.println("-----------------");
  display.println(dir_two);
  display.println("-----------------");
  display.println(getCurrentTime());
}

String getCurrentTime() {
    time_t now = timeClient.getEpochTime();
    struct tm * timeinfo;
    char buffer[80];
    timeinfo = gmtime(&now);
    strftime(buffer, sizeof(buffer), "%d-%m-%y %H:%M:%S", timeinfo);
    return buffer;
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  timeClient.update();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
