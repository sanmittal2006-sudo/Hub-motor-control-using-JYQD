#include <Arduino.h>
#include <STM32FreeRTOS.h>

// --- Pin Definitions ---
#define MOTOR1_VR  PA0  // Speed Pin (PWM)
#define MOTOR1_ZF  PA1  // Direction Pin (HIGH = Forward, LOW = Backward)
#define MOTOR1_EL  PA2  // Enable Pin (HIGH = Run, LOW = Brake/Disable)
#define MOTOR1_SC  PA3  // Hall Speed Pulse Signal (Input from JYQD)

volatile uint32_t pulseCount = 0;

// Interrupt Service Routine for Speed Pulses
void handlePulse() {
    pulseCount++;
}

// Function Declarations
void vMotorTestSequenceTask(void *pvParameters);
void vSpeedTelemetryTask(void *pvParameters);
void setMotorState(bool enable, bool forward, uint8_t pwmSpeed);

void setup() {
    Serial.begin(115200);

    pinMode(MOTOR1_VR, OUTPUT);
    pinMode(MOTOR1_ZF, OUTPUT);
    pinMode(MOTOR1_EL, OUTPUT);
    pinMode(MOTOR1_SC, INPUT);

    setMotorState(false, true, 0);

    // Attach Hardware Interrupt to PA3 on RISING edge
    attachInterrupt(digitalPinToInterrupt(MOTOR1_SC), handlePulse, RISING);

    // Create FreeRTOS Tasks
    xTaskCreate(vMotorTestSequenceTask, "MotorSeqTask", 256, NULL, 2, NULL);
    xTaskCreate(vSpeedTelemetryTask, "TelemetryTask", 256, NULL, 1, NULL);

    vTaskStartScheduler();
}

void loop() {
    // Empty: FreeRTOS manages execution
}

void setMotorState(bool enable, bool forward, uint8_t pwmSpeed) {
    digitalWrite(MOTOR1_EL, enable ? HIGH : LOW);
    digitalWrite(MOTOR1_ZF, forward ? HIGH : LOW);
    analogWrite(MOTOR1_VR, pwmSpeed);
}

// --- TASK 1: Motor Movement Sequence ---
void vMotorTestSequenceTask(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        // STEP 1: 40 PWM FORWARD
        Serial.println("\n=================================");
        Serial.println(">>> STEP 1: FORWARD @ 40 PWM");
        Serial.println("=================================");
        setMotorState(true, true, 40);
        vTaskDelay(pdMS_TO_TICKS(3000));

        setMotorState(true, true, 0);
        vTaskDelay(pdMS_TO_TICKS(1000)); 

        // STEP 2: 40 PWM BACKWARD
        Serial.println("\n=================================");
        Serial.println(">>> STEP 2: BACKWARD @ 40 PWM");
        Serial.println("=================================");
        setMotorState(true, false, 40);
        vTaskDelay(pdMS_TO_TICKS(3000));

        setMotorState(true, false, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // STEP 3: 100 PWM FORWARD
        Serial.println("\n=================================");
        Serial.println(">>> STEP 3: FORWARD @ 100 PWM");
        Serial.println("=================================");
        setMotorState(true, true, 100);
        vTaskDelay(pdMS_TO_TICKS(3000));

        setMotorState(true, true, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // STEP 4: 100 PWM BACKWARD
        Serial.println("\n=================================");
        Serial.println(">>> STEP 4: BACKWARD @ 100 PWM");
        Serial.println("=================================");
        setMotorState(true, false, 100);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // REST PERIOD
        Serial.println("\n>>> Sequence Complete! Resting for 3 seconds...");
        setMotorState(false, false, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// --- TASK 2: Speed Telemetry ---
void vSpeedTelemetryTask(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(100));

        noInterrupts();
        uint32_t count = pulseCount;
        pulseCount = 0;
        interrupts();

        Serial.print("[SC Pulses / 100ms]: ");
        Serial.println(count);
    }
}