void callsignal();
void stopsignal();

void setup() {
  pinMode(9, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
}

void loop() {

  digitalWrite(2, HIGH);  // Tx1
  digitalWrite(4, HIGH);  // Tx2
  digitalWrite(6, HIGH);  // Tx3
  digitalWrite(8, LOW);   // Tx4

  if (digitalRead(2) == LOW) {
    digitalWrite(4, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(8, HIGH);
    callsignal();
    delay(400);
    stopsignal();
    digitalWrite(2, HIGH);
    delay(500); 
  }

  if (digitalRead(4) == LOW) {
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(8, HIGH);
    callsignal();
    delay(500);
    stopsignal();
    digitalWrite(4, HIGH);
    delay(500); 
  }

  if (digitalRead(6) == LOW) {
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(8, HIGH);
    callsignal();
    delay(500);
    stopsignal();
    digitalWrite(6, HIGH);
    delay(500);
  }

  if (digitalRead(8) == LOW) {
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(4, HIGH);
    callsignal();
    delay(500);
    stopsignal();
    digitalWrite(8, HIGH);
    delay(500); 
  } 
  
  //while (true);  // Infinite loop to halt the program
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