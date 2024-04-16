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