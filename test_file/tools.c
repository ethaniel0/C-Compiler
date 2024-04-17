inline void pinMode(int pin, int mode){
    int addr = 12288 + pin; // 2^12 + 2^13 + pin num
    __asm__(
            "sw (mode) 0((addr))"
            );
}
inline void digitalWrite(int pin, int l){
    int addr = 4096 + pin; // 2^13 + pin num
    __asm__(
            "sw (l) 0((addr))"
            );
}

inline int digitalRead(int pin){
    int addr = 4096 + pin; // 2^12 + pin num
    __asm__(
            "lw $v0 0((addr))"
            );
}

inline int micros(){
    __asm__("addi $2 $3 0");
}

void delayMicroseconds(int m){
    int clk = micros();
    int now = micros();
    while (now - clk < m){now = micros();}
}

int pulseIn(int pin, int level, int maxTime){
    __asm__(
            "addi $8 $3 0"
            "addi $9 $3 0"
            "addi (pin) (pin) 4096"
            "lw $10 0((pin))"
            "pulseIn_loop:"
            "bne $10 (level) pulseIn_inner_loop"
            "j pulseIn_outloop"
            "pulseIn_inner_loop:"
            "lw $10 0((pin))"
            "sub $11 $3 $8"
            "blt (maxTime), $11, pulseIn_end"
            "j pulseIn_loop"
            "addi $8 $3 0"
            "lw $10 0((pin))"
            "pulseIn_loop2:"
            "bne $10 (level) pulseIn_end"
            "lw $10 0((pin))"
            "sub $11 $3 $8"
            "blt (maxTime), $11, pulseIn_end"
            "j pulseIn_loop2"
            "pulseIn_end:"
            "sub $2 $3 $8"
            ""
            );
//    int clk = micros();
//    int now = micros();
//    while (digitalRead(pin) != level){
//        now = micros();
//        if (now - clk > maxTime){ return 0; }
//    }
//    clk = micros();
//    while (digitalRead(pin) == level){
//        now = micros();
//        if (now - clk > maxTime){ return 0; }
//    }
//    clk = micros() - clk;
//    return clk;
}