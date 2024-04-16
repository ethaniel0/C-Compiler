#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

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
//        stepMotor(0, 1, 2, 3);
//        step += 1;
    }
}
