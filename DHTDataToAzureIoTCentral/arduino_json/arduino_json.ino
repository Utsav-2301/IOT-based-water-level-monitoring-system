#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <EduIntro.h>    //Include the DHT library for the temperature - humidity sensor


#define TRIG_PIN 4  //Connect TRIG pin to digital pin 4
#define ECHO_PIN 5  //Connect ECHO pin to digital pin 5
DHT11 dht11(12); //Connect Signal pin from DHT11 sensor to digital pin 12
SoftwareSerial nodemcu(10, 11);//Initialise Arduino to NodeMCU (10=Rx & 11=Tx)


void setup() {
 pinMode(TRIG_PIN, OUTPUT);                        //Set the TRIG pin to OUTPUT mode
 pinMode(ECHO_PIN, INPUT);                         //Set the ECHO pin to INPUT mode  
 Serial.begin(9600);
 nodemcu.begin(9600);
 delay(500);                                       //Delay to let system boot
 Serial.println("Accurate distance sensing\n\n");  //Write a welcoming message to serial monitor
 delay(1000);                                      //Wait before accessing the DHT11 Sensor
}
void loop() {
  dht11.update();
  float humidity = dht11.readHumidity();
  float temp = dht11.readCelsius();           //Read the data from the DHT sensor
 float p = 101000;               //Set atmospheric pressure to 101.000 kPa
 //Use the fromula from http://gsd.ime.usp.br/~yili/SpeedOfSound/Speed.html to evaluate speed of sound
 float a0 = 331.5024;
 float a1 = 0.603055;
 float a2 = -0.000528;
 float a3 = 51.471935;
 float a4 = 0.1495874;
 float a5 = -0.000782;
 float a6 = -1.82e-7;       
 float a7 = 3.73e-8;         
 float a8 = -2.93e-10;     
 float a9 = -85.20931;
 float a10 = -0.228525;
 float a11 = 5.91e-5;  
 float a12 = -2.835149;
 float a13 = -2.15e-13; 
 float a14 = 29.179762;
 float a15 = 0.000486;
 float T = temp + 273.15;
 float h = humidity /100.0;
 float f = 1.00062 + 0.0000000314 * p + 0.00000056 * temp * temp;
 float Psv = exp(0.000012811805 * T * T - 0.019509874 * T + 34.04926034 - 6353.6311 / T);
 float Xw = h * f * Psv / p;
 float c = 331.45 - a0 - p * a6 - a13 * p * p;
 c = sqrt(a9 * a9 + 4 * a14 * c);
 float Xc = ((-1) * a9 - c) / ( 2 * a14);
 float speedOfSound = a0 + a1 * temp + a2 * temp * temp + (a3 + a4 * temp + a5 * temp * temp) * Xw + (a6 + a7 * temp + a8 * temp * temp) * p + (a9 + a10 * temp + a11 * temp * temp) * Xc + a12 * Xw * Xw + a13 * p * p + a14 * Xc * Xc + a15 * Xw * p * Xc;
 //Send a short (10 microseconds) ultrasonic burst 
 digitalWrite(TRIG_PIN, HIGH);
 delayMicroseconds(10);
 digitalWrite(TRIG_PIN, LOW);
 float microseconds = pulseIn(ECHO_PIN, HIGH, 100000); //Mesure the duration of a HIGH pulse in echo pin in microseconds. Timeout in 0,1 seconds
 float seconds = microseconds / 1000000;               //Convert microseconds to seconds
 float meters = seconds * speedOfSound;                //Get the distance in meters using the speed of sound calculated earlier
 float cm = meters * 100;                              //Convert meters to cm
 cm = cm/2;                                            //We only want the distance to the obstacle and not the roundtrip

  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  //Assign collected data to JSON Object
  data["humidity"] = humidity;
  data["temperature"] = temp; 
  data["distance"] = cm;
  data["speed"] = speedOfSound;

  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();
  delay(100); 
}
