/*
  DHCP-based IP printer

  This sketch uses the DHCP extensions to the Ethernet library
  to get an IP address via DHCP and print the address obtained.
  using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12 April 2011
  modified 9 Apr 2012
  by Tom Igoe
  modified 02 Sept 2015
  by Arturo Guadalupi

*/

#include <SPI.h>
#include <Ethernet.h>
 /* The circuit:
 * LCD RS pin to digital pin 2
 * LCD Enable pin to digital pin 3
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 8
 * LCD D6 pin to digital pin 5
 * LCD D7 pin to digital pin 6
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 8, 5, 6);

#include "DHT.h"

DHT dht;

#define AM2302_PIN 7     // what pin on the arduino is the DHT22 data line connected to

// For details on how to hookup the DHT22 sensor to the Arduino then checkout this page
// http://cactus.io/hookups/sensors/temperature-humidity/am2302/hookup-arduino-to-am2302-temp-humidity-sensor

// Initialize DHT sensor for normal 16mhz Arduino. 
//AM2302 dht(AM2302_PIN);
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
int t = 0;  // TEMPERATURE VAR
int h = 0;  // HUMIDITY VAR
String data;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  lcd.begin(16, 2);
  print_to_lcd("initializing...");
  //lcd.print("initializing...");
  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }

  Serial.println("AM2302 Humidity - Temperature Sensor started...");
  //Serial.println("RH\t\tTemp (C)\tTemp (F)\tHeat Index (C)\t Heat Index (F)");

  dht.setup(7); // data pin 2
  //dht.begin();
  
  // print your local IP address:
  printIPAddress();
  data = "";
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());

  h = dht.getHumidity();
  t = dht.getTemperature();
  data = String("temp1=") + t + "&hum1=" + h;
  String to_print = String(" T = ") + t + ", H = " + h;

  Serial.println("sending data to server...");
  if (client.connect("eliranc12.atwebpages.com",80)) { // REPLACE WITH YOUR SERVER ADDRESS
    print_to_lcd_2_lines("Sending data" ,"to server...");
    Serial.println("connected to server");
    Serial.println("sending to server: " + data);
    client.println("POST /add.php HTTP/1.1"); 
    client.println("Host: eliranc12.atwebpages.com"); // SERVER ADDRESS HERE TOO
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: "); 
    client.println(data.length()); 
    client.println(); 
    client.print(data); 
    Serial.println("done sending data to server...");
  } else {
    Serial.println("Failed to send data to server...");  
  }
  
  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }


  //Serial.print(dht.humidity); Serial.print(" %\t\t");
  //Serial.print(dht.temperature_C); Serial.print(" *C\t");
  //Serial.print(dht.temperature_F); Serial.print(" *F\t");
  //Serial.print(dht.computeHeatIndex_C()); Serial.print(" *C\t");
  //Serial.print(dht.computeHeatIndex_F()); Serial.println(" *F");
  
  // Wait a few seconds between measurements. The AM2302 should not be read at a higher frequency of
  // about once every 2 seconds. So we add a 3 second delay to cover this.
  delay(1000);
  print_to_lcd(to_print);
  delay(300000);
  
  switch (Ethernet.maintain())
  {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");

      //print your local IP address:
      printIPAddress();
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");

      //print your local IP address:
      printIPAddress();
      break;

    default:
      //nothing happened
      printIPAddress();
      break;

  }
}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

void print_to_lcd(String to_print) {
  lcd.clear();
  delay(100);
  //lcd.setCursor(0, 1);
  lcd.print(to_print);
}

void print_to_lcd_2_lines(String to_print1, String to_print2) {
  lcd.clear();
  delay(100);
  lcd.print(to_print1);
  lcd.setCursor(0, 1);
  delay(100);
  lcd.print(to_print2);
}

