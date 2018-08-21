# Project Overview
This application will be used to measure the distance from the measuring device to its closest obstacle. Once the distance has been measured the data will be sent to AWS using MQTT protocol for the next subsystem to process the data.

The application is built considering re-usability and tried to use standard libraries made available by Arduino IDE. 

Once the system boots it reads Wi-Fi credentials from a file “WiFi.json” and connects to the local area network. Then the application reads the private key and certificate files and using the keys connects to AWS as an IoT client.  The system sends data to AWS approximately once every minute. 

The following projects/libraries were used as reference or used to build the system.

[Example project](https://github.com/copercini/esp8266-aws_iot/)  to connecting to AWS by @copercini 


