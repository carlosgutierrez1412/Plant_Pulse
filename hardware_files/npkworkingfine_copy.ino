#include <SoftwareSerial.h>
#include <DHT.h>

// Define pins for SoftwareSerial
#define RX 2  // Connect to RO on MAX485
#define TX 3  // Connect to DI on MAX485
// Define pins for DHT22
#define DHTPIN 4
#define DHTTYPE DHT22

SoftwareSerial RS485(RX, TX);  // Create SoftwareSerial object
DHT dht(DHTPIN, DHTTYPE); //Create DHT22 object

// Function to initialize MAX485 communication
void setupMax485() {
  pinMode(8, OUTPUT);  // RE pin
  pinMode(7, OUTPUT);  // DE pin
  digitalWrite(8, LOW);  // Enable Receiver
  digitalWrite(7, LOW);  // Disable Transmitter
}

// void setupDHT22(){
//   dht.setup(4, DHT22); //DHT sensor to digital pin 4 on Nano R3
// }

void setup() {
  Serial.begin(9600);
  RS485.begin(4800);  // Baud rate to communicate with the soil sensor
  setupMax485();
  dht.begin();
}


//function for master/slave send & response request
void sendRequest() {
  // Prepare the Modbus request frame
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  
  // Switch to transmission mode
  digitalWrite(8, HIGH);  // Disable Receiver
  digitalWrite(7, HIGH);  // Enable Transmitter
  RS485.write(queryData, sizeof(queryData));  // Send request
  RS485.flush();
  
  // Switch back to reception mode
  digitalWrite(7, LOW);  // Disable Transmitter
  digitalWrite(8, LOW);  // Enable Receiver
  
  delay(10000);  // Wait for sensor to respond, in seconds
}



void loop() {
  sendRequest();
  
  float hum = dht.readHumidity();
  float cel = dht.readTemperature();
  float fah = dht.readTemperature(true);
  if(isnan(hum) || isnan(cel) || isnan(fah)){
    Serial.println("Failed to read from DHT22 Sensor!");
  } else {
    Serial.println("\n\t DHT22 Readings (Outside Soil):");
    Serial.print("Humidity (0% - 100% RH): \t\t"); Serial.print(hum); Serial.println("%");
    Serial.print("Temperature (-40.0 C - +80.0 C): \t"); Serial.print(cel); 
    Serial.print(" Celsius  -  "); Serial.print(fah); Serial.println(" Fahrenheit");
  }

  if (RS485.available()) {
    byte receivedData[19];  // Buffer to hold response data
    if (RS485.readBytes(receivedData, sizeof(receivedData)) == sizeof(receivedData)) {
      // Parse and print the received data in decimal format
      unsigned int humidity = (receivedData[3] << 8) | receivedData[4];
      unsigned int temperature = (receivedData[5] << 8) | receivedData[6];
      unsigned int pH = (receivedData[9] << 8) | receivedData[10];
      unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
      unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
      unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

      Serial.println();
      Serial.println("\tRS485 Readings (Inside Soil):");
      Serial.print("Humidity: (0% - 100% RH): \t\t"); Serial.print((float)humidity / 10.0); Serial.println("%");
      Serial.print("Temperature: (-40.0 C - +80.0 C): \t"); Serial.print((float)temperature / 10.0); Serial.print(" Celsius  -  ");
      Serial.print(1.8*((float)temperature/10.0) + 32.0); Serial.println(" Fahrenheit");
      Serial.print("pH: (3.0-9.0): \t\t\t\t"); Serial.println((float)pH / 10.0);
      Serial.print("Nitrogen (0-1000mg/kg): \t\t"); Serial.println(nitrogen);
      Serial.print("Phosphorus (0-1000mg/kg): \t\t"); Serial.println(phosphorus);
      Serial.print("Potassium (0-1000g/kg): \t\t"); Serial.println(potassium);
      Serial.println();
    }
  } else {
    Serial.println("\n Failed to read data from RS485 sensor, attempting to read again!");
  }
  delay(2000);  // Wait before sending the next request
}
