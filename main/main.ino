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
#define MAINTEXT_MAXLEN 32

// Create display object
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, SHARP_WIDTH, SHARP_HEIGHT);

// create rotaryencoder object
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B, ROTARY_ENCODER_PIN_BT, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

uint8_t state = 0;
uint8_t scroll_state = 0;
uint8_t last_button_state = 1;
int text_scroll_offset = 0;
int last_encoder_pos = 0;

static unsigned long lastTimePressed = 0;
struct offset_wrap { 
  String text;
  int offset;
};


//0 -- welcome screen
//1 -- navigation screen
//2 -- bitmap
//3 -- info text

int handle_click() {
    if(state == 0) {
      text_scroll_offset = 0;
      return 1;
    }
    else if(state == 1) return 2;
    else if(state == 2) return 3;
    else if(state == 3) {
      text_scroll_offset = 0;
      return 1;
    }
    else if(state == 4) return 0;
}

int handle_hold() {
    if(state == 0) {
      text_scroll_offset = 0;
      return 4;
    }
    else if(state == 1) return 0;
    else if(state == 2) return 0;
    else if(state == 3) {
      text_scroll_offset = 0;
      return 0;
    }
    else if(state == 4) return 0;
}

String wrap(String s, int limit){
  int space = 0;
  int i = 0;
  int line = 0;
  while(i<s.length()){
    if(s.substring(i,i+2)=="~"){
      line=0;
      Serial.println("found newline");
    }
    if(s.substring(i,i+1)==" "){
      space=i; 
    }
    if(line > limit-1){ 
      s=s.substring(0,space)+"~"+s.substring(space+1);
      line = 0;
      i = space+1;
    }
    i++;
    line++;
  }
  s.replace("~","\n ");
    return s;
}

struct offset_wrap offsetWrap(String s, int limit, int offset){
  int space = 0;
  int i = 0;
  int line = offset;
  while(i<s.length()){
    if(s.substring(i,i+2)=="~"){
      line=0;
      Serial.println("found newline");
    }
    if(s.substring(i,i+1)==" "){
      space=i; 
    }
    if(line > limit-1){ 
      s=s.substring(0,space)+"~"+s.substring(space+1);
      line = 0;
      i = space+1;
    }
    i++;
    line++;
  }
  s.replace("~","\n ");
    return {s, line};
}


// clip off first N lines of text
String text_buffer(String s, int offset_counter){
  int space = 0;
  int i = 0;
  int line = 0;
  String clipped = s;

  while(i<s.length() && offset_counter > 0){
    if(s[i] == '\n'){
      clipped = s.substring(i+1,s.length());
      offset_counter--;
    }
    i++;
  }
  return clipped;
}

void display_state(int scroll_state) {
    display.clearDisplay();

    if(state == 0) {
        //welcome screen
        display.setCursor(5, 5); // Start at top-left corner
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(3);
        display.println("Testa");
        display.setTextSize(2);
        String clipped_text = text_buffer(wrap(welcome_text, MAINTEXT_MAXLEN), text_scroll_offset);
        display.print(clipped_text);
    }

     else if(state == 1) {
        display.setCursor(5, 5); // Start at top-left corner
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(3);
        display.println("Main Menu");

        display.setTextSize(2);
        display.println(wrap(main_text, MAINTEXT_MAXLEN));
        uint32_t menu_length = sizeof(menu) / sizeof(menu[0]);

        for (int i=0; i<menu_length; i++){
          if(i == scroll_state) display.setTextColor(WHITE, BLACK);
          else display.setTextColor(BLACK, WHITE);
          String menu_text = wrap(menu[i][0], MAINTEXT_MAXLEN);
          display.println(menu_text);
        }
      }

    else if(state == 2) {
        display.setCursor(0,0); // Start at top-left corner
        display.drawBitmap(0, 0, bitmaps[scroll_state], SHARP_WIDTH, SHARP_HEIGHT, BLACK);
    }

    else if (state == 3)
    {
        display.setCursor(5, 5);
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(3);
        display.println(menu[scroll_state][1]);
        Serial.println(text_scroll_offset);
        display.setTextSize(2);
        String clipped_text = text_buffer(wrap(menu[scroll_state][2], MAINTEXT_MAXLEN), text_scroll_offset);
        display.println(clipped_text);
    }

    else if (state == 4)
    {
        display.setCursor(5,5); // Start at top-left corner
        display.println("easter egg");
    }

    display.refresh();
}

void rotary_loop()
{
    int button_state = digitalRead(ROTARY_ENCODER_PIN_BT); // 1 when released, 0 when pressed

    if (button_state != last_button_state){
      int current_millis = millis();

      // get falling edge
     if (button_state == 1){
        if ((current_millis - lastTimePressed) > 2000){
          state = handle_hold();
          display_state(scroll_state);
        }

        else if ((current_millis - lastTimePressed) > 30){
          state = handle_click();
          display_state(scroll_state);
        }
      }
     
     else if (button_state == 0 && (current_millis - lastTimePressed) > 30){
      lastTimePressed = millis();
     }
    }
    last_button_state = button_state;


    // handle menu scroll state
    if(rotaryEncoder.encoderChanged()){
      if (state == 1)
        {
          int value = 999 - rotaryEncoder.readEncoder();
          scroll_state = value%NUM_ENTRIES;
          display_state(scroll_state);
        }

      // handle text scroll
      if (state == 0 || state == 3)
        {
          int value = rotaryEncoder.readEncoder();
          if(value < last_encoder_pos){
            text_scroll_offset+=1;
            display_state(scroll_state);
          }
          else if (value > last_encoder_pos){
            text_scroll_offset-=1;
            //return here to prevent rerender
            if(text_scroll_offset < 0) text_scroll_offset = 0;
            display_state(scroll_state);
          }

          last_encoder_pos = value;
        }
    };


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
    rotaryEncoder.setBoundaries(0, 999, true); //minscroll_state, maxscroll_state, circlescroll_states true|false (when max go to min and vice versa)
    rotaryEncoder.disableAcceleration();
    display_state(0);

}

void loop() {
  rotary_loop();
}