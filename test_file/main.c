__asm__(
        "addi $8 $0 1"
        "sw $8 12288($0)" // set output mode
        "sw $8 12289($0)"
        "sw $8 12290($0)"
        "sw $8 12291($0)"
        // digital writes setup (write 0 to the pins)
        "sw $0 4096($0)"
        "sw $0 4097($0)"
        "sw $0 4098($0)"
        "sw $0 4099($0)"
        // stepper step
        "sw $0 0($0)"
        "j mainloop"
        // step motor
        "step:"
        "addi $8 $0 37740"  // $8 = seq
        "lw $9 0($0)"        // load step
        "addi $10 $0 3"
        "and $10 $9 $10"    // $10 = step % 4
        "addi $11 $0 0"     // loop for step times, shift seq by 4
        "step_loop_start: bne $11 $10 step_loop_end"
        "sra $8 $8 4"
        "addi $11 $11 1"
        "j step_loop_start"
        "step_loop_end:"
        "addi $12 $0 1"     // we're gonna & 1 a bunch

        "and $13 $13 $12"
        "sw $13 4096($0)"   // digitalWrite(0,seq & 1);

        "sra $13 $8 1"
        "and $13 $8 $12"
        "sw $13 4097($0)"   // digitalWrite(1,(seq >> 1) & 1);

        "sra $13 $8 2"
        "and $13 $8 $12"
        "sw $13 4098($0)"   // digitalWrite(3,(seq >> 2) & 1);

        "sra $13 $8 4"
        "and $13 $8 $12"
        "sw $13 4099($0)"   // digitalWrite(3,(seq >> 23 & 1);

        "addi $9 $9 1"
        "addi $13 $0 3"
        "and $9 $9 $13"
        "sw $9 0($0)"        // _stepper_step = (_stepper_step + 1) & 3;
        "jr $31"
        // keep stepping
        "mainloop:"
        "jal step"
        "j mainloop"
        );

//
//#define HIGH 1
//#define LOW 0
//#define INPUT 0
//#define OUTPUT 1
//
//int main(){
//    pinMode(0,OUTPUT);
//    pinMode(1,OUTPUT);
//    pinMode(2,OUTPUT);
//    pinMode(3,OUTPUT);
//
//    digitalWrite(0,LOW);
//    digitalWrite(1,LOW);
//    digitalWrite(2,LOW);
//    digitalWrite(3,LOW);
//
////    stepMotor();
////    for (int i = 0; i > 12000; i += 1){}
////    stepMotor();
//
//    while(1) {
////        stepMotor();
//        digitalWrite(0, HIGH);
//        for (int i = 0; i < 300; i += 1){}
//        digitalWrite(0, LOW);
//        for (int i = 0; i < 300; i += 1){}
//    }
////
//    while(1){}
//}
