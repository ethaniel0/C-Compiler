#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

// int numSteps = 4; // Number of steps in the sequence
// Define the sequence of steps for the stepper motor
// const byte stepSequence4[4] = {
//   B1100, // Step A: AB
//   B0110, // Step B: BC
//   B0011, // Step C: CD
//   B1001  // Step D: DA
// };
// or?
// const byte stepSequence4[4] = {
//   B1010, // Step A: AB
//   B0110, // Step B: BC
//   B0101, // Step C: CD
//   B1001  // Step D: DA
// };

// int stepSequence = 37740; // See above

inline void pinMode(int pin, int mode){
    int addr = 12288 + pin; // 2^12 + 2^13 + pin num
    __asm__(
            "sw (mode) 0((addr))"
            );
}
inline void digitalWrite(int pin, int level){
    int addr = 4096 + pin; // 2^13 + pin num
    __asm__(
            "sw (level) 0((addr))"
            );
}

inline void digitalRead(int pin){
    int addr = 4096 + pin; // 2^12 + pin num
    __asm__(
            "lw $v0 0((addr))"
            );
}

void stepMotor(int step) {
    // Output the step sequence
    int seq = 38250;
    int mod4 = step & 3;
    for (int i = 0; i < mod4; i += 1){
        seq = seq >> 4;
    }
    digitalWrite(0,seq & 1);
    digitalWrite(1,(seq >> 1) & 1);
    digitalWrite(2,(seq >> 2) & 1);
    digitalWrite(3,(seq >> 3) & 1);

//    int pin0_state = seq & 1;
//    int pin1_state = (seq >> 1) & 1;
//    int pin2_state = (seq >> 2) & 1;
//    int pin3_state = (seq >> 3) & 1;
//
//    __asm__("sw (pin0_state) 4096($0)");
//    __asm__("sw (pin1_state) 4097($0)");
//    __asm__("sw (pin2_state) 4098($0)");
//    __asm__("sw (pin3_state) 4099($0)");
}


pinMode(0,OUTPUT);
pinMode(1,OUTPUT);
pinMode(2,OUTPUT);
pinMode(3,OUTPUT);

digitalWrite(0,LOW);
digitalWrite(1,LOW);
digitalWrite(2,LOW);
digitalWrite(3,LOW);

int step = 0;

while(1) {
    stepMotor(step);
    step += 1;
    __asm__("print (step)");
}

