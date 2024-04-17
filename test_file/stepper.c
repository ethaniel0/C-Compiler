//int _stepper_step = 0;
//void stepMotor() {
//    // const byte stepSequence4[4] = {
//    //   B1100, // Step A: AB
//    //   B0110, // Step B: BC
//    //   B0011, // Step C: CD
//    //   B1001  // Step D: DA
//    // };
//
//    int seq = 37740;
//    int mod4 = _stepper_step & 3;
//    for (int i = 0; i < mod4; i += 1){
//        seq = seq >> 4;
//    }
//    digitalWrite(0,seq & 1);
//    digitalWrite(1,(seq >> 1) & 1);
//    digitalWrite(2,(seq >> 2) & 1);
//    digitalWrite(3,(seq >> 3) & 1);
//    _stepper_step = (_stepper_step + 1) & 3;
//}