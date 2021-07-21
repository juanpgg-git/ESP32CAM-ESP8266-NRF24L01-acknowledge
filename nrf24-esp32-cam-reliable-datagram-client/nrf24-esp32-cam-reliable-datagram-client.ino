#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <RHSoftwareSPI.h>
#include <SPI.h>

//function declaration
void init_nrf24();

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//create a SPI with software
RHSoftwareSPI spi;

// Singleton instance of the radio driver
RH_NRF24 driver(15, 2, spi);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  spi.setPins(12, 13, 14);
  Serial.begin(9600);
  init_nrf24();
  
}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
  Serial.println("Sending to nrf24_reliable_datagram_server");
    
  // Send a message to manager_server
  if (!manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))//returns true if success
  {
    Serial.println("sendtoWait failed");
  }
    
  delay(500);
}

void init_nrf24(){
  
  if (!manager.init()){

    Serial.println("init failed");
  }
  else{

    Serial.println("init succeed");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
}
