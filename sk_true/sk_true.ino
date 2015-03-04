#include <SPI.h>
#include <WiFi.h>

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// CONFIG
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------

// 0: on production nothing out from serial port
// 1: on dev the program will waiting for serial connection 
int CONFIG_DEBUG_MODE = 1;
// force send command button
int CONFIG_FORCE_COMMAND_PIN = 3;

//  your network SSID (name)
char CONFIG_WIFI_SSID[] = "mn-wlan";
// your network password
char CONFIG_WIFI_PASS[] = "ndit123!@#simple";
// 0: WPA/WPA2
// 1: WEP
int CONFIG_WIFI_MODE = 0;
// your network key Index number (needed only for WEP)
int CONFIG_WIFI_KEYINDEX = 0;


// Point to Cm Proxy Service IP
IPAddress CONFIG_CMPROXY_IP(192,168,1,3);
//IPAddress CONFIG_CMPROXY_IP(127,0,0,1);
// Point to Cm Proxy Service port
// in some case firewall will block the connection 
// then disable firewall or allow connection on specific port
const int CONFIG_CMPROXY_PORT = 9092;
const char CONFIG_CMPROXY_COMMAND[] = "111";
 
// infrared
const int CONFIG_INFRARED_INPUT_PIN = 5;
const int CONFIG_INFRARED_EN_PIN = 4;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
 // Initialize the Wifi client library
  WiFiClient client;
  
// build-in LED on wifi shield
int L9 = 9;
int status = WL_IDLE_STATUS;

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L; // delay between updates, in milliseconds

unsigned long infraredInterval = 0L; // delay between updates, in milliseconds
unsigned long countObstacle = 0; 

// ----------------------------------------------------
void setup() {

  if(CONFIG_DEBUG_MODE == 1){
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

  }
  pinMode(L9, OUTPUT);
  // force send command
  pinMode(CONFIG_FORCE_COMMAND_PIN, INPUT);
  // infrared
  pinMode(CONFIG_INFRARED_INPUT_PIN,INPUT);
  pinMode(CONFIG_INFRARED_EN_PIN,OUTPUT);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    if(CONFIG_DEBUG_MODE == 1) Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" ){
    if(CONFIG_DEBUG_MODE == 1) Serial.println("Please upgrade the firmware");
  }

}
// ----------------------------------------------------
void loop() {

  int isTryConnectWifi = 0;

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    
    digitalWrite(L9, HIGH);
    isTryConnectWifi = 1;

    if(CONFIG_DEBUG_MODE == 1)Serial.print("Attempting to connect to SSID: ");
    if(CONFIG_DEBUG_MODE == 1) Serial.println(CONFIG_WIFI_SSID);


    if(CONFIG_WIFI_MODE == 0){
      // Connect to WPA/WPA2 network.
      status = WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASS);
    }
    else{
      // Connect to WEP network.
      status = WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_KEYINDEX, CONFIG_WIFI_PASS);
    }

    // wait 10 seconds for connection:
    delay(10000);

  }
  digitalWrite(L9, LOW);
  
  // you're connected now, so print out the status:
  if(isTryConnectWifi == 1)  printWifiStatus();
  
  
  if(CONFIG_DEBUG_MODE == 1){
    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }
  
  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  
    
    int switchState = digitalRead(CONFIG_FORCE_COMMAND_PIN);
    if (switchState == LOW) {
        
        // -------------------------------------------
        // check infrared
        int infraredState = digitalRead(CONFIG_INFRARED_INPUT_PIN); 
        //if(CONFIG_DEBUG_MODE == 1) Serial.println(infraredState);
        if(infraredState==LOW)
        {
          if (millis() - lastConnectionTime > infraredInterval) {  
          
             // have the obstacle
             countObstacle++;
             if(countObstacle > 20){
                   
                if(CONFIG_DEBUG_MODE == 1) Serial.println("send from infrared...");
                sendCommand();
                //countObstacle = 0;
             }
          }
        }else{
          // nothing in front of sensor
          countObstacle = 0;
        }
        // -------------------------------------------
    } 
    else {
      if (millis() - lastConnectionTime > postingInterval) {  
        // force send command
        sendCommand();
      }
    }
  
  
  unsigned long statusL9 = millis();
  if(statusL9 % 3000 > 1500) digitalWrite(L9, LOW); else digitalWrite(L9, HIGH);
}

// ----------------------------------------------------
// this method makes a HTTP connection to the server:
void sendCommand() {
  
 

   digitalWrite(L9, HIGH);
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  //client.stop();
  //client.setTimeout(500);
  // if there's a successful connection:
  if (client.connect(CONFIG_CMPROXY_IP, CONFIG_CMPROXY_PORT)) {
    if(CONFIG_DEBUG_MODE == 1) Serial.println("connected...");
    
    //delay(200);
    // send the HTTP PUT request
    // CONFIG_CMPROXY_COMMAND
    client.print(CONFIG_CMPROXY_COMMAND);
    client.flush();
    
    // note the time that the connection was made:
    lastConnectionTime = millis();
    infraredInterval = 0;
    //delay(300);
    //client.stop();
    /*
    // waiting for reply
    int waitingTimeout = 0;
    while (client.available()) {
        Serial.write("Waiting reply from server");
        char c = client.read();
        Serial.write("Reply from server: " + c);
    }
    */
    infraredInterval = 1000L;
    if(CONFIG_DEBUG_MODE == 1){ 
      Serial.print("sent...");
      Serial.println(CONFIG_CMPROXY_COMMAND);
    }
    
    //client.stop();
  }
  else {
    // if you couldn't make a connection:
    if(CONFIG_DEBUG_MODE == 1)Serial.println("connection failed");
  }
  //client.flush();
  client.stop();
  countObstacle = 0;
  digitalWrite(L9, LOW);
}
// ----------------------------------------------------
void printWifiStatus() {
  if(CONFIG_DEBUG_MODE == 0) return;
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
// ----------------------------------------------------


