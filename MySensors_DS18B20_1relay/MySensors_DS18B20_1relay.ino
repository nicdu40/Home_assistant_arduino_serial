//Ds18b20 + 1 relay




#define MY_GATEWAY_SERIAL
//#include <SPI.h>
#include <MySensors.h>  
#include <DallasTemperature.h>
#include <OneWire.h>

//temp
#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No
#define ONE_WIRE_BUS 7 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 10

//relay
#define RELAY_PIN 13  // Arduino Digital I/O pin number for relay
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay
#define CHILD_ID 3   // Id of the relay child
bool state = false;
bool initialValueSent = false;

unsigned long SLEEP_TIME = 1; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;


// Initialize 
MyMessage msg(0,V_TEMP);
MyMessage msgRelay(CHILD_ID, V_STATUS);

void before()
{
  sensors.begin(); // Startup up the OneWire library
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(RELAY_PIN, OUTPUT);
}

void setup()  
{ 
  sensors.setWaitForConversion(false);
}

void presentation() { 
  sendSketchInfo("Temperature Sensor", "1.1"); // Send the sketch version information to the gateway and Controller
  numSensors = sensors.getDeviceCount(); // Fetch the number of attached temperature sensors
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) { // Present all sensors to controller  
     present(i, S_TEMP);
  }
 
  //relay
  //sendSketchInfo("Relay", "1.0");  
 
 /* for (int sensor=1; sensor<=NUMBER_OF_RELAYS; sensor++) {
      // Register all sensors to gw (they will be created as child devices)
      //present(sensor, S_BINARY);
      present(3, S_BINARY);   
  }*/
  //present(2, S_BINARY);  
  present(CHILD_ID, S_BINARY);
 // send(msgRelay.set(0));
  
}

void loop()     
{     
  sensors.requestTemperatures();// Fetch temperatures from Dallas sensors
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution()); // query conversion time and sleep until conversion completed
  wait(conversionTime); // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) { // Read temperatures and send them to controller
    float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(i):sensors.getTempFByIndex(i)) * 10.)) / 10.; // Fetch and round temperature to one decimal
    if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) { // Only send data if temperature has changed and no error         
      send(msg.setSensor(i).set(temperature,1)); // Send in the new temperature
      lastTemperature[i]=temperature; // Save new temperatures for next compare
      //Serial.println("Temp = " + (String)temperature);
      Serial.println ("{ 'temp1': {'garage': ' + temperature + '}}") ;
    }
   // Serial.println("Temp = " + (String)temperature);
  }
 // send(msgRelay.set(state?RELAY_ON:RELAY_OFF), true);
 //send(msgRelay.set(state?RELAY_ON:RELAY_OFF));
 //gw.process();

  wait(SLEEP_TIME);
  //Serial.println("0;3;1;0;0;1");
  //request(3, V_STATUS);
}

void receive(const MyMessage &message)
{  
    if (message.type == V_STATUS) { // We only expect one type of message from controller. But we better check anyway.
    state = (bool)message.getInt(); // Change relay state

  //Serial.println("0;12;0;0;0;" + state );
  //Serial.println (state);    
    digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
    send(msgRelay.set(state?RELAY_ON:RELAY_OFF));
  }
}




