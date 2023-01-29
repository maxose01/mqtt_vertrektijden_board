#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
// #include <ESPAsyncWebServer.h>
#include "index_html.h"
#include "success_html.h"
#include "secrets.h"
#include <MD_MAX72xx.h>
#include "font.h"

// Define the number of devices and the SPI hardware interface
#define MAX_DEVICES 2
#define MAX_CLK_PIN   D5
#define MAX_DATA_PIN  D7
#define MAX_CS_PIN    D8

// Replace the next variables with your SSID/Password combination
// const char* ssid = WIFI_SSID;
// const char* password = WIFI_PASSWORD;

const char* apName = "Vertrektijdenbord_001";
const char* apPassword = AP_PASSWORD;

// AsyncWebServer server(80);

const char delimiter = ',';
String dir_one;
String dir_two;

const char* mqtt_server = MQTT_SERVER_HOST;
WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// // Create two LedControl objects, one for each batch of 8 matrixes
// LedControl row1=LedControl(12,11,10,1);
// LedControl row2=LedControl(12,11,10,1);

// // Create two LedMatrix-Font objects, one for each batch of 8 matrixes
// LedMatrixFont font1(row1);
// LedMatrixFont font2(row2);
MD_MAX72XX mx = MD_MAX72XX(MAX_CS_PIN, MAX_DEVICES);

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
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
  print_on_matrix(messageTemp);
  Serial.println();
}

void print_on_matrix(String departure)
{
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
}

String getCurrentTime() {
  time_t now = timeClient.getEpochTime();
  struct tm * timeinfo;
  char buffer[80];
  timeinfo = gmtime(&now);
  strftime(buffer, sizeof(buffer), "%d-%m-%y %H:%M:%S", timeinfo);
  return buffer;
}

void displayText(const char* text, uint8_t device)
{
  uint8_t c;
  
  for (int i = 0; i < strlen(text); i++)
  {
    c = text[i] - ' ';
    if (c < ' ' || c > '~') c = 0; // only printable characters
    for (int j = 0; j < 8; j++)
      mx.setColumn(device, i, j, font[c][j]);
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

   // Initialize the library
  mx.begin();
  mx.control(TEST, OFF);  // turn off test mode
  mx.control(DECODE, OFF);  // turn off decode mode
  mx.control(INTENSITY, 8);  // set the intensity

  mx.clear();
  
  // Display text on the first batch of 8 matrixes
  displayText("Hello World", 0);
  delay(1000);

}

void loop() {
  timeClient.update();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
