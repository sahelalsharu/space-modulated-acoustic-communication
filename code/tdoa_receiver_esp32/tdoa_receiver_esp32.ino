#include <Arduino.h>

void IRAM_ATTR ISR_Rx1_Receive();
void IRAM_ATTR ISR_Rx2_Receive();
void IRAM_ATTR ISR_Rx3_Receive();
void IRAM_ATTR ISR_Rx4_Receive();

// Timer definitions
#define TIMER_FREQ_HZ        1000000  // 1 MHz
#define TIMER_DIVIDER        80       // Hardware timer clock divider
#define TIMER_RESOLUTION     ((double)TIMER_FREQ_HZ / 80000000.0) // Conversion to microseconds

volatile uint64_t TOA_Rx1 = 0;
volatile uint64_t TOA_Rx2 = 0;
volatile uint64_t TOA_Rx3 = 0;
volatile uint64_t TOA_Rx4 = 0;
volatile uint8_t  Rx1State = LOW;
volatile uint8_t  Rx2State = LOW;
volatile uint8_t  Rx3State = LOW;
volatile uint8_t  Rx4State = LOW;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const int Rx1Pin = 32;
const int Rx2Pin = 33;
const int Rx3Pin = 25;
const int Rx4Pin = 26;

// Timer handle
hw_timer_t *timer = NULL;

// Function to get current timer value in microseconds
uint64_t IRAM_ATTR getTimerValue() {
    return (uint64_t)timerRead(timer);
}

void setup() {
    Serial.begin(115200);
    
    timer = timerBegin(TIMER_FREQ_HZ);
    timerStart(timer);

    pinMode(Rx1Pin, INPUT_PULLDOWN);
    pinMode(Rx2Pin, INPUT_PULLDOWN);
    pinMode(Rx3Pin, INPUT_PULLDOWN);
    pinMode(Rx4Pin, INPUT_PULLDOWN);

    attachInterrupt(digitalPinToInterrupt(Rx1Pin), ISR_Rx1_Receive, RISING);
    attachInterrupt(digitalPinToInterrupt(Rx2Pin), ISR_Rx2_Receive, RISING);
    attachInterrupt(digitalPinToInterrupt(Rx3Pin), ISR_Rx3_Receive, RISING);
    attachInterrupt(digitalPinToInterrupt(Rx4Pin), ISR_Rx4_Receive, RISING);
}

void loop() {
    uint8_t s1, s2, s3, s4;
    uint64_t t1, t2, t3, t4;

    portENTER_CRITICAL(&mux);
    s1 = Rx1State; t1 = TOA_Rx1;
    s2 = Rx2State; t2 = TOA_Rx2;
    s3 = Rx3State; t3 = TOA_Rx3;
    s4 = Rx4State; t4 = TOA_Rx4;
    portEXIT_CRITICAL(&mux);

    if (s1 == HIGH || s2 == HIGH || s3 == HIGH || s4 == HIGH) {
        struct { uint8_t idx; uint64_t t; } arr[4] = {
            {1, t1}, {2, t2}, {3, t3}, {4, t4}
        };

        // Sort by timestamp
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3 - i; j++) {
                if (arr[j].t > arr[j+1].t) {
                    auto tmp = arr[j];
                    arr[j] = arr[j+1];
                    arr[j+1] = tmp;
                }
            }
        }

        // Print Transmitter Tx
        Serial.print("Tx");
        Serial.print(arr[0].idx);
        Serial.println(" was the sender.");

        // Print ordered Rx arrival
        for (int i = 0; i < 4; i++) {
            Serial.print("Rx");
            Serial.print(arr[i].idx);
            Serial.print(" was Received ");
            switch(i) {
                case 0: Serial.print("First");  break;
                case 1: Serial.print("Second"); break;
                case 2: Serial.print("Third");  break;
                case 3: Serial.print("Last");   break;
            }
            Serial.print(" at ");
            Serial.print((uint32_t)arr[i].t);
            Serial.println(" us");
        }

        // TDOA calculation
        Serial.println("Time Difference Of Arrival:");
        for (int i = 1; i < 4; i++) {
            // Convert timer ticks to microseconds
            float tdoa = (float)(arr[i].t - arr[0].t) * TIMER_RESOLUTION;
            Serial.print("Between Rx");
            Serial.print(arr[0].idx);
            Serial.print(" and Rx");
            Serial.print(arr[i].idx);
            Serial.print(" is ");
            Serial.print(tdoa, 2); 
            Serial.println(" us");
        }

        Serial.println("---------End Of This Cycle----------\n");

        // Reset states
        portENTER_CRITICAL(&mux);
        Rx1State = Rx2State = Rx3State = Rx4State = LOW;
        TOA_Rx1 = TOA_Rx2 = TOA_Rx3 = TOA_Rx4 = 0;
        portEXIT_CRITICAL(&mux);

        delay(1000);

        // Reattach interrupts
        attachInterrupt(digitalPinToInterrupt(Rx1Pin), ISR_Rx1_Receive, RISING);
        attachInterrupt(digitalPinToInterrupt(Rx2Pin), ISR_Rx2_Receive, RISING);
        attachInterrupt(digitalPinToInterrupt(Rx3Pin), ISR_Rx3_Receive, RISING);
        attachInterrupt(digitalPinToInterrupt(Rx4Pin), ISR_Rx4_Receive, RISING);
    } else {
        Serial.println("Waiting for signals...");
        delay(500);
    }
}

void IRAM_ATTR ISR_Rx1_Receive() {
    uint64_t timestamp = getTimerValue();
    detachInterrupt(digitalPinToInterrupt(Rx1Pin));
    portENTER_CRITICAL_ISR(&mux);
    Rx1State = HIGH;
    TOA_Rx1 = timestamp;
    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR ISR_Rx2_Receive() {
    uint64_t timestamp = getTimerValue();
    detachInterrupt(digitalPinToInterrupt(Rx2Pin));
    portENTER_CRITICAL_ISR(&mux);
    Rx2State = HIGH;
    TOA_Rx2 = timestamp;
    portEXIT_CRITICAL_ISR(&mux);
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