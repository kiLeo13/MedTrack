#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <Stepper.h>

// LCD and RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 rtc;

// Stepper Motor 28BYJ-48 (ULN2003)
const int STEPS_PER_REVOLUTION = 2048;
Stepper stepper(STEPS_PER_REVOLUTION, 8, 10, 9, 11); // IN1, IN3, IN2, IN4

// Pins
const int BUZZER_PIN = 7;
const int GREEN_LED_PIN = 4;
const int RED_LED_PIN = 5;
const int SELECT_BUTTON_PIN = 2;
const int UP_HOUR_BUTTON_PIN = 3;
const int DOWN_HOUR_BUTTON_PIN = 6;
const int MINUTE_BUTTON_PIN = A0;
const int SENSOR_PIN = A1;

int dispensingHour = 8;
int dispensingMinute = 0;
bool hasDispensed = false;
bool adjustingTime = false;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_HOUR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_HOUR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MINUTE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  Wire.begin();

  stepper.setSpeed(10); // RPM
}

void loop() {
  bool h12Flag, pmFlag;

  int currentHour = rtc.getHour(h12Flag, pmFlag);
  int currentMinute = rtc.getMinute();

  // Display current time
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(formatTime(currentHour, currentMinute));

  // Display dispensing time
  lcd.setCursor(0, 1);
  if (adjustingTime) {
    lcd.print("Set:  ");
  } else {
    lcd.print("Disp: ");
  }
  lcd.print(formatTime(dispensingHour, dispensingMinute));

  // Toggle time setting mode
  if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
    adjustingTime = !adjustingTime;
    delay(300);
  }

  if (adjustingTime) {
    if (digitalRead(UP_HOUR_BUTTON_PIN) == LOW) {
      dispensingHour = (dispensingHour + 1) % 24;
      delay(300);
    }
    if (digitalRead(DOWN_HOUR_BUTTON_PIN) == LOW) {
      dispensingHour = (dispensingHour - 1 + 24) % 24;
      delay(300);
    }
    if (digitalRead(MINUTE_BUTTON_PIN) == LOW) {
      dispensingMinute = (dispensingMinute + 1) % 60;
      delay(300);
    }
  } else {
    if (currentHour == dispensingHour && currentMinute == dispensingMinute && !hasDispensed) {
      dispenseMedicine();
      hasDispensed = true;
    }
    if (currentMinute != dispensingMinute) {
      hasDispensed = false;
    }
  }

  delay(200);
}

void dispenseMedicine() {
  // Sound the buzzer until a button is pressed
  digitalWrite(BUZZER_PIN, HIGH);
  while (
    digitalRead(SELECT_BUTTON_PIN) == HIGH &&
    digitalRead(UP_HOUR_BUTTON_PIN) == HIGH &&
    digitalRead(DOWN_HOUR_BUTTON_PIN) == HIGH &&
    digitalRead(MINUTE_BUTTON_PIN) == HIGH
  ) {
    delay(100);
  }
  digitalWrite(BUZZER_PIN, LOW);

  // Rotate motor 60 degrees (≈342 steps)
  stepper.step(342);

  // Check if medicine was collected
  if (digitalRead(SENSOR_PIN) == HIGH) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }

  delay(3000);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
}

String formatTime(int hour, int minute) {
  String timeStr = "";
  if (hour < 10) timeStr += "0";
  timeStr += String(hour) + ":";
  if (minute < 10) timeStr += "0";
  timeStr += String(minute);
  return timeStr;
}
