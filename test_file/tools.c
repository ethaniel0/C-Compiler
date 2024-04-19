inline void pinMode(int pin, int mode){
    // 2^12 + 2^13 + pin num
    __asm__("sw (mode) 12288((pin))");
}
inline void digitalWrite(int pin, int level){
    // 2^13 + pin num
    __asm__("sw (level) 4096((pin))");
}

inline int digitalRead(int pin){
    // 2^12 + pin num
    __asm__("lw $return 4096((pin))");
}

inline int micros(){
    __asm__("addi $return $3 0");
}

void delayMicroseconds(int m){
    __asm__(
            "addi $8 $3 0"
            "sub $9 $3 $8"
            "delayMicrosLoopStart:"
            "blt (m) $9 delayMicrosEnd"
            "sub $9 $3 $8"
            "j delayMicrosLoopStart"
            "delayMicrosEnd:"
            "add $0 $0 $0"
            );
}

int pulseIn(int puslsePin, int pulseLevel, int pulseMaxTime){
    int clk = micros();
    while (digitalRead(puslsePin) != pulseLevel){
        int now = micros();
        if (pulseMaxTime < now - clk){ return 0; }
    }
    clk = micros();
    while (digitalRead(puslsePin) == pulseLevel){
        int now = micros();
        if (pulseMaxTime < now - clk){ return 0; }
    }
    return micros() - clk;
}

void write_8_bit_serial(int pin, int val){
    // delay time is 1042 us for 9600 baud rate
    int bit_delay = 1042;
    digitalWrite(pin, 1);
    delayMicroseconds(bit_delay);
    for (int i = 0; i < 8; i += 1){
        digitalWrite(pin, val & 1);
//        val >>= 1;
//        delayMicroseconds(bit_delay);
    }
//    digitalWrite(pin, 1);
//    delayMicroseconds(bit_delay >> 2);
}

//void write_32_bit_serial(int pin, int val){
//    // delay time is 1042 us for 9600 baud rate
//    write_8_bit_serial(pin, val & 255);
//    write_8_bit_serial(pin, (val >> 8) & 255);
//    write_8_bit_serial(pin, (val >> 16) & 255);
//    write_8_bit_serial(pin, (val >> 24) & 255);
//}