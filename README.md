# ESP32CAM-ESP8266-NRF24L01Z-acknowledge
The idea of this project is to send data images from an ESP32-CAM to an ESP8266 using the NRF24L01 module. After the ESP8266 receives the image it will send it to a web server.

The NRF24L01 supports a distance from 200m up to 800m. Which allows you to have the transmitter and receiver station very far away. The communication only takes a few seconds if you set up the antennas pointing correctly to each other.

Due to sending images, the data must come to the receiver, so it can be able to reconstruct the image again. If any data is lost, the image will be blurry or damaged. To avoid this, a communication using acknowledge bits must be used.

The Radiohead library plays a key role in this part. With a very easy-to-use API, you can implement communication using acknowledge bits: http://www.airspayce.com/mikem/arduino/RadioHead/index.html

To upload images to a web server, you can check: https://randomnerdtutorials.com/esp32-cam-http-post-php-arduino/

Note: both your transmitter and receiver microcontrollers must be working at the same frequency, if not, the communication will last 10+ minutes.
