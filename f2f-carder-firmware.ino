
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
  byte enablePin;     // Enable pin number
  byte directionPin;  // Direction pin number
  byte pulsePin;      // Pulse pin number

  // Trackers
  byte enable;                    // 0 if the stepper is disabled, 1 if the stepper is enabled.
  byte direction;                 // Determines if the stepper direction pin should be set or not. 
  unsigned int ticksPerStep;      // The number active number of ticks to count per step. If zero, the stepper is stopped. This is what the stepper is counting until.
  unsigned int goalTicksPerStep;  // The number of ticks per step that this stepper is configured to ramp up or down to. This is the "goal speed" and what ticksPerStep will be set to on "start"
  unsigned int tickCounter;       // The current number of ticks that have been done on this stepper
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

const char* errorMessage[] = {
  "",
  "The entered command exceeds the maximum command length.",
  "The entered command could not be tokenized.",
  "The specified drum index was not valid.",
};

byte error = ERROR_OK;
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
  }
}

#define MAX_COMMAND_LENGTH 16

char command[MAX_COMMAND_LENGTH + 1];
byte commandLength = 0;

// Takes a string, and attempts to parse it to a valid drum index. If it contains non-digits, returns NUMBER_OF_DRUMS which is an invalid index.
byte parseDrum(char* pDrum) {
  char* i = pDrum;
  // Iterate over pDrum by getting a pointer to a character. If it's not a digit, return NUMBER_OF_DRUMS which is invalid.
  while (*i != '\0') {
    if (!isdigit(*i)) return NUMBER_OF_DRUMS;
    i++;
  }
  return atoi(pDrum);
}

void doStart() {
  char* pDrum = strtok(NULL, " \r\n");
  if (pDrum == NULL) {
    for(byte i = 0; i<NUMBER_OF_DRUMS; i++) {
      startStepper(i);
    }
  }
  else {
    byte drumIndex = parseDrum(pDrum);
    if (drumIndex >= NUMBER_OF_DRUMS) {
      error = ERROR_DRUM_OUT_OF_BOUNDS;
      return;
    }
    startStepper(drumIndex);
  }
}

void doStop() {
  char* pDrum = strtok(NULL, " \r\n");
  if (pDrum == NULL) {
    for(byte i = 0; i<NUMBER_OF_DRUMS; i++) {
      stopStepper(i);
    }
  }
  else {
    byte drumIndex = parseDrum(pDrum);
    if (drumIndex >= NUMBER_OF_DRUMS) {
      error = ERROR_DRUM_OUT_OF_BOUNDS;
      return;
    }
    stopStepper(drumIndex);
  }
}

void executeCommand(char* pCommand, byte commandLen) {

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

}

// Read characters from serial, and process command as it becomes available.
void readAndProcessInput() {
  while (Serial.available()) {
    char c = Serial.read();
    
    // If we find a semicolon, a full command has been found.
    if (c == ';') {
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
      command[commandLength++];
    }
  }
}

// Get a pointer to stepper struct, returns NULL if invalid index.
inline Stepper* getStepper(byte index) {
  if (index > NUMBER_OF_DRUMS) return NULL;
  return &(drums[index]);
}

void initializeSteppers(void) {
  for (byte i=0; i<NUMBER_OF_DRUMS; i++) {
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

inline void enableStepper(byte index){
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->enable = 1;
  digitalWrite(s->enablePin, HIGH);
}

inline void disableStepper(byte index){
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->enable = 0;
  digitalWrite(s->enablePin, LOW);
}

inline void startStepper(byte index) {
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  // TODO: change this to setting ramp rate, if that's something we're concerned with.
  s->ticksPerStep = s->goalTicksPerStep;
}

inline void stopStepper(byte index) {
  Stepper* s = getStepper(index);
  if (s == NULL) return;

  s->ticksPerStep = 0;
}

inline void setStepperDirection(byte index, byte direction){
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
  for (byte i=0; i<NUMBER_OF_DRUMS; i++){
    Stepper* s = getStepper(i);
    if (s == NULL) continue;

    doStepperTick(s);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing steppers...");

  initializeSteppers();
}

void loop() {
  checkForError();
  readAndProcessInput();
}
