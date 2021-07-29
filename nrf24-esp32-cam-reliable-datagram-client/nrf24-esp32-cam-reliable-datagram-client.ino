#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <RHSoftwareSPI.h>

#include "esp_camera.h"
#include "camera_gpio.h"

//function declaration
void init_nrf24();
void setup_camera();
void take_image();
void send_pixel_payload();
void send_final_pixel_payload();
void send_start();
void send_len();
void send_last_chunk();
void send_finish();


#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//create a SPI with software
RHSoftwareSPI spi;

// Singleton instance of the radio driver
RH_NRF24 driver(15, 2, spi);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

//RadioHead library accepts an array of characters, so convert the
//buffer length to an array of characters
int buffer_length;
char char_buffer_length[5];

//to let the receiver know the communication started or finished
uint8_t start[] = "Start";
uint8_t finish[] = "Finish";
uint8_t last_chunk[] = "Last chunk";

//to store chunk_interator and 27 pixels
uint8_t pixel_payload[28];

//to iterate the image buffer and store it into the pixel_payload
int chunk_iterator = 0;
int x = 0;
int i = 0;

//how many chunks will the image be divided into and how many pixels are left in the final chunk
uint16_t chunks;
uint8_t final_pixel_chunk;

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

//struct to store image data
camera_fb_t * image;

void setup() 
{
  spi.setPins(12, 13, 14);
  Serial.begin(115200);
  init_nrf24();
  setup_camera(); 
}

void loop()
{
  //take and image an store it in camera_fb_t * image
  take_image();
  
  //convert the buffer length to an array of characters
  buffer_length = image->len;
  itoa(buffer_length, char_buffer_length, 10);
  
  //ESP8266 can only allocate memory for up to 52k bytes,
  //so to be sure, send only images < 45000 bytes
  if(buffer_length < 20000){

    Serial.print("buffer length: ");
    Serial.println(buffer_length);
     
    //not all images are divisible by 28, so we need to know how much pixels are left in the final chunk
    final_pixel_chunk = buffer_length % 27;

    //how many chunks are we going to send
    chunks = buffer_length / 27;

    // 1. Send "Start"
    send_start();
    
    //2. send the buffer length as an arrray of characters
    send_len();

    //3. send pixel data in an array of 28 numbers. First number is the 
    //chunk_iterator and the rest is the pixel data
    buffer_length = 999;
    final_pixel_chunk = buffer_length % 27;
    chunks = buffer_length / 27;
    for(i = 1; i < 28; i++){
      pixel_payload[i] = image->buf[(i - 1) + x];
      Serial.println(pixel_payload[i]);
      //it means we have 27 pixels ready for a given chunk
      if( i == 27){

        //store the chunk order we are going to send
        pixel_payload[0] = chunk_iterator;
        //Serial.print(pixel_payload[0]);
        //Serial.print(" : ");
        //Serial.println(chunk_iterator);
        chunk_iterator++;
        
        //send pixel payload
        send_pixel_payload();
       
        //reset the counter to start again
        i = 0;
        x += 27;
        
        //this means there is no pixel left to be send, so we must exit the for loop
        if( chunk_iterator == chunks){
          
          //reset values to start over again
          chunk_iterator = 0;
          x = 0;
          i = 28;
        }
      }
    }

    if(final_pixel_chunk != 0){
      
      //4. send "Last chunk"
      send_last_chunk();

      //5.Send the final pixel payload
      send_final_pixel_payload();
    }
 
    //6. send "Finish" so the receiver can now the communication is over
    send_finish();
    
    //print image buffer
    for(int i = 0; i < buffer_length; i++){
    Serial.println(image->buf[i]);
    }
  }
  
  esp_camera_fb_return(image);
  
  delay(5000);
}

void send_final_pixel_payload(){
  
  int m = 0;
  //store the remainder pixels 
  for(m = 0; m < final_pixel_chunk; m++){
    pixel_payload[m] = image->buf[m + (27 * chunks)];
    Serial.println(pixel_payload[m]);
  }
  
  if (!manager.sendtoWait(pixel_payload, final_pixel_chunk, SERVER_ADDRESS)){        
            //Serial.println("pixel fail");
  }
}

void send_pixel_payload(){

   if (!manager.sendtoWait(pixel_payload, 28, SERVER_ADDRESS)){        
          //Serial.println("pixel fail");
   } 
}

void send_last_chunk(){

  Serial.println("Last chunk");
  
  if (!manager.sendtoWait(last_chunk, sizeof(last_chunk), SERVER_ADDRESS)){  
      Serial.println("Last chunk failed");
  }
}

void send_finish(){

  Serial.println("Finish");
  
  if (!manager.sendtoWait(finish, sizeof(finish), SERVER_ADDRESS)){  
      Serial.println("Finish failed");
  }
}

void send_start(){

  Serial.println("Start");
  
  if (!manager.sendtoWait(start, sizeof(start), SERVER_ADDRESS)){  
      Serial.println("Start failed");
  }
}

void send_len(){

  Serial.print(buffer_length);
  
  if (!manager.sendtoWait((uint8_t*)char_buffer_length, sizeof(char_buffer_length), SERVER_ADDRESS)){
    
    Serial.println(" len failed");
  } 
}

void init_nrf24(){
  
  if (!manager.init()){

    Serial.println("nrf24 init failed");
  }
  else{

    Serial.println("nrf24 init succeed");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if(!driver.setChannel(125)){
    Serial.println("change chanel failed");
  }
  else{
    Serial.println("change channel succeed");
  }
}

void setup_camera(){

  //camera configuration variable
  camera_config_t config; 
  
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  else if(err == ESP_OK){
    
    Serial.println("Camera init succeed");
  }
}

void take_image(){

  //take an images
  image = esp_camera_fb_get();
  if(!image) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
}
