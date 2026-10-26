#include <Wire.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);
// Pin connections
#define SERVO_PIN 17  // TX2 / GPIO17
#define GREEN_LED 4   // D4
#define RED_LED 18    // D18
// Servo settings
Servo doorServo;
const int LOCK_ANGLE = 0;
const int UNLOCK_ANGLE = 90;
// Password settings
const String PASSWORD = "1234";
String enteredPassword = "";
bool doorUnlocked = false;
unsigned long unlockStartTime = 0;
const unsigned long UNLOCK_DURATION = 5000;



// 4x4 keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {15, 14, 12, 13};
Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);
// OLED message function
void showMessage(String line1, String line2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println(line1);
  display.setCursor(0, 35);
  display.println(line2);
display.display();
}


// Lock door
void lockDoor() {
  doorServo.write(LOCK_ANGLE);
  doorUnlocked = false;
  enteredPassword = "";
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  showMessage("SMART DOOR LOCK", "Door Locked");
}
// Unlock door
void unlockDoor() {
  doorServo.write(UNLOCK_ANGLE);
  doorUnlocked = true;
  unlockStartTime = millis();
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  enteredPassword = "";
  showMessage("Access Granted", "Door Unlocked");
};
// Check entered password
void checkPassword() {
  if (enteredPassword == PASSWORD) {
    unlockDoor();
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    enteredPassword = "";
    showMessage("Access Denied", "Wrong Password");
    delay(1500);


    digitalWrite(RED_LED, LOW);
    showMessage("Enter Password", "* Clear  # Enter");
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  // OLED uses SDA = GPIO21 and SCL = GPIO22
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not detected!");
    while (true) {
      delay(100);
    }
  }
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(LOCK_ANGLE);
showMessage("SMART DIGITAL", "DOOR LOCK");
  delay(1500);
  showMessage("Enter Password", "* Clear  # Enter");
}
void loop() {
  if (doorUnlocked &&millis() - unlockStartTime >= UNLOCK_DURATION) {
    lockDoor();
    return;
  }

  char key = keypad.getKey();
  if (key == NO_KEY) {
    return;
  }
  Serial.print("Key: ");
  Serial.println(key);
  // Clear entered password
  if (key == '*') {
    enteredPassword = "";
    digitalWrite(RED_LED, LOW);
showMessage("Enter Password", "Password Cleared");
    delay(500);
    showMessage("Enter Password", "* Clear  # Enter");
    return;
  }
  if (key == '#') {
    checkPassword();
    return;
  }
  if (key >= '0' && key <= '9') {
    if (enteredPassword.length() < 8) {
      enteredPassword += key;
      String hiddenPassword = "";
      for (int i = 0; i < enteredPassword.length(); i++) {
        hiddenPassword += '*';
      }
      showMessage("Enter Password", hiddenPassword);
    }
  }
}
