#include <avr/io.h>
#include <avr/interrupt.h>
// Define the clock speed as being 16MHz. Setting this does not change the clock speed,
// only calculations that need to know the speed.
#define CLOCK_SPEED     16000000

#define NUMBER_OF_DRUMS 12

// Set this number to the drum that you want to use as the main drum
#define DRUM_MAIN 0

// Each one of these represents a drum. Set the number to the right of the
// definition to its respective pin number.
#define DRUM_0_ENABLE	0
#define DRUM_0_DIR	0
#define DRUM_0_PULSE 	0

#define DRUM_1_ENABLE	0
#define DRUM_1_DIR	0
#define DRUM_1_PULSE 	0

#define DRUM_2_ENABLE	0
#define DRUM_2_DIR	0
#define DRUM_2_PULSE 	0

#define DRUM_3_ENABLE	0
#define DRUM_3_DIR	0
#define DRUM_3_PULSE 	0

#define DRUM_4_ENABLE	0
#define DRUM_4_DIR	0
#define DRUM_4_PULSE 	0

#define DRUM_5_ENABLE	0
#define DRUM_5_DIR	0
#define DRUM_5_PULSE 	0

#define DRUM_6_ENABLE	0
#define DRUM_6_DIR	0
#define DRUM_6_PULSE 	0

#define DRUM_7_ENABLE	0
#define DRUM_7_DIR	0
#define DRUM_7_PULSE 	0

#define DRUM_8_ENABLE	0
#define DRUM_8_DIR	0
#define DRUM_8_PULSE 	0

#define DRUM_9_ENABLE	0
#define DRUM_9_DIR	0
#define DRUM_9_PULSE 	0

#define DRUM_10_ENABLE	0
#define DRUM_10_DIR	0
#define DRUM_10_PULSE 	0

#define DRUM_11_ENABLE	0
#define DRUM_11_DIR	0
#define DRUM_11_PULSE 	0

typedef struct {
  // Pin definitions
  uint8_t enablePin;     // Enable pin number
  uint8_t directionPin;  // Direction pin number
  uint8_t pulsePin;      // Pulse pin number

  // Trackers
  uint8_t enable;                    // 0 if the stepper is disabled, 1 if the stepper is enabled.
  uint8_t direction;                 // Determines if the stepper direction pin should be set or not. 
  uint16_t ticksPerStep;      // The number active number of ticks to count per step. If zero, the stepper is stopped. This is what the stepper is counting until.
  uint16_t goalTicksPerStep;  // The number of ticks per step that this stepper is configured to ramp up or down to. This is the "goal speed" and what ticksPerStep will be set to on "start"
  uint16_t tickCounter;       // The current number of ticks that have been done on this stepper
} Stepper;

// Set up an array of 12 steppers with their pin definitions. This is a totally
// unnecessary way of doing this but it lets me loop over the motors really easily.
Stepper drums[] = {
  {.enablePin = DRUM_0_ENABLE, .directionPin = DRUM_0_DIR, .pulsePin = DRUM_0_PULSE},
  {.enablePin = DRUM_1_ENABLE, .directionPin = DRUM_1_DIR, .pulsePin = DRUM_1_PULSE},
  {.enablePin = DRUM_2_ENABLE, .directionPin = DRUM_2_DIR, .pulsePin = DRUM_2_PULSE},
  {.enablePin = DRUM_3_ENABLE, .directionPin = DRUM_3_DIR, .pulsePin = DRUM_3_PULSE},
  {.enablePin = DRUM_4_ENABLE, .directionPin = DRUM_4_DIR, .pulsePin = DRUM_4_PULSE},
  {.enablePin = DRUM_5_ENABLE, .directionPin = DRUM_5_DIR, .pulsePin = DRUM_5_PULSE},
  {.enablePin = DRUM_6_ENABLE, .directionPin = DRUM_6_DIR, .pulsePin = DRUM_6_PULSE},
  {.enablePin = DRUM_7_ENABLE, .directionPin = DRUM_7_DIR, .pulsePin = DRUM_7_PULSE},
  {.enablePin = DRUM_8_ENABLE, .directionPin = DRUM_8_DIR, .pulsePin = DRUM_8_PULSE},
  {.enablePin = DRUM_9_ENABLE, .directionPin = DRUM_9_DIR, .pulsePin = DRUM_9_PULSE},
  {.enablePin = DRUM_10_ENABLE, .directionPin = DRUM_10_DIR, .pulsePin = DRUM_10_PULSE},
  {.enablePin = DRUM_11_ENABLE, .directionPin = DRUM_11_DIR, .pulsePin = DRUM_11_PULSE},
};

#define ERROR_OK 0
#define ERROR_MAX_COMMAND_LENGTH_REACHED 1
#define ERROR_FAILED_TO_TOKENIZE 2
#define ERROR_DRUM_OUT_OF_BOUNDS 3
#define ERROR_INVALID_CMD        4
#define ERROR_INVALID_INTEGER    5
#define ERROR_MUST_PROVIDE_DRUM  6
#define ERROR_MISSING_INT_ARG    7

const char* errorMessage[] = {
  "",
  "The entered command exceeds the maximum command length.",
  "The entered command could not be tokenized.",
  "The specified drum index was not valid.",
  "The specified command is not valid.",
  "The provided integer was not valid.",
  "The entered command requires a valid drum argument.",
  "The entered command is missing an integer argument.",
};

uint8_t error = ERROR_OK;
bool errorShouldHalt = false;

void checkForError(){
  if (error != ERROR_OK) {
    Serial.print("ERROR: ");
    Serial.println(errorMessage[error]);
    if (errorShouldHalt) {
      Serial.println("Error indicated that it is in an unrecoverable state. Halting.");
      // TODO: Clear interrupts
      while(1){}
    }
    error = ERROR_OK;
  }
}

#define MAX_COMMAND_LENGTH 16
#define ECHO_OUTPUT

#ifdef ECHO_OUTPUT
#define ECHO(X) Serial.print(X)
#endif
#ifndef ECHO_OUTPUT
#define ECHO(X) //
#endif

char command[MAX_COMMAND_LENGTH + 1];
uint8_t commandLength = 0;

// Takes a string, and attempts to parse it to a valid drum index. If it contains non-digits, returns NUMBER_OF_DRUMS which is an invalid index.
uint8_t parseDrum(char* pDrum) {
  char* i = pDrum;
  // Iterate over pDrum by getting a pointer to a character. If it's not a digit, return NUMBER_OF_DRUMS which is invalid.
  while (*i != '\0') {
    if (!isdigit(*i)) return NUMBER_OF_DRUMS;
    i++;
  }
  return atoi(pDrum);
}

uint16_t parseInteger(char* pInteger) {
  char* i = pInteger;
  while(*i != '\0') {
    if (!isdigit(*i)) error = ERROR_INVALID_INTEGER;
    i++;
  }
  return atoi(pInteger);
}

void doSetSpeed() {
  char* pDrum = strtok(NULL, " \r\n");
  if (pDrum == NULL) {
    error = ERROR_MUST_PROVIDE_DRUM;
    return;
  }

  uint8_t drumIndex = parseDrum(pDrum);
  if (drumIndex >= NUMBER_OF_DRUMS) {
    error = ERROR_MUST_PROVIDE_DRUM;
    return;
  }

  char* pSpeedString = strtok(NULL, " \r\n");
  if (pSpeedString == NULL) {
    error = ERROR_MISSING_INT_ARG;
    return;
  }
  
  uint8_t speedValue = parseInteger(pSpeedString);
  if (error != ERROR_OK) {
    return;
  }

  setStepperSpeed(drumIndex, speedValue);
}

void doStart() {
  char* pDrum = strtok(NULL, " \r\n");
  if (pDrum == NULL) {
    for(uint8_t i = 0; i<NUMBER_OF_DRUMS; i++) {
      startStepper(i);
    }
  }
  else {
    uint8_t drumIndex = parseDrum(pDrum);
    if (drumIndex >= NUMBER_OF_DRUMS) {
      error = ERROR_DRUM_OUT_OF_BOUNDS;
      return;
    }
    startStepper(drumIndex);
  }
  Serial.println("Started.");
}

void doStop() {
  char* pDrum = strtok(NULL, " \r\n");
  if (pDrum == NULL) {
    for(uint8_t i = 0; i<NUMBER_OF_DRUMS; i++) {
      stopStepper(i);
    }
  }
  else {
    uint8_t drumIndex = parseDrum(pDrum);
    if (drumIndex >= NUMBER_OF_DRUMS) {
      error = ERROR_DRUM_OUT_OF_BOUNDS;
      return;
    }
    stopStepper(drumIndex);
  }
  Serial.println("Stopped.");
}

void executeCommand(char* pCommand, uint8_t commandLen) {

  char* verb = strtok(pCommand, " \r\n");
  if (verb == NULL) {
    error = ERROR_FAILED_TO_TOKENIZE;
    return;
  }

  if (strcmp(verb, "help") == 0) {

  }
  else if (strcmp(verb, "setspeed") == 0) {

  }
  else if (strcmp(verb, "setramp") == 0) {

  }
  else if (strcmp(verb, "setdirection") == 0) {

  }
  else if (strcmp(verb, "getsettings") == 0) {

  }
  else if (strcmp(verb, "start") == 0) {
    doStart(); 
  }
  else if (strcmp(verb, "stop") == 0) {
    doStop();
  }
  else {
    error = ERROR_INVALID_CMD;
  }

}

// Read characters from serial, and process command as it becomes available.
void readAndProcessInput() {
  while (Serial.available()) {
    char c = Serial.read();
    ECHO(c);
    
    // If we find a semicolon, a full command has been found.
    if (c == ';') {
      ECHO("\n");
      // Terminate and then parse the command
      command[commandLength] = '\0';
      executeCommand(command, commandLength);
      // Reset the command buffer
      command[0] = '\0';
      commandLength = 0;

      // Return after executing the command so that we can check for errors
      return;
    }
    else {
      if (commandLength >= MAX_COMMAND_LENGTH) {
        // If the new character puts us beyond the max command length, set an error and zero the command.
        error = ERROR_MAX_COMMAND_LENGTH_REACHED;
        command[0] = '\0';
        commandLength = 0;
        return;
      } 
      command[commandLength++] = c;
    }
  }
}

// Get a pointer to stepper struct, returns NULL if invalid index.
inline Stepper* getStepper(uint8_t index) {
  if (index > NUMBER_OF_DRUMS) return NULL;
  return &(drums[index]);
}

void initializeSteppers(void) {
  for (uint8_t i=0; i<NUMBER_OF_DRUMS; i++) {
    Stepper* s = getStepper(i);
    // Initialize status items to zero.
    s->enable = 0;
    s->direction = 0;
    s->ticksPerStep = 0;
    s->goalTicksPerStep = 0;
    s->tickCounter = 0;
    // Initialize pins to be outputs
    pinMode(s->enablePin, OUTPUT);
    pinMode(s->directionPin, OUTPUT);
    pinMode(s->pulsePin, OUTPUT);
  }
}


void initializeTickInterrupt(uint16_t frequency_hz) {

  Serial.println("Initializing tick interrupt...");

  // Clear control registers
  TCCR4A = 0;
  TCCR4B = 0;

  // Initialize the timer counter to 0
  TCNT4 = 0;

  // Find the prescaler value that has the greatest resolution.
  // For 1000Hz this should be 64, for 1Hz this should be 1024.
  uint16_t prescalerValue = 0;
  uint8_t tccr4bValue = 0;
  for (uint16_t prescalerOption = 1024; prescalerOption >= 1; prescalerOption /= 2) {
      uint32_t interruptPeriod = CLOCK_SPEED / (prescalerOption * frequency_hz);
      if (interruptPeriod >= 1) {
        prescalerValue = prescalerOption;
        if (prescalerValue == 1024)
            tccr4bValue = _BV(CS42) | _BV(CS40);  // Set prescaler to 1024
        else if (prescalerValue == 256)
            tccr4bValue = _BV(CS42);  // Set prescaler to 256
        else if (prescalerValue == 64)
            tccr4bValue = _BV(CS41) | _BV(CS40);  // Set prescaler to 64
        else if (prescalerValue == 8)
            tccr4bValue = _BV(CS41);  // Set prescaler to 8
        else if (prescalerValue == 1)
            tccr4bValue = _BV(CS40);  // Set prescaler to 1
          break;
      }
  }

  // Set timer ticks per motion tick
  // timer ticks = clock speed / (prescaler * desired frequency in Hz) - 1
  // We subtract 1 because the timer start at zero.
  // Prescaler is chosen to have the greatest 
  OCR4A = (CLOCK_SPEED / (prescalerValue * frequency_hz)) - 1;

  TCCR4B |= tccr4bValue;

  // Enable CTC (Clear Timer on Compare match)
  TCCR4B |= (1 << WGM42);

  // Enable the timer compare interrupt
  TIMSK4 |= (1 << OCIE4A);

}

void initializeInterrupts(void) {
  // Disable interrupts
  cli();

  // Configure interrupt registers
  initializeTickInterrupt(1);

  Serial.println("Enabling global interrupts...");
  // Re-enable interrupts
  sei();
}

inline void enableStepper(uint8_t index){
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->enable = 1;
  digitalWrite(s->enablePin, HIGH);
}

inline void disableStepper(uint8_t index){
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->enable = 0;
  digitalWrite(s->enablePin, LOW);
}

inline void startStepper(uint8_t index) {
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  // TODO: change this to setting ramp rate, if that's something we're concerned with.
  s->ticksPerStep = s->goalTicksPerStep;
}

inline void stopStepper(uint8_t index) {
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->ticksPerStep = 0;
}

inline void setStepperSpeed(uint8_t index, uint16_t speed) {
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->goalTicksPerStep = speed;

  // TODO: figure out ramp or something idk.
}

inline void setStepperDirection(uint8_t index, uint8_t direction){
  Stepper* s = getStepper(index);
  if (s == NULL) return;
  if (direction > 1) return;

  s->direction = direction;
  // This is dumb because LOW is probably 0 and HIGH is probably 1 but I'm too lazy to check.
  // The compiler will fix it anyway.
  digitalWrite(s->directionPin, ((direction == 0) ? LOW : HIGH));
}

// Set the pulse value of the stepper to HIGH or LOW
inline void setStepperPulse(Stepper* s, char value) {
  digitalWrite(s->pulsePin, value);
}

// Things to do at the end of a step, such as adjusting the current ticks per step for ramping purposes.
inline void doEndOfStepActivities(Stepper* s) {
  
}

inline void doStepperTick(Stepper* s) {
  s->tickCounter++;
  // If we've reached our tick count, start over. This includes the case where ticksPerStep is 0 (stopped)
  if (s->tickCounter >= s->ticksPerStep) {
    s->tickCounter = 0;
    doEndOfStepActivities(s);
  }
  
  // If the number of ticks is less than half of the ticks per step, set the pulse value low.
  // When it passes the halfway point, trigger a rising edge to do a step.
  if (s->tickCounter < (s->ticksPerStep / 2)) {
    setStepperPulse(s, LOW);
  }
  else
  {
    setStepperPulse(s, HIGH);
  }
}

void tick() {
  Serial.println("Tick!");
  for (uint8_t i=0; i<NUMBER_OF_DRUMS; i++){
    Stepper* s = getStepper(i);
    if (s == NULL) continue;

    doStepperTick(s);
  }
}

// Interrupt handler to do a tick on each interrupt
ISR(TIMER4_COMPA_vect) {
  tick();
}

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing steppers...");
  initializeSteppers();

  Serial.println("Initializing interrupts...");
  initializeInterrupts();
}

void loop() {
  checkForError();
  readAndProcessInput();
}
