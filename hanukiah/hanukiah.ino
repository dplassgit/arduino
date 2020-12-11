/**
   Hanukiah sketch. Press the button to turn on another light.

   The lights flicker every once in a while.

   Does not require PWM!
*/

const int candlePins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int button = 11; // input pin

// Number of candles on
int numOn = 0;

void setup() {
  Serial.begin(9600);

  Serial.println("Hello hanukiah");

  // set up all outputs
  for (int i = 0; i < 9; ++i) {
    pinMode(candlePins[i], OUTPUT);
  }
  // set up input
  pinMode(button, INPUT);
  randomSeed(analogRead(12));
}

unsigned long started = 0L;
#define ONE_HOUR_MILLIS 3600000L

void loop() {
  // if switch down (with debounce)
  if (digitalRead(button) == LOW) {
    Serial.println("button is pushed");
    numOn = (numOn + 1) % 10; // max is 9
    Serial.print("numOn is now "); Serial.println(numOn);

    // Are there better ways to do this? Yes. Do I care? No.
    // Turn off all.
    for (int i = 0; i < 9; ++i) {
      digitalWrite(candlePins[i], LOW);
    }
    // Turn on the the appropriate # of candles.
    for (int i = 0; i < numOn; ++i) {
      digitalWrite(candlePins[i], HIGH);
    }

    // Record last click time, to turn off after an hour.
    started = millis();

    // For cheap "debouncing"
    delay(200);
  } else if (millis() - started > ONE_HOUR_MILLIS) {
    // Candles go out after an hour
    numOn = 0;
    for (int i = 0; i < 9; ++i) {
      digitalWrite(candlePins[i], LOW);
    }
  } else if (numOn > 0) {
    // Fake-flicker some candles
    int iters = random(5);
    for (int i = 0; i < iters; ++i) {
      long candle = random(4000);
      // Slows down the flickering
      if (candle < numOn) {
        // Turn one off for up to 50 ms
        digitalWrite(candlePins[candle], LOW);
        int offness = random(50);
        delay(offness);
        // Turn it back on for up to 200 ms
        digitalWrite(candlePins[candle], HIGH);
        int onness = random(200);
        delay(onness);
      }
    }
  }
}
