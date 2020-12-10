const int shamash = 2;  // output pin
const int candles[] = {3, 4, 5, 6, 7, 8, 9, 10};
const int button = 11; // input pin

int night = 0;

void setup() {
  // set up all outputs
  pinMode(shamash, OUTPUT);
  for (int i = 0; i < 8; ++i) {
    pinMode(candles[i], OUTPUT);
  }
  // set up input
  pinMode(button, INPUT);
}

unsigned long started = 0L;
#define ONE_HOUR_MILLIS 3600000L

void loop() {
  // if switch down (with debounce)
  if (digitalRead(button) == HIGH) {
    night = (night + 1) % 9; // yes 9, so that it maxes at 8

    if (night == 0) {
      digitalWrite(shamash, LOW);
    } else {
      digitalWrite(shamash, HIGH);
    }

    // Are there better ways to do this? Yes. Do I care? No.
    // Turn on the the appropriate # of candles.
    for (int i = 0; i < night; ++i) {
      digitalWrite(candles[i], HIGH);
    }
    // Turn off the rest.
    for (int i = night; i < 8; ++i) {
      digitalWrite(candles[i], LOW);
    }

    // Started now.
    started = millis();

    // for cheapo debouncing
    delay(200);
  } else if (millis() - started > ONE_HOUR_MILLIS) {
    // Candles go out after an hour
    night = 0;
    digitalWrite(shamash, LOW);
    for (int i = 0; i < 8; ++i) {
      digitalWrite(candles[i], LOW);
    }
  } else if (night > 0) {
    // Fake-flicker a candle
    int candle = random(night);
    int iters = random(5);
    for (int i = 0; i < iters; ++i) {
      digitalWrite(candles[candle], LOW);
      int offness = random(100);
      delay(offness);
      digitalWrite(candles[candle], HIGH);
    }
  }
}
