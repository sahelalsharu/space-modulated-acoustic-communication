#include <Arduino.h>

void IRAM_ATTR ISR_Rx3_Receive();
void IRAM_ATTR ISR_Rx4_Receive();


#define TIMER_FREQ_HZ        1000000
#define TIMER_DIVIDER        80
#define TIMER_RESOLUTION     ((double)TIMER_FREQ_HZ / 80000000.0)


volatile uint64_t TOA_Rx3 = 0;
volatile uint64_t TOA_Rx4 = 0;
volatile uint8_t  Rx3State = LOW;
volatile uint8_t  Rx4State = LOW;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


const int Rx3Pin = 25;
const int Rx4Pin = 26;


uint8_t bitCount = 0;
char receivedChar;
String receivedMessage = "";


hw_timer_t *timer = NULL;

uint64_t IRAM_ATTR getTimerValue() {
    return (uint64_t)timerRead(timer);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Receiver ready...");
    // Initialize Timer 
    timer = timerBegin(TIMER_FREQ_HZ);
    timerStart(timer);

    pinMode(Rx3Pin, INPUT);
    pinMode(Rx4Pin, INPUT);

    attachInterrupt(digitalPinToInterrupt(Rx3Pin), ISR_Rx3_Receive, RISING);
    attachInterrupt(digitalPinToInterrupt(Rx4Pin), ISR_Rx4_Receive, RISING);
    
    
}

void processBit(uint8_t bit) {
    bitWrite(receivedChar, bitCount, bit);
    Serial.print("Bit ");
    Serial.print(bitCount);
    Serial.print(" is: ");
    Serial.print(bit);
    Serial.println(" was Received");
    bitCount++;

    if (bitCount == 8) {
        Serial.print("The Received Character is: ");
        Serial.println(receivedChar);

        receivedMessage += receivedChar;  
        bitCount = 0;

        Serial.print("Current Message: ");
        Serial.println(receivedMessage);
    }
}


void loop() {
    uint8_t s3, s4;
    uint64_t t3, t4;
    
    portENTER_CRITICAL(&mux);
    s3 = Rx3State; t3 = TOA_Rx3;
    s4 = Rx4State; t4 = TOA_Rx4;
    portEXIT_CRITICAL(&mux);

    if (s3 == HIGH && s4 == HIGH) {
        int64_t TDOA = 0;
        TDOA = int64_t (t3 - t4);

        if(TDOA > 0){
          processBit(1);
        }
        else{
          processBit(0);
        }

        portENTER_CRITICAL(&mux);
        Rx3State = Rx4State = LOW;
        TOA_Rx3 = TOA_Rx4 = 0;
        portEXIT_CRITICAL(&mux);

        delay(400);

        attachInterrupt(digitalPinToInterrupt(Rx3Pin), ISR_Rx3_Receive, RISING);
        attachInterrupt(digitalPinToInterrupt(Rx4Pin), ISR_Rx4_Receive, RISING);
    }
}

void IRAM_ATTR ISR_Rx3_Receive() {
    uint64_t timestamp = getTimerValue();
    detachInterrupt(digitalPinToInterrupt(Rx3Pin));
    portENTER_CRITICAL_ISR(&mux);
    Rx3State = HIGH;
    TOA_Rx3 = timestamp;
    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR ISR_Rx4_Receive() {
    uint64_t timestamp = getTimerValue();
    detachInterrupt(digitalPinToInterrupt(Rx4Pin));
    portENTER_CRITICAL_ISR(&mux);
    Rx4State = HIGH;
    TOA_Rx4 = timestamp;
    portEXIT_CRITICAL_ISR(&mux);
}
