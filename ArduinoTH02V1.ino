//---------------------------------------------------------------------------------
// Copyright ® December 2017, devMobile Software
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Thanks to the creators and maintainers of the libraries used by this project
//   https://github.com/maniacbug/RF24
//   https://github.com/Seeed-Studio/Grove_Temper_Humidity_TH02
//
#include <RF24.h>
#include <TH02_dev.h>

// nRF24L01 ISM wireless module setup for Arduino Uno R3/Seeeduino V4.2 using http://embeddedcoolness.com/shop/rfx-shield/
// RF24 radio( ChipeEnable , ChipSelect )
RF24 radio(3,7);
const byte FieldGatewayChannel = 15 ;
const byte FieldGatewayAddress[] = "Base1";
const rf24_datarate_e RadioDataRate = RF24_250KBPS; 
const rf24_pa_dbm_e RadioPALevel = RF24_PA_MAX;

// Payload configuration
const int PayloadSizeMaximum = 32 ;
char payload[PayloadSizeMaximum] = "";
const byte DeviceIdPlusCsvSensorReadings = 1 ;
const byte SensorReadingSeperator = ',' ;

const int LoopSleepDelaySeconds = 30 ;

// Manual serial number configuration
uint8_t DeviceId[] = {0,0,0,2};


void setup() 
{
  Serial.begin(9600);
  Serial.println("Setup called");

  Serial.print("DeviceID:");
  for (int i=0; i<sizeof(DeviceId); i++)
  {
    // Add a leading zero if required
    if (DeviceId[i] < 16)
    {
      Serial.print("0");
    }    
    Serial.print(DeviceId[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); 

  // Configure the Seeedstudio TH02 temperature & humidity sensor
  Serial.println("TH02 setup");
  TH02.begin();
  delay(100);

  // Configure the nRF24 module
  Serial.println("nRF24 setup");
  radio.begin();
  radio.setChannel(FieldGatewayChannel);
  radio.openWritingPipe(FieldGatewayAddress);
  radio.setDataRate(RadioDataRate) ;
  radio.setPALevel(RadioPALevel);
  radio.enableDynamicPayloads();

  Serial.println("Setup done");
}


void loop() 
{
  int payloadLength = 0 ;  
  float temperature ;
  float humidity ;
  float batteryVoltage ;
  
  Serial.println("Loop called");
  memset(payload, 0, sizeof(payload));

  // prepare the payload header with PayloadMessageType (top nibble) and DeviceID length (bottom nibble)
  payload[0] = (DeviceIdPlusCsvSensorReadings << 4) | sizeof(DeviceId) ; 
  payloadLength += 1;

  // Copy the deviceID into payload
  memcpy(&payload[payloadLength], DeviceId, sizeof(DeviceId));
  payloadLength += sizeof(DeviceId) ;

  // Read the temperature and humidity values then display nicely
  temperature = TH02.ReadTemperature();
  humidity = TH02.ReadHumidity();
  
  Serial.print("T:");
  Serial.print( temperature, 1 ) ;
  Serial.print( "C" ) ;

  Serial.print(" H:");
  Serial.print( humidity, 0 ) ;
  Serial.println( "%" ) ;

  // Copy the temperature into the payload, with one decomal place and upto 3 leading spaces
  payload[ payloadLength] = 't';
  payloadLength += 1 ;
  dtostrf(temperature, 6, 1, &payload[payloadLength]);  
  payloadLength += 6;
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += sizeof(SensorReadingSeperator) ;

  // Copy the humidity into the payload, with no decimal place and upto 3 leading spaces
  payload[ payloadLength] = 'h';
  payloadLength += 1 ;
  dtostrf(humidity, 4, 0, &payload[payloadLength]);  
  payloadLength += 4;
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += sizeof(SensorReadingSeperator) ;

  // Send the payload to base station
  Serial.print( "nRF24 Payload length:");
  Serial.println( payloadLength );

  boolean result = radio.write(payload, payloadLength);
  if (result)
    Serial.println("Write Ok...");
  else
    Serial.println("Write failed.");

  Serial.println("Loop done");

  delay(LoopSleepDelaySeconds * 1000l);
}

