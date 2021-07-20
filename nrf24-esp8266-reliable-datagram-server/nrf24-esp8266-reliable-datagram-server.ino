#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//function declaration
void init_nrf24();

// Singleton instance of the radio driver
RH_NRF24 driver(2, 4);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

void setup() 
{
  Serial.begin(115200);
  init_nrf24();
}

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available()){
    
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    
    if (manager.recvfromAck(buf, &len, &from)){
      
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
  }
}

/*
 * Initialize the NRF24Ñ01 with the RHReliableDatagram
 * This class works with acknowledge bit
 */
void init_nrf24(){

  if (!manager.init()){
    
    Serial.println("init failed");
  }
  else{

    Serial.println("init succeed");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
}
