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
int pins[4] = {0, 1, 2, 3};


void pinMode(int pin, int mode){
    int addr = 12288 + pin; // 2^12 + 2^13 + pin num
    __asm__(
            "sw (mode) 0((addr))"
            );
}
void digitalWrite(int pin, int level){
    int addr = 4096 + pin; // 2^13 + pin num
    __asm__(
            "sw (level) 0((addr))"
            );
}

void digitalRead(int pin){
    int addr = 4096 + pin; // 2^12 + pin num
    __asm__(
            "lw $v0 0((addr))"
            );
}

void stepMotor(int step) {
    // Output the step sequence
    int seq = 37740;
    int mod4 = step & 3;
    for (int i = 0; i < mod4; i += 1){
        seq = seq >> 4;
    }
    int pin0_state = seq & 1;
    int pin1_state = (seq >> 1) & 1;
    int pin2_state = (seq >> 2) & 1;
    int pin3_state = (seq >> 3) & 1;

    __asm__("sw (pin0_state) 4096($0)");
    __asm__("sw (pin1_state) 4097($0)");
    __asm__("sw (pin2_state) 4098($0)");
    __asm__("sw (pin3_state) 4099($0)");
}

int main(){

    pinMode(0,OUTPUT);
    pinMode(1,OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);

    digitalWrite(0,LOW);
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);

    int step = 0;
    int clk = 0;

    while(1) {
        stepMotor(step);
//        for (int i = 0; i < 1000; i += 1){}
        step += 1;
//
//        int c = 0;
//        __asm__("addi (c) $3 0");
//        if (c - clk > 50){
//            stepMotor(step);
//            __asm__("print (step)");
//            step += 1;
//        }
    }
}
