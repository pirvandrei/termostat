
int motorASpeed = 1023;


#define motor_a_speed 5
#define motor_a_dir 0

#define motor_b_speed 4
#define motor_b_dir = 2

void setup() {
  // Debug console
  Serial.begin(9600);

  //motor spped

  // motor A
  pinMode(motor_a_speed, OUTPUT);  // speed
  pinMode(motor_a_dir, OUTPUT);

}
void loop() {

  // move forward
  analogWrite(motor_a_speed, motorASpeed);
  digitalWrite(motor_a_dir, 1);
  delay(3000);
  
  // stop
  analogWrite(motor_a_speed, 0);
  delay(3000);
  
  // backwards
  analogWrite(motor_a_speed, motorASpeed);
  digitalWrite(motor_a_dir, 0);
  delay(3000);


}


void stop() {
 // analogWrite(4, 0);
 // digitalWrite(2, motorAForward);

  /*
    analogWrite(5, 0);
    digitalWrite(0, motorAForward); */

}

void run() {
 //7 analogWrite(4, motorASpeed);
 // digitalWrite(2, motorAForward);


  // analogWrite(5, motorASpeed);
  // digitalWrite(0, motorAForward);

}
