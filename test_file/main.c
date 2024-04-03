//
// Created by Ethan Horowitz on 4/2/24.
//

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

int* malloc(int size){
    __asm__(
        "add $v0 $28 $a0"
        "add $28 $28 (size)"
    );
}
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

void wait(){

    for (int i = 0; i < 1000; i += 1){
        for (int j = 0; j < 1000; j += 1){
        }
    }
}

int main() {
    //////////////////////
    int pin = 1;

    int addr = 12288 + pin; // 2^12 + 2^13 + pin num
    __asm__(
        "sw (pin) 0((addr))"
    );

//    pinMode(1, OUTPUT);
    int level = 1;
    addr = 4096 + 1; // 2^13 + pin num
    for(;;){
        __asm__(
        "sw (level) 0((addr))"
        );
        __asm__(
            "sw $0 0((addr))"
            );
        digitalWrite(1, HIGH);
//        wait();
        digitalWrite(1, LOW);
//        wait();
    }
}