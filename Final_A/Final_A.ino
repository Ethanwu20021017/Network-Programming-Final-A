#include <WiFiS3.h>
#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"

ArduinoLEDMatrix matrix;
WiFiClient client;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char HOST[] = "nycu.waynewolf.tw";
const char PATH[] = "/weather/weather.do";
const int PORT = 80;
const char LOCATION[] = "taipei";

char weatherData[80];
char showMsg[120];

byte sunIcon[8][12] = {
  {0,0,1,0,0,0,0,0,0,1,0,0},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {1,0,0,0,0,1,1,0,0,0,0,1},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {1,0,0,0,0,1,1,0,0,0,0,1},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {0,0,1,0,0,0,0,0,0,1,0,0}
};

byte snowIcon[8][12] = {
  {0,0,0,0,0,1,0,1,0,0,0,0},
  {0,0,1,0,0,0,1,0,0,0,1,0},
  {0,0,0,1,0,1,1,1,0,1,0,0},
  {0,0,0,0,1,1,1,1,1,0,0,0},
  {0,0,0,1,0,1,1,1,0,1,0,0},
  {0,0,1,0,0,0,1,0,0,0,1,0},
  {0,0,0,0,0,1,0,1,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

byte warnIcon[8][12] = {
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,1,1,0,0,1,1,0,0,0},
  {0,0,1,1,0,0,0,0,1,1,0,0},
  {0,1,1,0,0,1,1,0,0,1,1,0},
  {1,1,0,0,0,1,1,0,0,0,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,0,0,1,1,0,0,0,0,0}
};

void setup() {
  Serial.begin(9600);
  matrix.begin();

  Serial.println("Scan start");
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks");

  for (int i = 0; i < n; i++) {
    Serial.println(WiFi.SSID(i));
  }

  showIcon(warnIcon);
  delay(500);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("WiFi Status = ");
    Serial.println(WiFi.status());
    delay(1000);
  }

  Serial.println();
  Serial.println("WiFi connected");
  delay(2000);
}

void loop() {
  int ok = getWeatherData(weatherData, sizeof(weatherData));
  if (!ok) {
    Serial.println("Weather service failed.");
    showIcon(warnIcon);
    delay(3000);
    return;
  }

  Serial.print("Raw weather data: ");
  Serial.println(weatherData);

  char *p1 = strchr(weatherData, ',');
  char *p2 = NULL;
  if (p1 != NULL) p2 = strchr(p1 + 1, ',');

  if (p1 == NULL || p2 == NULL) {
    Serial.println("Data format error.");
    showIcon(warnIcon);
    delay(3000);
    return;
  }

  *p1 = '\0';
  *p2 = '\0';

  char *timeText = weatherData;
  float temp = atof(p1 + 1);
  float humid = atof(p2 + 1);

  snprintf(showMsg, sizeof(showMsg), "Time:%s Temp. %.2f C Humid. %.0f%%", timeText, temp, humid);
  Serial.print("Display: ");
  Serial.println(showMsg);

  scrollText(showMsg);

  if (temp >= 25.0) {
    showIcon(sunIcon);
    delay(2000);
  } else if (temp <= 24.0) {
    showIcon(snowIcon);
    delay(2000);
  }
  delay(10000);
}

int getWeatherData(char *out, int outSize) {
  char body[50];
  snprintf(body, sizeof(body), "time=now&location=%s", LOCATION);

  if (!client.connect(HOST, PORT)) return 0;

  client.print("POST ");
  client.print(PATH);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(HOST);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(strlen(body));
  client.println("Connection: close");
  client.println();
  client.print(body);

  unsigned long t0 = millis();
  while (client.connected() && !client.available()) {
    if (millis() - t0 > 5000) {
      client.stop();
      return 0;
    }
  }

  int bodyStarted = 0;
  int index = 0;
  char last4[4] = {0, 0, 0, 0};

  while (client.available()) {
    char c = client.read();
    last4[0] = last4[1];
    last4[1] = last4[2];
    last4[2] = last4[3];
    last4[3] = c;

    if (!bodyStarted) {
      if (last4[0] == '\r' && last4[1] == '\n' && last4[2] == '\r' && last4[3] == '\n') {
        bodyStarted = 1;
      }
    } else {
      if (index < outSize - 1) {
        out[index++] = c;
      }
    }
  }
  out[index] = '\0';
  client.stop();

  trimText(out);
  if (strlen(out) == 0) return 0;
  return 1;
}

void showIcon(byte icon[8][12]) {
  matrix.renderBitmap(icon, 8, 12);
}

void scrollText(const char *text) {
  int textLen = strlen(text);
  int width = textLen * 6;
  byte frame[8][12];

  for (int shift = -12; shift < width; shift++) {
    clearFrame(frame);
    for (int y = 0; y < 7; y++) {
      for (int x = 0; x < 12; x++) {
        int realX = x + shift;
        if (getTextPixel(text, realX, y)) frame[y][x] = 1;
      }
    }
    matrix.renderBitmap(frame, 8, 12);
    delay(70);
  }
}

void clearFrame(byte frame[8][12]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 12; x++) frame[y][x] = 0;
  }
}

int getTextPixel(const char *text, int x, int y) {
  if (x < 0 || y < 0 || y > 6) return 0;

  int charIndex = x / 6;
  int col = x % 6;

  if (charIndex >= strlen(text) || col >= 5) return 0;

  return getCharColumn(text[charIndex], col) & (1 << y);
}

byte getCharColumn(char c, int col) {
  byte font[5] = {0, 0, 0, 0, 0};
  switch (c) {
    case '0': { byte f[5] = {62, 81, 73, 69, 62}; memcpy(font, f, 5); break; }
    case '1': { byte f[5] = {0, 66, 127, 64, 0}; memcpy(font, f, 5); break; }
    case '2': { byte f[5] = {98, 81, 73, 73, 70}; memcpy(font, f, 5); break; }
    case '3': { byte f[5] = {34, 65, 73, 73, 54}; memcpy(font, f, 5); break; }
    case '4': { byte f[5] = {24, 20, 18, 127, 16}; memcpy(font, f, 5); break; }
    case '5': { byte f[5] = {39, 69, 69, 69, 57}; memcpy(font, f, 5); break; }
    case '6': { byte f[5] = {60, 74, 73, 73, 48}; memcpy(font, f, 5); break; }
    case '7': { byte f[5] = {1, 113, 9, 5, 3}; memcpy(font, f, 5); break; }
    case '8': { byte f[5] = {54, 73, 73, 73, 54}; memcpy(font, f, 5); break; }
    case '9': { byte f[5] = {6, 73, 73, 41, 30}; memcpy(font, f, 5); break; }
    case 'A': { byte f[5] = {126, 17, 17, 17, 126}; memcpy(font, f, 5); break; }
    case 'C': { byte f[5] = {62, 65, 65, 65, 34}; memcpy(font, f, 5); break; }
    case 'F': { byte f[5] = {127, 9, 9, 9, 1}; memcpy(font, f, 5); break; }
    case 'H': { byte f[5] = {127, 8, 8, 8, 127}; memcpy(font, f, 5); break; }
    case 'K': { byte f[5] = {127, 8, 20, 34, 65}; memcpy(font, f, 5); break; }
    case 'O': { byte f[5] = {62, 65, 65, 65, 62}; memcpy(font, f, 5); break; }
    case 'T': { byte f[5] = {1, 1, 127, 1, 1}; memcpy(font, f, 5); break; }
    case 'W': { byte f[5] = {127, 2, 12, 2, 127}; memcpy(font, f, 5); break; }
    case 'a': { byte f[5] = {32, 84, 84, 84, 120}; memcpy(font, f, 5); break; }
    case 'e': { byte f[5] = {56, 84, 84, 84, 24}; memcpy(font, f, 5); break; }
    case 'i': { byte f[5] = {0, 68, 125, 64, 0}; memcpy(font, f, 5); break; }
    case 'm': { byte f[5] = {124, 4, 120, 4, 120}; memcpy(font, f, 5); break; }
    case 'p': { byte f[5] = {124, 20, 20, 20, 8}; memcpy(font, f, 5); break; }
    case 't': { byte f[5] = {4, 63, 68, 64, 32}; memcpy(font, f, 5); break; }
    case 'u': { byte f[5] = {60, 64, 64, 32, 124}; memcpy(font, f, 5); break; }
    case 'd': { byte f[5] = {56, 68, 68, 36, 127}; memcpy(font, f, 5); break; }
    case ':': { byte f[5] = {0, 54, 54, 0, 0}; memcpy(font, f, 5); break; }
    case '.': { byte f[5] = {0, 96, 96, 0, 0}; memcpy(font, f, 5); break; }
    case '%': { byte f[5] = {35, 19, 8, 100, 98}; memcpy(font, f, 5); break; }
    case '-': { byte f[5] = {8, 8, 8, 8, 8}; memcpy(font, f, 5); break; }
    case ' ': { byte f[5] = {0, 0, 0, 0, 0}; memcpy(font, f, 5); break; }
  }
  return font[col];
}

void trimText(char *s) {
  int len = strlen(s);
  while (len > 0 && (s[len - 1] == '\r' || s[len - 1] == '\n' || s[len - 1] == ' ')) {
    s[--len] = '\0';
  }

  int start = 0;
  while (s[start] == '\r' || s[start] == '\n' || s[start] == ' ') start++;

  if (start > 0) {
    int i = 0;
    while (s[start] != '\0') s[i++] = s[start++];
    s[i] = '\0';
  }
}