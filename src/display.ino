#include <LiquidCrystal.h> // Include library   path---Sketch -> Include Library -> LiquidCrystal

#define t 4000 // Delay time

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // arduino pin(rs=2, e=3, D4=4, D5=5, D6=6, D7=7)

void setup() {
  lcd.begin(16, 2); // set up the LCD's number of columns and rows
}

void loop() {
  intro();
  // ends();
  // thumbnail();
}

void intro() {
    lcd.setCursor(2, 0); // set print point
    lcd.print("Ol"); // print text
    delay(T);
    lcd.clear(); // clear display

    lcd.setCursor(0, 0);
    lcd.print("Testing");

    lcd.setCursor(0, 1);
    lcd.print("the display");

    delay(T);
    lcd.clear();
}

void ends() {
    lcd.setCursor(0, 0);
    lcd.print("Thanks for Watching");
    lcd.scrollDisplayLeft(); // scroll text left
    delay(250);

    lcd.setCursor(0, 1);
    lcd.print("Please like share and subscribe");
    lcd.scrollDisplayLeft(); // scroll text left
    delay(250);
}

void thumbnail() {
    lcd.setCursor(2, 0);
    lcd.print("LCD tutorial");
}