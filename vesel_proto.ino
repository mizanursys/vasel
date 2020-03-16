#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>



/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.
 
 */

byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;



TinyGPSPlus gps;  // The TinyGPS++ object

SoftwareSerial ss(D4, D3);

const char *ssid = "Workspace";  //ENTER YOUR WIFI ssid
const char *password = "whoneedsit???";  //ENTER YOUR WIFI password

float latitude , longitude;
String lat_str , lng_str;

//sonar
const int trigPin = D4;
const int echoPin = D3;
// defines variables
long duration;
int distance;
//ir
//int ledPin = D5; // choose pin for the LED
int inputPin = D6; // choose input pin (for Infrared sensor)
int val = 0; // variable for reading the pin status
int value = 0;
int voltage;



void setup() {
  
  Serial.begin(115200);
  //sonar
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  //ir
  //pinMode(ledPin, OUTPUT); // declare LED as output
  pinMode(inputPin, INPUT); // declare Infrared sensor as input
    // Initialize a serial connection for reporting values to the host
  
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  
}


void loop() {
  Serial.begin(115200);
  SendSensorData();
  //sonar
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  
}


  void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}



//function to send sensor data
void SendSensorData() {
  distance = duration * 0.034 / 2;
  
  //voltage
  value = analogRead(A0);
  voltage = value * 5.0/1023;
  
  Serial.print("Voltage=== ");
  Serial.println(voltage);
  
  Serial.print("Distance=== ");
  Serial.println(distance);
  
  //ir
  val = digitalRead(inputPin); // read input value
  if (val == HIGH)
  { // check if the input is HIGH
   // digitalWrite(ledPin, LOW); // turn LED OFF
    Serial.print("Ir Sensor====");
    Serial.println("off");
  }
  else
  {
   Serial.print("Ir Sensor====");
    Serial.print("on");
  }


     if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate===: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.print("L");
    

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

  delay(5000);  //Post Data at every 5 seconds
}
