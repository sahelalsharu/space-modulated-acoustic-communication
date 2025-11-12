void callsignal();
void stopsignal();

const int trig0 = 6;
const int trig1 = 8;
char message[100];      
int bitDelay = 15;

void setup() {
  pinMode(9, OUTPUT);
  pinMode(trig0, OUTPUT);
  pinMode(trig1, OUTPUT);
  
  digitalWrite(trig0, HIGH);
  digitalWrite(trig1, HIGH);
  digitalWrite(9, LOW);
  
  Serial.begin(115200);
  Serial.println("Enter a message to send: "); 
}

void loop() {
  if (Serial.available() > 0) {
    int messageLength = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
    message[messageLength] = '\0'; 

    for (int i = 0; i < messageLength; i++) {
      sendChar(message[i]);  // send one char = 8 bits
      delay(500);           
    }

    Serial.println("Message Sent. Enter New Message to Send:");
  }
}

void sendChar(char currentChar) {
  Serial.print("Sending character: ");
  Serial.print(currentChar);
  Serial.print(" (Binary: ");
  Serial.print(currentChar, BIN);
  Serial.println(")");

  for (int j = 0; j < 8; j++) {
    if (currentChar & (1 << j)) {
      send1();
    } else {
      send0();
    }
  }
}

void send1() {
  digitalWrite(trig0, LOW);    // Tx4 sender
  digitalWrite(trig1, HIGH);
  callsignal();
  delay(300);
  stopsignal();
  digitalWrite(trig0, HIGH);
  delay(500);
}

void send0() {
  digitalWrite(trig1, LOW);    // Tx3 sender
  digitalWrite(trig0, HIGH);
  callsignal();
  delay(300);
  stopsignal();
  digitalWrite(trig1, HIGH);
  delay(500);
}

void callsignal() {
  TCCR1A = _BV(COM1A0);
  TCCR1B = _BV(WGM12) | _BV(CS10);
  OCR1A = 199;  
}

void stopsignal() {
  TCCR1A = 0;
  TCCR1B = 0;
  digitalWrite(9, LOW);
}

