#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

#define SERIAL_PIN 7

int us_sensor_dist()
{
    int trigger = 4;
    int echo = 5;
    digitalWrite(trigger, 0);
    delayMicroseconds(2);
    digitalWrite(trigger, 1);
    delayMicroseconds(10);
    digitalWrite(trigger, 0);
    int d = pulseIn(echo, 1, 23529); // max sensor dist ~4m
    return d;
}

void display_number(int num){

}

int main(){
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, INPUT);

    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);

    digitalWrite(0,LOW);
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);

    int delay_time = 5000;
    int stop_time = 100000;

    while(1) {
        for (int i = 0; i < 64; i += 1){
            stepMotor();
            delayMicroseconds(delay_time);
        }
        int dist = us_sensor_dist();
//        write_32_bit_serial(SERIAL_PIN, dist);
        digitalWrite(8, dist > 0);
        digitalWrite(9, dist > 1000);
        digitalWrite(10, dist > 2000);
        digitalWrite(11, dist > 3000);
        delayMicroseconds(stop_time);
    }
}
