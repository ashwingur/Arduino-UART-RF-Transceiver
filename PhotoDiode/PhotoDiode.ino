void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue0 = analogRead(A0);
  int sensorValue1 = analogRead(A1);
  int sensorValue2 = analogRead(A2);
  int sensorValue3 = analogRead(A3);
  int sensorValue4 = analogRead(A4);

  Serial.print("A0: ");
  Serial.print(sensorValue0);
  Serial.print(" A1: ");
  Serial.print(sensorValue1);
  Serial.print(" A2: ");
  Serial.print(sensorValue2);
  Serial.print(" A3: ");
  Serial.print(sensorValue3);
  Serial.print(" A4: ");
  Serial.println(sensorValue4);

  delay(100);
}
