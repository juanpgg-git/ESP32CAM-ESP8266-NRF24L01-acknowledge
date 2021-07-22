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
      /*
      //dummy code to test array of 28 numbers
        Serial.print("got request from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println(buf[0]);
        Serial.println(buf[1]);
        Serial.println(buf[2]);
        Serial.println(buf[3]);
        Serial.println(buf[4]);
        Serial.println(buf[5]);
        Serial.println(buf[6]);
        Serial.println(buf[7]);
        Serial.println(buf[8]);
        Serial.println(buf[9]);
        Serial.println(buf[10]);
        Serial.println(buf[11]);
        Serial.println(buf[12]);
        Serial.println(buf[13]);
        Serial.println(buf[14]);
        Serial.println(buf[15]);
        Serial.println(buf[16]);
        Serial.println(buf[17]);
        Serial.println(buf[18]);
        Serial.println(buf[19]);
        Serial.println(buf[20]);
        Serial.println(buf[21]);
        Serial.println(buf[22]);
        Serial.println(buf[23]);
        Serial.println(buf[24]);
        Serial.println(buf[25]);
        Serial.println(buf[26]);
        Serial.println(buf[27]);
        */
      /*
      //if the received message is equal to "Finish" then the communication finished
      if(memcmp(buf, finish_com, 6) == 0){
        
        Serial.println((char*)buf);

        //testing with sending 10 pixels
        for(int i = 0; i < 100; i++){
         
          Serial.println(image[i]);
        }
        
        //free the allocated memory for the image buffer
        free(image);

        //reset the counter for the next communication
        counter = 0; 
      }
      
      else if(counter > 1){

        //counter 1 is buffer length, counter 2 and so on are pixel data
        image[0 + (counter - 2)] = buf[0];
    
        counter++;
      }
      
      //this would be buffer length
      else if(counter == 1){

       //the buffer length comes as an array of characters,
       //so convert it into an int
       memcpy(char_buffer_length, buf, 5);
       buffer_length = atoi(char_buffer_length);       

       //allocate a very big memory for the image buffer
       image = (uint8_t*)calloc(buffer_length, 1);
       
       Serial.println(char_buffer_length);
       
       counter = 2;
      }

      //if the received message is equal to "Start" then the communication started
      else if(memcmp(buf, start_com, 5) == 0){
        
       Serial.println((char*)buf);

       counter = 1;
      }
      */
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
