/*
 * HC-SR04 sonar module
 *
 * Doc: https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE
 */

#define trigPin 13
#define echoPin 12
#define led1 10  // Object close LED
#define led2 9  // No object close LED

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
}

void loop() {
  long duration, distance;
  
  // Initiate by sending a 10us pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);  // Read back pulse length of echo
  distance = (duration/2) / 29.1;    // Calculate distance in centimeters
  
  if (distance < 10) {
    digitalWrite(led1,HIGH); // When the Red condition is met, the Green LED should turn off
    digitalWrite(led2,LOW);
  } else {
    digitalWrite(led1,LOW);
    digitalWrite(led2,HIGH);
  }
  
  if (distance >= 200 || distance <= 0) {
    Serial.println("Out of range");
  } else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);
}
