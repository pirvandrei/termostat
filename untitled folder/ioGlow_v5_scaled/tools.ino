/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


#define trigPin D8
#define echoPin D7


void usSetup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}
float usValue = 0;
float usRead()
{
  long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 100000);
  if (duration != 0)
  {
    usValue = (float)((duration / 2) / 29.1) * 0.1f + usValue * 0.9f;
  }
  return usValue;


}

