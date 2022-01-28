#include <Arduino.h>
#include "HX711.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

float calibration_factor = -2750.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  32
#define CLK  33

#define DEFAULT_BW 70 // body weight
#define ADC_Pin 34 // battery voltage divider

bool printWelcome = true; // boot message
bool calibrationMode = false;

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
    String ctrlCharBW = "!!";   // set body weight
    String ctrlCharBATT = "##"; // battery level
    String ctrlCharCFP = "++";  // increase calibration
    String ctrlCharCFM = "--";  // decreae calibration
    String ctrlCharCM = "cc";   // calibration mode
    if(incMsg[0] == ctrlCharBW[0]){
      SerialBT.print("Setting new BW: ");
      SerialBT.println(&incMsg[0] + 1);
    }
    else if(incMsg[0] == ctrlCharBATT[0]){
      // display battery levels
      int val = 0;
      // do some ignal smoothing
      for(int i = 0; i < 10; i++){
          val += analogRead(ADC_Pin);
        delay(5);
      }
      val = val / 10;
      val = map(val, 650, 900, 0, 100);
      SerialBT.print("Current battery level: ");
      SerialBT.print(val);
      SerialBT.println("%");
    }
    // change calibration factor
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
    // activate calibration mode
    else if(incMsg[0] == ctrlCharCM[0]){
      if(calibrationMode){
        calibrationMode = false;
      }else{
        calibrationMode = true;
      }
      SerialBT.print("Calibration mode set to: ");
      SerialBT.println(calibrationMode);
      scale.tare();
    }
    else{
      // shot menu options
      SerialBT.print("'!BW' to set new body weight");
      SerialBT.print("'#' to see battery level.");
      SerialBT.print("'#' to toggle calibration mode.");
      SerialBT.print("'+/-' to change calibration level");
    }
    delay(1000);
  }
  // do some smoothing
  float weight = 0.0;
  for(int i = 0; i < 10; i++){
    weight += scale.get_units();
    delay(1);
  }
  weight = weight / -10;

  // loop prints
  if(calibrationMode){
    SerialBT.print(weight, 1);
    SerialBT.println(" kg");
  }
  else{
    // dont bother printing when no weight is applied
    if(weight > 2.0){
      SerialBT.print(weight*100/DEFAULT_BW, 1);
      SerialBT.println(" %BW");
    }
  }
  
  SerialBT.flush();
  delay(25);
  digitalWrite(LED_BUILTIN, LOW);
  delay(25);
}