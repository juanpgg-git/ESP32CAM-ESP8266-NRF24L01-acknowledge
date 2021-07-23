#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <RHSoftwareSPI.h>
#include <SPI.h>
#include "esp_camera.h"
#include "camera_gpio.h"


//function declaration
void init_nrf24();
void setup_camera();

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

//an array of numbers to store pixel data
uint8_t pixel[1];

//to let the receiver know the communication started or finished
uint8_t start_com[] = "Start";
uint8_t finish_com[] = "Finish";

//to store 28 pixels and then send it
uint8_t pixels_chunk[28];
uint8_t *final_chunk_payload = NULL;

//how many chunks will the image be divided into
uint32_t chunks;
uint8_t final_pixel_chunk;

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void setup() 
{
  spi.setPins(12, 13, 14);
  Serial.begin(115200);
  init_nrf24();
  setup_camera(); 
}

void loop()
{
  //take an images
  camera_fb_t * image = esp_camera_fb_get();
  if(!image) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
   
  //convert the buffer length to an array of characters
  buffer_length = image->len;
  itoa(buffer_length, char_buffer_length, 10);
  
  //ESP8266 can only allocate memory for up to 52k bytes,
  //so to be sure, send only images < 45000 bytes
  if((image->len) < 20000){

    //not all images are divisible by 28, so we need to know how much pixels are left 
    final_pixel_chunk = image->len % 28;
    Serial.println(final_pixel_chunk);

    chunks = image->len - final_pixel_chunk;
    Serial.println(chunks);


    
   
    // 1. Send "Start"
    Serial.println("Start");
    if (!manager.sendtoWait(start_com, sizeof(start_com), SERVER_ADDRESS)){
      
      Serial.println("Start failed");
    }


    
    //2. send the buffer length as an arrray of characters
    if (!manager.sendtoWait((uint8_t*)char_buffer_length, sizeof(char_buffer_length), SERVER_ADDRESS)){
      
      Serial.print(char_buffer_length);
      Serial.println(" len failed");
    } 
    
    //3. send pixel data in an array of 28 numbers each time
    int i, j = 0;
    for(i = 0; i < chunks; i+=28){
       for(j = 0; j < 28; j++){
        pixels_chunk[j] = image->buf[j + i];
      }
      
      if (!manager.sendtoWait(pixels_chunk, 28, SERVER_ADDRESS)){        
        //Serial.println("pixel fail");
      } 
    }


    
    //4. send "Final chunk" 
    uint8_t final_chunk[] = "Final chunk";
    if (!manager.sendtoWait(final_chunk, sizeof(final_chunk), SERVER_ADDRESS)){        
        //Serial.println("final chunk fail");
    } 

    //5. send final chunk payload
    final_chunk_payload = (uint8_t*)malloc(final_pixel_chunk);
    for(int m = 0; m < final_pixel_chunk; m++){
      final_chunk_payload[m] = image->buf[chunks + m];
    }
    if (!manager.sendtoWait(final_chunk_payload, sizeof(final_chunk_payload), SERVER_ADDRESS)){        
        //Serial.println("pixel fail");
    } 
    free(final_chunk_payload);


    //6. send "Finish"
    if (!manager.sendtoWait(finish_com, sizeof(finish_com), SERVER_ADDRESS)){
      
      Serial.println("Finish failed");
    }
    Serial.println("Finish"); 


    //print image buffer
    for(int i = 0; i < image->len; i++){
    Serial.println(image->buf[i]);
    }
  }

  
  
  esp_camera_fb_return(image);
  
  delay(10000);
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
