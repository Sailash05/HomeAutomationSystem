#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <SPI.h>
#include <Wire.h>
#include <Servo.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define DHTTYPE DHT22
#define DHTPIN 5
DHT dht(DHTPIN, DHTTYPE);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_FT6206 touch = Adafruit_FT6206();
bool doorState = true;
bool lightState = true;
Servo myServo;
#define TRIG_PIN 6
#define ECHO_PIN 7
long duration;
float distance;
float prevDistance;
bool motionState = false;
float prevTemperature, currentTemperature;
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define MQ2_PIN A0
int threshold = 400;
bool blinkerState = false;

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_WHITE);
  if (!touch.begin(10)) {
    Serial.println("Touch screen not found");
    while (1);
  }
  introScreen();
  displayMain();
  myServo.attach(3);
  doorController();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);
  dht.begin();
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  pinMode(12, OUTPUT);
}

void loop() {
  int gasValue = analogRead(MQ2_PIN);
  if (gasValue > threshold) {
    blinkerState = true;
  } else {
    blinkerState = false;
  }
  blinker();
  currentTemperature = dht.readTemperature();
  if (isnan(currentTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  acController();
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  doorController();
  int motionDetected = digitalRead(2);
  if (motionDetected == HIGH && !motionState) {
    motionState = true;
    lightState = false;
    digitalWrite(4, HIGH);
    toggleLight();
  } else if(motionDetected == LOW && motionState) {
    motionState = false;
    lightState = true;
    digitalWrite(4, LOW);
    toggleLight();
  }
  prevTemperature = currentTemperature;
  prevDistance = distance;
  delay(200);
}

void blinker() {
  if(blinkerState) {
    digitalWrite(12, HIGH);
    delay(200);
    digitalWrite(12, LOW);
  }
}

void acController() {
  if(prevTemperature == currentTemperature) return;
  toggleAC();
  if (currentTemperature > 30.0) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("AIR CONDITIONER");
    lcd.setCursor(7, 1);
    lcd.print("ON");
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("AIR CONDITIONER");
    lcd.setCursor(7, 1);
    lcd.print("OFF");
  }
}

void doorController() {
  if(distance == prevDistance) return;
  if(distance <= 100) {
    doorState = false;
    toggleDoor();
  }
  else {
    doorState = true;
    toggleDoor();
  }
  if(doorState) {
    myServo.write(0);
  }
  else {
    myServo.write(90);
  }
}

void displayMain() {
  tft.fillScreen(ILI9341_WHITE);
  tft.fillRect(0, 0, 240, 30, ILI9341_BLUE);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(90, 10);
  tft.print("Menu");
  tft.setCursor(10, 50);
  tft.setTextColor(ILI9341_BLACK);
  tft.print("Door: ");
  toggleDoor();
  tft.setCursor(10, 100);
  tft.setTextColor(ILI9341_BLACK);
  tft.print("Light: ");
  toggleLight();
  tft.setCursor(10, 150);
  tft.setTextColor(ILI9341_BLACK);
  tft.print("Temperature: ");
  toggleAC();
}

void toggleAC() {
  tft.setCursor(155, 150);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.print(currentTemperature);
}

void toggleDoor() {
  if(doorState) {
    tft.fillRect(110, 45, 100, 25, ILI9341_RED);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(135, 50);
    tft.print("CLOSE");
  }
  else {
    tft.fillRect(110, 45, 100, 25, ILI9341_GREEN);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(135, 50);
    tft.print("OPEN");
  }
}

void toggleLight() {
  if(lightState) {
    tft.fillRect(110, 95, 100, 25, ILI9341_RED);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(145, 100);
    tft.print("OFF");
  }
  else {
    tft.fillRect(110, 95, 100, 25, ILI9341_GREEN);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(145, 100);
    tft.print("ON");
  }
}

void introScreen() {
  tft.fillRect(0, 0, 240, 50, ILI9341_BLUE);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 10);
  tft.print("Welcome to Home");
  tft.setCursor(10, 30);
  tft.print("Automation System");
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor(10, 160);
  for (int i = 10; i <= 100; i += 20) {
    tft.setCursor(10, 100);
    tft.print("Loading ");
    tft.print(i);
    tft.print("%   ");
    delay(300);
  }
}