#include <EEPROM.h>
#include <FastLED.h>

#define LED_PIN  4
#define COLOR_ORDER GRB
#define CHIPSET     WS2811

const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 8;
const bool    kMatrixSerpentineLayout = true;
boolean paused = false, blinky = true;
int BRIGHTNESS = 20;
long previous, interval = 250;

#define LEFT A5
#define UP A4
#define ADDITIONAL 3

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if ( x >= kMatrixWidth) return -1;
  if ( y >= kMatrixHeight) return -1;
  return XY(x, y);
}

//SNAKESNAKESNAKESNAKESNAKESNAKE
unsigned int len = 1, x = 2, y = 2, pos = 0, dir = -1;
int food = 32;
int snake[NUM_LEDS];
int high = 0;

void setup() {
  pinMode(LEFT, INPUT);
  pinMode(UP, INPUT);
  pinMode(ADDITIONAL, INPUT_PULLUP);
  pinMode(A7, OUTPUT);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );

  FastLED.showColor(CRGB(255, 0, 0));
  delay(200);
  analogWrite(A7, 1024);
  FastLED.showColor(CRGB(0, 255, 0));
  delay(200);
  FastLED.showColor(CRGB(0, 0, 255));
  delay(200);
  FastLED.showColor(CRGB(0, 0, 0));

  for (int i = 1; i < NUM_LEDS - 1; i++)
  {
    snake[i] = -1;
  }
  pos = XY(x, y);
  high = (int)EEPROM.read(1);

  for (int i = 0; i < high; i++)
  {
    leds[i] = CHSV(i * 15, 255, 255);
  }
  FastLED.show();
  delay(2000);
}

void loop() {
  const int thresh = 50;
  int up = analogRead(UP) - 512;
  int left = 512 - analogRead(LEFT);

  if (abs(up) > abs(left))
  {
    if (up < -thresh) {
      dir = 2;
    }
    if (up > thresh) {
      dir = 0;
    }
  }
  else {
    if (left < -thresh) {
      dir = 1;
    }
    if (left > thresh) {
      dir = 3;
    }
  }

  if (!digitalRead(ADDITIONAL)) {
    while (!digitalRead(ADDITIONAL)) {}
    paused = !paused;
    delay(250);
    if (!paused)
    {
      fill_solid(leds, NUM_LEDS, CRGB(0,0,0));
      for (int i = 1; i < NUM_LEDS - 1; i++)
      {
        if (snake[i] >= 0) {
          leds[snake[i]] = CRGB(0, 255, 0);
        }
      }

      leds[XY(x, y)] = CRGB(255, 255, 255);
      leds[food] = CRGB::Red;
      FastLED.show();
      delay(250);
      FastLED.showColor(CRGB(0, 0, 0));
      delay(250);
      FastLED.show();
      delay(125);
      FastLED.showColor(CRGB(0, 0, 0));
      delay(125);
      FastLED.show();
      delay(75);
      FastLED.showColor(CRGB(0, 0, 0));
      delay(75);
      previous = millis();
    }
  }

  if (!paused) {
    if (millis() - previous > interval) {
      previous = millis();
      switch (dir) {
        default: break;
        case 0: y--; break;
        case 1: x--; break;
        case 2: y++; break;
        case 3: x++; break;
      }
      if (XYsafe(x, y) == -1)
      {
        death();
      }
      else {
        pos = XY(x, y);
        if (leds[pos] == CRGB(0, 255, 0)) {
          leds[pos] = CRGB(0, 0, 255);
          death();
        }
      }

      if (pos == food) {
        len++;
        int r;
waitLoop:
        r = random(0, NUM_LEDS - 1);
        if (!(leds[r] == CRGB(0, 0, 0)))
        {
          goto waitLoop;
        }
        food = r;
      }


      for (int k = NUM_LEDS - 2; k > 0; k--) //shift the array values from point i
      {
        snake[k] = snake[k - 1];
      }

      snake[0] = pos;
      snake[len] = -1;
    }
  }

  if (paused)
  {
    fill_solid(leds, NUM_LEDS, CHSV((millis() / 26), 255, 255));

    if (millis() - previous > interval) {
      previous = millis();
      blinky = !blinky;
    }

    if (blinky) {
      leds[0] = CRGB(255, 255, 255);
    }
    else {
      //leds[0] = CRGB(0, 0, 0);
    }

    if (up < -thresh) {
      BRIGHTNESS = min(255, BRIGHTNESS + 1);
    }
    if (up > thresh) {
      BRIGHTNESS = max(0, BRIGHTNESS - 1);
    }
    if (left < -thresh) {
      interval++;
    }
    if (left > thresh) {
      interval = max(interval - 1, 5);
    }
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
    delay(7);
  }
  else {
    fadeToBlackBy( leds, NUM_LEDS, 55);
    for (int i = 1; i < NUM_LEDS - 1; i++)
    {
      if (snake[i] >= 0) {
        leds[snake[i]] = CRGB(0, 255, 0);
      }
    }

    leds[XY(x, y)] = CRGB(255, 255, 255);
    leds[food] = CRGB::Red;
    FastLED.show();
    FastLED.delay(5);
  }
}

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;

  if ( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if ( kMatrixSerpentineLayout == true) {
    if ( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }

  return i;
}

void death()
{
  CRGB ledTmp[NUM_LEDS];
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      ledTmp[i] = leds[i];
    }
    FastLED.showColor(CRGB(0, 0, 0));
    delay(400);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = ledTmp[i];
    }
    FastLED.show();
    delay(400);
  }

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if ((i + j) % 2 == 0) {
        if (len > high) {
          leds[XY(i, j)] = CRGB::Green;
        }
        else {
          leds[XY(i, j)] = CRGB::Red;
        }
      }
      else {
        leds[XY(i, j)] = CRGB::Gray;
      }
    }
  }
  if (len > high) {
    high = len;
    EEPROM.write(1, (byte)high);
  }

  FastLED.delay(600);
  FastLED.showColor(CRGB(0, 0, 0));
  x = 2; y = 2; dir = -1; len = 1;
  pos = XY(x, y);
  for (int i = 1; i < NUM_LEDS - 1; i++)
  {
    snake[i] = -1;
  }
}
