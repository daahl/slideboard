#include <HX711.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

float calibration_factor = -650.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  32
#define CLK  33

#define DEFAULT_BW 70 // body weight

bool printWelcome = true;

HX711 scale;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  SerialBT.begin("SlideboardV1"); // Bluetooth device name
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  // print the pre-set body weight on startup
  if(printWelcome){
    while(!SerialBT.hasClient()){
      delay(2000);
    }
    SerialBT.print("Default BW: ");
    SerialBT.println(DEFAULT_BW);
    printWelcome = false;
  }
  
  // main API
  if (SerialBT.available()){
    String incMsg = SerialBT.readString();
    String ctrlCharBW = "!!";
    String ctrlCharBATT = "##";
    String ctrlCharCFP = "++";
    String ctrlCharCFM = "--";
    if(incMsg[0] == ctrlCharBW[0]){
      SerialBT.print("Setting new BW: ");
      SerialBT.println(&incMsg[0] + 1);
    }
    else if(incMsg[0] == ctrlCharBATT[0]){
      SerialBT.print("Current battery level: ");
      SerialBT.println(" TO BE IMPLEMENTED");
    }
    else if(incMsg[0] == ctrlCharCFP[0]){
      calibration_factor += 100;
      scale.set_scale(calibration_factor);
      SerialBT.println(calibration_factor);
    }
    else if(incMsg[0] == ctrlCharCFM[0]){
      calibration_factor -= 100;
      scale.set_scale(calibration_factor);
      SerialBT.println(calibration_factor);
    }
    else{
      SerialBT.print("Write '!BW' to set new body weight, or '#' to see battery level.");
    }
    delay(1000);
  }
  SerialBT.print(scale.get_units(), 1);
  SerialBT.print(" kg");
  SerialBT.flush();
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
}
