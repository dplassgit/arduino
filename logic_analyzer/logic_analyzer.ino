/*
 * Simple logic analyzer. Triggers when the data pin goes low.
 * Records bits until 30  1's in a row are received.
 * Attemps to debounce the button (poorly). Resets the board
 * (reset pin low) when the button is pressed, and starts listening.
*/

const int buttonPin = 7;     // the number of the pushbutton pin
const int dataPin = 2;      // corresponds to pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 5;     // corresponds to pin 2 on the KB connector (and pin 4 on the RJ12)

int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);

  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin, LOW);
  delay(10);
  digitalWrite(resetPin, HIGH);

  Serial.begin(9600);
  Serial.println("Hello logic_analyzer");
}

int changed = 0;
int shouldRead = 0;
int data = 0;
int inKey = 0;  // are we reading a key?
int numOnes = 0;  // the # of 1s in a row

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH && !changed) {
    // turn LED on, and toggle recording.
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("on");
    shouldRead = 1 - shouldRead;
    Serial.print("ShouldRead is "); Serial.println(shouldRead);
    changed = 1;
  } else if (buttonState == LOW && changed) {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("off");
    changed = 0;
 
    // beep/reset, and keep reading.
    digitalWrite(resetPin , LOW);
    delay(10);
    digitalWrite(resetPin , HIGH);
  }

  if (shouldRead == 1) {
    data = digitalRead(dataPin);
    if (data == LOW && !inKey) {
      inKey=1;  // we're recording until numones==30
      numOnes=0;
      Serial.println("START");
      Serial.println("0");
    } else if (inKey) {
      if (data == HIGH) {
        Serial.println("1");
        numOnes++;
        if (numOnes == 30) {
          inKey=0;
          numOnes=0;
          Serial.println("END");
          delay(500);
        }
      } else {
        numOnes=0;
        Serial.println("0");
      }
    }
  }
}
