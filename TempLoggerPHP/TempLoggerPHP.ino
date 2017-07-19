#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include "DHT.h"


#define LCD_BUTTON 17
#define SENSE_INTERVAL 600000 //10 min
//#define SENSE_INTERVAL 30000 //30 seconds DBG...
#define LCD_DELAY 1000


// degree centigrade
byte newChar1[8] = {
  B01000,
  B10100,
  B01000,
  B00011,
  B00100,
  B00100,
  B00011,
  B00000
};

 /* The LiquidCrystal circuit:
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
 initialize the library with the numbers of the interface pins
 */
LiquidCrystal lcd(2, 3, 4, 8, 5, 6);

// initializing DHT22 sensor
DHT dht;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// globals
int t = 0;  // TEMPERATURE VAR
int h = 0;  // HUMIDITY VAR
String data;
int prev_button_value = 0;
unsigned long interval = 1000;
unsigned long previousMillis;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // init Display
  lcd.begin(16, 2);
  lcd.createChar(0, newChar1);
  print_to_lcd(String("        ...") + char(172) + char(167) + char(186) + char(160) + char(174));

 //init push button
  pinMode(LCD_BUTTON,INPUT); // set analog 0 pin to input
  previousMillis = millis();

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }

  Serial.println("DHT22 Humidity - Temperature Sensor started...");
  //Serial.println("RH\t\tTemp (C)\tTemp (F)\tHeat Index (C)\t Heat Index (F)");
  dht.setup(7); // data pin 2
  
  // print your local IP address to serial port:
  printIPAddress();
  data = "";
  delay(LCD_DELAY);
  //turn_off_display();
  sense();
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > SENSE_INTERVAL) {
    previousMillis = currentMillis; 
    sense(); 
  }
  
  int BUTTON = digitalRead(LCD_BUTTON);  // read analog pin 0 as digital and set it to a variable
  if (BUTTON == HIGH){
    if (!prev_button_value) {
      prev_button_value = 1;
      sense();
      previousMillis = currentMillis;
      //turn_on_display();      
    }
  } else if (BUTTON == LOW) {
    prev_button_value = 0;
  }
}


void sense() {
  print_to_lcd_2_lines(String("   ") + char(164) + char(184) + char(165) + char(168) + char(184) + char(180) + char(174) + char(168) + " " + char(163) + char(163) + char(165) + char(174) ,String("        ")  + char(175) + char(186) + char(174) + char(164) + " " + char(160) + char(176) + char(160));
  delay(dht.getMinimumSamplingPeriod());
  h = dht.getHumidity();
  t = dht.getTemperature();
  data = String("temp1=") + t + "&hum1=" + h;
  String line1_to_print = String("    ") + t;
  String line1_to_print_after_sign = String(" ") + char(164) + char(184) + char(165) + char(168) + char(184) + char(180) + char(174) + char(168);
  String line2_to_print = String ("        ") + h + "%" + " " + char(186) + char(165) + char(167) + char(172);
  
  delay (LCD_DELAY);
  if (client.connect("eliranc12.atwebpages.com",80)) { // REPLACE WITH YOUR SERVER ADDRESS
    print_to_lcd_2_lines(String("      ") + char(164) + char (163) + char (169) + char (163) + char (174) + " " + char(167) + char (172) + char(165) + char(185) ,String("         ...") + char(186) + char(184) + char(185) + char(172));
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

  delay(LCD_DELAY*2);
  print_to_lcd_2_lines_with_sign(line1_to_print,line1_to_print_after_sign,line2_to_print);
 
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

void print_to_lcd_2_lines_with_sign(String to_print1, String to_print1_after_sign, String to_print2) {
  lcd.clear();
  delay(100);
  lcd.print(to_print1);
  lcd.write(byte(0));
  int c = to_print1_after_sign.length();
  for(int x = 0; x < c; x++)
  {
     char to_write = to_print1_after_sign.charAt(x);
     lcd.write(to_write);
  }

  
  //lcd.print(to_print1_after_sign);
  lcd.setCursor(0, 1);
  delay(100);
  lcd.print(to_print2);
}

void turn_on_display(){
  lcd.display();  
}

void turn_off_display(){
  lcd.noDisplay();  
}

