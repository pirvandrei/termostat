
int motorASpeed = 1023;
bool motorAForward = 0; 

void setup() {
  // Debug console
  Serial.begin(9600);

  //motor spped
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);

  //motor direction
  pinMode(5, OUTPUT); 
  pinMode(2, OUTPUT); 
}
void loop() {
  //spin out 

   run(); 

  delay(10000);
   stop();
   
} 
void stop(){
  analogWrite(4, 0);
    digitalWrite(2, motorAForward);

 
   analogWrite(5, 0);
   digitalWrite(0, motorAForward); 
    
  }
  
void run(){
  analogWrite(4, motorASpeed);
    digitalWrite(2, motorAForward);

 
   analogWrite(5, motorASpeed);
   digitalWrite(0, motorAForward); 
    
  }
