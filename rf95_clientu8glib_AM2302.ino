#include <DHT22.h>

// Henry's Bench Hello World - for use with Monochrome OLEDs
#include <SPI.h>
#include <RH_RF95.h>
#include "U8glib.h"
#include <stdio.h>
#include "floatToString.h"

#define DHT22_PIN 30     // what pin we're connected to
#define fan 3
 
int maxHum = 60;
int maxTemp = 30;
float temp=0.0;
float hum=0.0;
 
// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);
 

// Singleton instance of the radio driver
RH_RF95 rf95;
int i=0;
//**************************************************
// Change this constructor to match your display!!!
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 

//**************************************************
float data;
String dataString="";
char databuf[20];
uint8_t dataoutgoing[10];
void setup() {  
    pinMode(fan, OUTPUT);
    Serial.println("DHT22 Library Demo");
    Serial.begin(9600); 
    u8g.setFont(u8g_font_helvB14);
    u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    while (!Serial) ; // Wait for serial port to be available
    if (!rf95.init())
      Serial.println("init failed");
}

void loop() {  
  DHT22_ERROR_t errorCode;
  
  // The sensor can only be read from every 1-2s, and requires a minimum
  // 2s warm-up after power-on.
  delay(1200);
  Serial.print("****");
  Serial.print("Requesting data...");
  errorCode = myDHT22.readData();
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      Serial.print("Got Data ");
      temp=myDHT22.getTemperatureC();
      dataString+=dtostrf(temp,5,2,databuf);
      dataString+="/";
      Serial.print(temp);
      Serial.print("C ");
      hum=myDHT22.getHumidity();
      dataString+=dtostrf(hum,5,2,databuf);
      Serial.print(hum);
      Serial.println("%");
      // Alternately, with integer formatting which is clumsier but more compact to store and
    // can be compared reliably for equality:
    //    
  /*    char buf[128];
      sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
                   myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
                   myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
      Serial.println(buf);*/
            Serial.println("****");
      break;
    case DHT_ERROR_CHECKSUM:
      Serial.print("DHT22: check sum error ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("C ");
      Serial.print(myDHT22.getHumidity());
      Serial.println("%");
      break;
    case DHT_BUS_HUNG:
      Serial.println("DHT22: BUS Hung ");
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println("DHT22: Not Present ");
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println("DHT22: ACK time out ");
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println("DHT22: Sync Timeout ");
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println("DHT22: Data Timeout ");
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println("DHT22: Polled to quick ");
      break;
  }
  u8g.firstPage();

  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server

  Serial.println(dataString);
  dataString.toCharArray(databuf,20);
  rf95.send((uint8_t *)databuf,sizeof(databuf));
  memset(dataString,'\0',strlen(dataString));
  i=1; 
  
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
       // Should be a reply message for us now   
       if (rf95.recv(buf, &len))
       {
          Serial.print("got reply: ");
          Serial.println((char*)buf);
          //      Serial.print("RSSI: ");
          //      Serial.println(rf95.lastRssi(), DEC);    
       }
       else
       {
          Serial.println("recv failed");
          i=2;
       }
  }
  else
  {
       Serial.println("No reply, is rf95_server running?");
       i=3;
  }
  do {  
       if(i==0){
         u8g.drawStr( 0, 30, "Falla...");
       }
       if(i==1){
          digitalWrite(4, HIGH);
          u8g.drawStr( 0, 30, "Enviado");
          delay(500);
          digitalWrite(4, LOW);
       }
       if(i==2){
          u8g.drawStr( 0, 30, "Falla rcv");
       }
       if(i==3){
          digitalWrite(4, HIGH);
            delay(200); 
          u8g.drawStr( 0, 30, "No server?...");
          digitalWrite(4, LOW);
          delay(200); 
          digitalWrite(4, HIGH);
          delay(200); 
          digitalWrite(4, LOW);
       }
  } while( u8g.nextPage() );
}
