/******************************************************************************

BLE-Weather-Station-Cloud-Gateway
Leonardo Bispo
March - 2019
https://github.com/ldab/BLE-Weather-Station-Cloud-Gateway

Distributed as-is; no warranty is given.

******************************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include "Arduino.h"
#include <Ticker.h>
#include <Adafruit_SHT31.h>

// BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Blynk
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// OTA
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// E-paper display
#include <SPI.h>
#include <MiniGrafx.h>
#include "DisplayDriver.h"
#include "EPD_WaveShare_42.h"  // Hardware-specific library
#include "WeatherIcons.h"

#define CS    2
#define RST   15
#define DC    5
#define BUSY  4
#define USR_B 12              // User button

#define SCREEN_WIDTH   400
#define SCREEN_HEIGHT  300
#define BITS_PER_PIXEL 1

uint16_t palette[] = {0, 1};

EPD_WaveShare42 epd(CS, RST, DC, BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);
uint32_t startMillis;

// You should get Auth Token in the Blynk App. Go to the Project Settings (nut icon)
char auth[] = "YourAuthToken";

// Your WiFi credentials.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

// The remote service we wish to connect to.
static BLEUUID serviceUUID("181A");
// The characteristic of the remote service we are interested in.
static BLEUUID tcharUUID("2A6E");
static BLEUUID hcharUUID("2A6F");
// This device name
const char charNAME[] = "SweetHome";

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan    = false;
static BLERemoteCharacteristic* tCharacteristic;
static BLERemoteCharacteristic* hCharacteristic;
static BLEAdvertisedDevice*     myDevice;

// Create functions prior to calling them as .cpp files are differnt from Arduino .ino
void setupBLE   ( void );
void readSensor ( void );
void blinky     ( void );
bool connectToServer( void );

// Initialize the Temperature and Humidity Sensor SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Create timers using Ticker library in oder to avoid delay()
Ticker blinkIt;
Ticker readLocal;

float tLocal = NAN;
float hLocal = NAN;
float tBLE   = NAN;
float hBLE   = NAN;

static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData,
                            size_t length,
                            bool isNotify )
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

// Scan for BLE servers and find the first one that advertises the service we are looking for
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      BLEDevice::getScan()->stop();
      myDevice  = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan    = true;
    } // Found our server
  } // onResult
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient)
  {
    Serial.println("onConnect");
  }

  void onDisconnect(BLEClient* pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    tCharacteristic = pRemoteService->getCharacteristic(tcharUUID);
    if (tCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(tcharUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our tcharUUID characteristic");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    hCharacteristic = pRemoteService->getCharacteristic(hcharUUID);
    if (hCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(hcharUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our hcharUUID characteristic");

    // Read the value of the characteristic.
    if(tCharacteristic->canRead()) {
      std::string value = tCharacteristic->readValue();
      tBLE = atoi(value.c_str()) / 100.0f;

      Serial.print("The characteristic value was: ");
      Serial.println(tBLE);
    }

    if(hCharacteristic->canRead()) {
      std::string value = hCharacteristic->readValue();
      hBLE = atoi(value.c_str()) / 100.0f;

      Serial.print("The characteristic value was: ");
      Serial.println(hBLE);
    }

    if(tCharacteristic->canNotify())
      tCharacteristic->registerForNotify(notifyCallback);

    if(hCharacteristic->canNotify())
      hCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}

void setup()
{
  pinMode( LED_RED  , OUTPUT );
  pinMode( LED_GREEN, OUTPUT );
  pinMode( LED_BLUE , OUTPUT );
  digitalWrite( LED_RED  , HIGH );
  digitalWrite( LED_GREEN, HIGH );
  digitalWrite( LED_BLUE , HIGH );

  Serial.begin(115200);

  // Start display
  gfx.init();
  gfx.setRotation(0);
  gfx.setFastRefresh(false);

  if( !sht31.begin(0x44) ){
    Serial.println("Failed to find sensor, please check wiring and address");
  }
  
  BLEDevice::init(charNAME);

  // Retrieve a Scanner and set the callback.
  BLEScan* pBLEScan = BLEDevice::getScan();
  // Callback function to be called when a new Peripheral is found
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  // Set the Scan interval in ms
  pBLEScan->setInterval(1349);
  // Scan duration in ms
  pBLEScan->setWindow(449);
  // Active scan means a scan response is expected
  pBLEScan->setActiveScan(false);
  // Start scan for [in] seconds
  pBLEScan->start(5, false);

  // Start Timers, read sensor and blink Blue LED
  readLocal.attach( 10, readSensor ); // we will now read the sensor only when sending data
  blinkIt.attach( 1, blinky );

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  //ArduinoOTA.setPassword("casasujachaosujo");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  /*ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();*/

  Blynk.config(auth);

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
  Blynk.run();
  //ArduinoOTA.handle();
  
  // If "doConnect" is true BLE Server has been found, Now we connect to it.
  if (doConnect == true) 
  {
    if ( connectToServer() ) // Connect to the BLE Server found 
    {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }

    doConnect = false;
  }

  // If connected to a peer BLE Server:
  if (connected)
  {

  }
  else if(doScan)
  {
    BLEDevice::getScan()->start(0);  // Start scan after disconnect
  }

} // end of loop

void readSensor( void )
{
  tLocal = sht31.readTemperature();
  hLocal = sht31.readHumidity();

  if (! isnan(tLocal)) {  // check if 'is not a number'
    Serial.print("Temp ºC = "); Serial.println(tLocal);
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(hLocal)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(hLocal);
  } else { 
    Serial.println("Failed to read humidity");
  }
  Serial.println();
  
}

// Toggle LED_Blue
void blinky( void )
{
  bool toggle = digitalRead( LED_BLUE );

  digitalWrite( LED_BLUE, !toggle );
}
