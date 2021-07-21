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

//To compare with the received message, and know if the
//communication is starting or finishing
uint8_t start_com[] = "Start";
uint8_t finish_com[] = "Finish";

/*The buffer length is being send as an array of characters,
* so first you need to store it in a char array, and then convert 
* it into an int for further use
*/
char char_buffer_length[5];
int buffer_length = 0;

//to know which pixel data are we storing
uint16_t counter = 0;

//pointer to the image buffer
uint8_t * image;

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

      //if the received message is equal to "Finish" then the communication finished
      if(memcmp(buf, finish_com, 6) == 0){
        
        Serial.println((char*)buf);

        //testing with sending 10 pixels
        for(int i = 0; i < 10; i++){
         
          Serial.println(image[i]);
        }
        
        //free the allocated memory for the image buffer
        free(image);

        //reset the counter for the next communication
        counter = 0; 
      }
      
      if(counter > 1){

        //counter 1 is buffer length, counter 2 and so on are pixel data
        image[0 + (counter - 2)] = buf[0];
        /*
        Serial.print("got request from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println(buf[0]);
        */
        counter++;
      }
      
      //this would be buffer length
      if(counter == 1){

       //the buffer length comes as an array of characters,
       //so convert it into an int
       memcpy(char_buffer_length, buf, 5);
       buffer_length = atoi(char_buffer_length);       

       //allocate a very big memory for the image buffer
       image = (uint8_t*)calloc(buffer_length, 1);
       
       Serial.println(char_buffer_length);
       
       counter++;
      }

      //if the received message is equal to "Start" then the communication started
      if(memcmp(buf, start_com, 5) == 0){
        
       Serial.println((char*)buf);

       counter++;
      }
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
