#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>

#define DHTPIN 3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);

int fanPin=4;
int fanPin1=5;

String inString = "";

int moistureSensorPin = A0;
int methaneSensorPin = A1;

int servoPin = 8;
Servo servo;
int action = 0;

int pumpPin = 7;
int pumpTimeToRun = 5000;

int waterLevelPin = 6;

int ventAngle=0;
int startPump=0;

int trigPin=11;
int echoPin=12;
double duration, cm, inches;

double binDepth=30;

int AmbientTemperature;
int Temperature;
int Moisture;
int MethanePPM;
int WaterLevel;
int ScrapeLevel;

void setup() {
  Serial.begin(9600);
  
  pinMode(pumpPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(fanPin1, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  dht.begin();
  delay(1000);
  sensor.begin();

  servo.attach(servoPin);
  servo.write(action);

  
  Serial.println("================================");
}

void loop() {
  readSensors();
  Serial.println("");
  Serial.println("==============================================");
  analyze(AmbientTemperature,Temperature, Moisture, MethanePPM, WaterLevel, ScrapeLevel);
  Serial.println("==============================================");
  delay(1000);
  Serial.println("");
  Serial.println("");
}

void readSensors() {

  AmbientTemperature=getAmbientTemperature();
  Temperature=getTemperature();
  Moisture=getMoisture();
  MethanePPM=getMethanePPM();
  WaterLevel=getWaterLevel();
  ScrapeLevel=getScrapeLevel();
}

int getAmbientTemperature() {
  Serial.print("Humidity: ");
  Serial.print(dht.readHumidity());
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(dht.readTemperature(true));
  Serial.println(" *F");
  return round(dht.readTemperature(true));
}

int getTemperature() {
  sensor.requestTemperatures();
  int sensorValue =  sensor.getTempFByIndex(0);
  Serial.print("Compost Temperature is: ");
  if (sensorValue < 0) {
    sensorValue = 0;
  }
  Serial.print(sensorValue);
  Serial.println(" *F");
  return sensorValue;
}

int getMoisture() {
  float sensorValue = analogRead(moistureSensorPin);
  int calcSensorValue;

  if (sensorValue < 750) {
    calcSensorValue = 30;
  }
  else if (sensorValue < 900) {
    calcSensorValue = 50;
  }
  else {
    calcSensorValue = 65;
  }
  
  Serial.print("Compost Moisture is: ");
  Serial.print(100-calcSensorValue);
  Serial.println(" %");
  return 100-calcSensorValue;
}

int getMethanePPM() {
  float sensorValue = analogRead(methaneSensorPin);
  double ppm = 10.938 * exp(1.7742 * (sensorValue * 5.0 / 4095));
  
  if (ppm < 0) {
    ppm = 0;
  }
  Serial.print("Methane PPM is: ");
  Serial.println(ppm);
  return ppm;
}

int getWaterLevel() {
  pinMode(waterLevelPin, INPUT_PULLUP);
  
  int waterLevelValue = digitalRead(waterLevelPin);
  if (waterLevelValue < 0) {
    waterLevelValue = 0;
  }
  Serial.print("Water Level is: ");
  Serial.println(waterLevelValue);
  return waterLevelValue;
}

int getScrapeLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);
  duration=pulseIn(echoPin, HIGH);
  cm=(duration/2)/29.1;
//  inches=(duration/2)/74;

  Serial.print("Scrape level = ");
//  Serial.print(round(inches));
//  Serial.print(" in, ");
  Serial.print(round(cm));
  Serial.println(" cm");

  double binFill = binDepth;
  if (cm <3) {
    cm =0;
  }
  if (cm>binDepth) {
    Serial.println("There was an error");
  }
  else {
    binFill = binDepth - cm;
  }
  Serial.println(binFill);
  delay(250);
  return binFill;
}

void analyze(int AmbientTemperature,int Temperature, int Moisture, int MethanePPM, int WaterLevel, int ScrapeLevel) {
  if (Moisture>60) {
    servo.write(180);
    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost moisture is too wet. Turn your compost and add 'green' (Nitrogen-rich) materials.");
    if (Temperature<90) {
    servo.write(0);
    digitalWrite(fanPin, HIGH);
    digitalWrite(fanPin1, LOW);
    if (Temperature<=AmbientTemperature) {
      Serial.println("Your compost temperature is lower than optimal. Turn compost and add 'green' (Nitrogen-rich) materials.");
    }
    else {
      Serial.println("Your compost temperature is lower than optimal. The ambient temperature is low, so you should cover your compost to continue aerobic composting.");
    }
  }
  else if (Temperature>=90 && Temperature<=140) {
    Serial.println("Your compost is at optimal temperature.");
  }
  else if (Temperature>=140 && Temperature<=160) {
    servo.write(180);
    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost temperature is slightly higher than optimal. You may want to turn the compost and add 'brown' materials.");
  }
  else if (Temperature>=160 && Temperature<=175) {
    servo.write(180);
    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached unhealthily temperature. Turn compost and add 'brown' (Carbon-rich) materials.");
  }
  else if (Temperature>175) {
    servo.write(180);
    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached an unsafe temperature. Immediately turn the compost and add water.");
    digitalWrite(pumpPin, HIGH);
    delay(pumpTimeToRun);
    digitalWrite(pumpPin, LOW);
  }
  }
  else if (Moisture>=40 && Moisture<=60) {
    servo.write(0);

    digitalWrite(fanPin, HIGH);
    digitalWrite(fanPin1, LOW);
    Serial.println("Your compost is at optimal moisture levels.");
    if (Temperature<90) {
    servo.write(0);

    digitalWrite(fanPin, HIGH);
    digitalWrite(fanPin1, LOW);
    if (Temperature<=AmbientTemperature) {
      Serial.println("Your compost temperature is lower than optimal. Turn compost and add 'green' (Nitrogen-rich) materials.");
    }
    else {
      Serial.println("Your compost temperature is lower than optimal. The ambient temperature is low, so you should cover your compost to continue aerobic composting.");
    }
  }
  else if (Temperature>=90 && Temperature<=140) {
    Serial.println("Your compost is at optimal temperature.");
  }
  else if (Temperature>=140 && Temperature<=160) {
    servo.write(180);

    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost temperature is slightly higher than optimal. You may want to turn the compost and add 'brown' materials.");
  }
  else if (Temperature>=160 && Temperature<=175) {
    servo.write(180);

    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached unhealthily temperature. Turn compost and add 'brown' (Carbon-rich) materials.");
  }
  else if (Temperature>175) {
    servo.write(180);

    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached an unsafe temperature. Immediately turn the compost and add water.");
    digitalWrite(pumpPin, HIGH);
    delay(pumpTimeToRun);
    digitalWrite(pumpPin, LOW);
  }
  }
  else if (Moisture<40) {
    servo.write(0);

    digitalWrite(fanPin, HIGH);
    digitalWrite(fanPin1, LOW);
    Serial.println("Your compost is too dry and requires your attention. You need to turn and water your compost.");
    digitalWrite(pumpPin, HIGH);
    delay(pumpTimeToRun);
    digitalWrite(pumpPin, LOW);
    if (Temperature<90) {
    servo.write(0);
    digitalWrite(fanPin, HIGH);
    digitalWrite(fanPin1, LOW);
    if (Temperature<=AmbientTemperature) {
      Serial.println("Your compost temperature is lower than optimal. Turn compost and add 'green' (Nitrogen-rich) materials.");
    }
    else {
      Serial.println("Your compost temperature is lower than optimal. The ambient temperature is low, so you should cover your compost to continue aerobic composting.");
    }
  }
  else if (Temperature>=90 && Temperature<=140) {
    Serial.println("Your compost is at optimal temperature.");
  }
  else if (Temperature>=140 && Temperature<=160) {
    servo.write(180);
    
    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost temperature is slightly higher than optimal. You may want to turn the compost and add 'brown' materials.");
  }
  else if (Temperature>=160 && Temperature<=175) {
    servo.write(180);

    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached unhealthily temperature. Turn compost and add 'brown' (Carbon-rich) materials.");
  }
  else if (Temperature>175) {
    servo.write(180);

    digitalWrite(fanPin, LOW);
    digitalWrite(fanPin1, HIGH);
    Serial.println("Your compost has reached an unsafe temperature. Immediately turn the compost and add water.");
    digitalWrite(pumpPin, HIGH);
    delay(pumpTimeToRun);
    digitalWrite(pumpPin, LOW);
  }
  }
  if (WaterLevel==0) {
    Serial.println("Your need to add water to your water reservoir");
  }
  if (ScrapeLevel<10) {
    Serial.println("Your dustbin is full. Move scrape to compostbin");
  }
}
