#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include "bitmaps.h"
#include "AiEsp32RotaryEncoder.h"

// Display connection pins
#define SHARP_SCK  1
#define SHARP_MOSI 2
#define SHARP_SS   3

// rotary encoder
#define ROTARY_ENCODER_PIN_A D8
#define ROTARY_ENCODER_PIN_B D9
#define ROTARY_ENCODER_PIN_BT D10
#define ROTARY_ENCODER_VCC_PIN -1 //directly connected to vcc
#define ROTARY_ENCODER_STEPS 4 //1 or 2

// Display dimensions
#define SHARP_WIDTH  400
#define SHARP_HEIGHT 240

//program variables
#define NUM_ENTRIES  4
#define BLACK 0
#define WHITE 1

// Create display object
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, SHARP_WIDTH, SHARP_HEIGHT);

// create rotaryencoder object
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B, ROTARY_ENCODER_PIN_BT, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

bool showing_info = false;

void rotary_onButtonClick()
{
      int value = NUM_ENTRIES - 1 - rotaryEncoder.readEncoder()%NUM_ENTRIES;

      if(showing_info == false) {
        display.clearDisplay();
        display.setCursor(0,0); // Start at top-left corner
        display.println(texts[value][3]);
        display.refresh();
        showing_info = true;
      }

    else
    {
        display.clearDisplay();
        int value = NUM_ENTRIES - 1 - rotaryEncoder.readEncoder()%NUM_ENTRIES;
        Serial.println(value);
        display.clearDisplay();
        display.setCursor(0, 0); // Start at top-left corner
        display.setTextSize(2);

        display.setTextColor(BLACK, WHITE);
        display.print(texts[value][0]);
        display.setTextColor(WHITE, BLACK);
        display.print(texts[value][1]);
        display.setTextColor(BLACK, WHITE);
        display.print(texts[value][2]);
        display.refresh();
        showing_info = false;
    }
}

void rotary_loop()
{

    static unsigned long lastTimePressed = 0;
  //ignore multiple press in that time milliseconds
    if (millis() - lastTimePressed < 50)
      {
        return;
      }
    lastTimePressed = millis();
    
    if (rotaryEncoder.encoderChanged())
    {
        //reset the info
        showing_info = false;

        int value = NUM_ENTRIES - 1 - rotaryEncoder.readEncoder()%NUM_ENTRIES;
        Serial.println(value);
        display.clearDisplay();
        display.setCursor(0, 0); // Start at top-left corner
        display.setTextSize(2);

        display.setTextColor(BLACK, WHITE);
        display.print(texts[value][0]);
        display.setTextColor(WHITE, BLACK);
        display.print(texts[value][1]);
        display.setTextColor(BLACK, WHITE);
        display.print(texts[value][2]);

        // Send the buffer to the display
        display.refresh();

        // display.drawBitmap(100, 0, leaf_screen, SHARP_WIDTH, SHARP_HEIGHT, BLACK);
        // Wait for a bit before clearing the display
        // delay(1000);
        // display.clearDisplay();
        // display.drawBitmap(0, 0, leaf_screen, SHARP_WIDTH, SHARP_HEIGHT, BLACK);
        // display.refresh();
        // delay(1000);

    }

  if (rotaryEncoder.isEncoderButtonClicked())
    {
        rotary_onButtonClick();
    }
}

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}


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


  //rotary encoder setup
  pinMode(ROTARY_ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_PIN_BT, INPUT_PULLUP);

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 1000, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  //rotaryEncoder.setAcceleration(250);
  rotaryEncoder.disableAcceleration();

}

void loop() {
  rotary_loop();
  delay(50);
}