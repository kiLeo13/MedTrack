#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>

// Instantiate objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 rtc;

// Pin definitions
const int buzzerPin = 10;
const int motorIN1  = 9;
const int motorIN2  = 8;
const int motorPWM  = 11;
const int encoderA  = 2;
const int encoderB  = 3;
const int greenLedPin = 4;
const int redLedPin = 5;
const int buttonUpPin = 6;
const int buttonDownPin = 7;
const int buttonSelectPin = A0;
const int sensorPin = A1; // Now a confirmation button

volatile int encoderCount = 0;
int targetEncoderCount = 100;

// Dispensing time
int dispensingHour = 8;
int dispensingMinute = 0;
bool hasDispensed = false;

void setup() {
    Serial.begin(9600);

    // Configure pins
    pinMode(buzzerPin, OUTPUT);
    pinMode(motorIN1, OUTPUT);
    pinMode(motorIN2, OUTPUT);
    pinMode(motorPWM, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    pinMode(buttonUpPin, INPUT_PULLUP);
    pinMode(buttonDownPin, INPUT_PULLUP);
    pinMode(buttonSelectPin, INPUT_PULLUP);
    pinMode(sensorPin, INPUT_PULLUP); // Changed to INPUT_PULLUP (because it's now a button)

    // Configure encoder
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderA), countEncoder, RISING);

    // Initialize LCD
    lcd.init();
    lcd.backlight();

    Serial.println("RTC Test...");

    // Test RTC communication
    Wire.begin();
}

void loop() {
    bool h12Flag, pmFlag;
    
    // Get the current hour and minute
    int currentHour = rtc.getHour(h12Flag, pmFlag);
    int currentMinute = rtc.getMinute();

    // Update display with current time and dispensing time
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(formatTime(currentHour, currentMinute));

    lcd.setCursor(0, 1);
    lcd.print("Disp: ");
    lcd.print(formatTime(dispensingHour, dispensingMinute));

    // Adjust dispensing time with buttons
    if (digitalRead(buttonUpPin) == LOW) {
        dispensingHour = (dispensingHour + 1) % 24;
        delay(300);
    }
    if (digitalRead(buttonDownPin) == LOW) {
        dispensingMinute = (dispensingMinute + 1) % 60;
        delay(300);
    }

    // Check if it's time to dispense
    if (currentHour == dispensingHour && currentMinute == dispensingMinute && !hasDispensed) {
        dispenseMedicine();
        hasDispensed = true;
    }

    // Reset flag when the minute changes
    if (currentMinute != dispensingMinute) {
        hasDispensed = false;
    }

    delay(1000);
}

// Function to dispense medicine using a DC motor and encoder
void dispenseMedicine() {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);

    // Reset encoder count
    encoderCount = 0;

    // Turn on motor to dispense
    digitalWrite(motorIN1, HIGH);
    digitalWrite(motorIN2, LOW);
    analogWrite(motorPWM, 150);

    // Wait until the desired rotation count is reached
    while (encoderCount < targetEncoderCount);

    // Stop the motor
    digitalWrite(motorIN1, LOW);
    digitalWrite(motorIN2, LOW);
    analogWrite(motorPWM, 0);

    // Wait for user confirmation (button press)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press the button");
    lcd.setCursor(0, 1);
    lcd.print("to confirm!");

    unsigned long startTime = millis();
    bool confirmed = false;
    while (millis() - startTime < 15000) { // Wait up to 15 seconds
        if (digitalRead(sensorPin) == LOW) { // Button pressed
            confirmed = true;
            break;
        }
    }

    if (confirmed) {
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
    } else {
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, HIGH);
    }

    delay(3000);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, LOW);
    lcd.clear();
}

// Interrupt function to count encoder pulses
void countEncoder() {
    if (digitalRead(encoderB) == HIGH) {
        encoderCount++;
    } else {
        encoderCount--;
    }
}

// Formats time for LCD display
String formatTime(int hour, int minute) {
    String timeStr = "";
    if (hour < 10) timeStr += "0";
    timeStr += String(hour) + ":";
    if (minute < 10) timeStr += "0";
    timeStr += String(minute);
    return timeStr;
}