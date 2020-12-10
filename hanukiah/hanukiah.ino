const int candles[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int button = 11; // input pin

int night = 0;

void setup() {
  Serial.begin(9600);

  Serial.println("Hello hanukiah");

  // set up all outputs
  for (int i = 0; i < 9; ++i) {
    pinMode(candles[i], OUTPUT);
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
    night = (night + 1) % 9;
    if (night == 1) {
      night = 2;
    }
    Serial.print("night is now "); Serial.println(night);

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
    for (int i = 0; i < 9; ++i) {
      digitalWrite(candles[i], LOW);
    }
  } else if (night > 0) {
    // Fake-flicker a candle
    int iters = random(4);
    for (int i = 0; i < iters; ++i) {
      long candle = random(4000);
      if (candle < night) {
        digitalWrite(candles[candle], LOW);
        int offness = random(50);
        delay(offness);
        digitalWrite(candles[candle], HIGH);
        int onness = random(300);
        delay(onness);
      }
    }
  }
}
