#include <SPI.h>
#include <Adafruit_SharpMem.h>

// Display connection pins
#define SHARP_SCK  1
#define SHARP_MOSI 2
#define SHARP_SS   3

// Display dimensions
#define SHARP_WIDTH  400
#define SHARP_HEIGHT 240

#define BLACK 0
#define WHITE 1

// Create display object
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, SHARP_WIDTH, SHARP_HEIGHT);

void setup() {
  // Begin SPI

  SPI.begin();
  // Start the display
  Serial.begin(115200);

  if (!display.begin()) {
    Serial.println("Display initialization failed!");
    while (1);
}

Serial.println("Display initialized successfully.");
delay(50);
  // Set the rotation of the display
  display.setRotation(0);
  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  // Write 'Hello, World!' to the buffer
  Serial.println("hello");
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(10, 10);
  display.print(F("Hello, World!"));
  // Send the buffer to the display
  display.refresh();
  // Wait for a bit before clearing the display
  delay(2000);
  display.clearDisplay();
}