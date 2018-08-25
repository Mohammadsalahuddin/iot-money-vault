/*   Infinenon 3D Magnetic I2c
 *   this code was taken from 
 *   https://www.allaboutcircuits.com/technical-articles/tutorial-and-overview-of-infineons-3d-magnetic-2go-kit/
 *   thanks to by Mark J. Hughes#include <ThingerYun.h>

 *   
 */

//--- Begin Includes ---//
#include <I2C.h>       // http://dsscircuits.com/articles/arduino-i2c-master-library
#include<math.h>
#include <BridgeSSLClient.h>
#include <ThingerYun.h>

#define USERNAME "Your user name"
#define DEVICE_ID "your device id"
#define DEVICE_CREDENTIAL "your credential"

ThingerYun thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// Variable Declaration
const byte addr = 0x35; // default address of magnetic sensor 0x35
byte rbuffer[10];       // store data from sensor read registers
byte delaytime = 1;     // time to wait before next read
bool PRINT_RAW_VALUES = false;
float norm;

//--- Begin Setup ---//
void setup() {
  Serial.begin(115200);      // Begin serial connection for debug.
  I2c.begin();              // Begin I²C I2c communication
  I2c.timeOut(100);
  I2c.write(addr, 0x00,0x05);

   pinMode(LED_BUILTIN, OUTPUT);

  // initialize bridge
  Bridge.begin();

  // pin control example (i.e. turning on/off a light, a relay, etc)
  thing["led"] << digitalPin(LED_BUILTIN);

  // resource output example (i.e. reading a sensor value, a variable, etc)

  // more details at http://docs.thinger.io/arduino/
  
} 
//--- End of Setup --//

//--- Begin Main Program Loop --//
void loop() {
  
  delay(delaytime); // wait time between reads.
  // Read sensor registers and store in rbuffer
    I2c.read(addr,7);
      for(int i=0; i < 7; i++){
        rbuffer[i] = I2c.receive();
      }  

  // Goto decode functions below     
  int x = decodeX(rbuffer[0],rbuffer[4]);
  int y = decodeY(rbuffer[1],rbuffer[4]);
  int z = decodeZ(rbuffer[2],rbuffer[5]);
  int t = decodeT(rbuffer[3],rbuffer[6]);

  if(rbuffer[3] & B00000011 != 0){ // If bits are not 0, TLV is still reading Bx, By, Bz, or T
    Serial.println("Data read error!");

  }
  else {
    if(PRINT_RAW_VALUES){
        Serial.print(x);
        Serial.print("\t");
        Serial.print(y);
        Serial.print("\t");
        Serial.print(z);
        Serial.print("\t");
        Serial.println(t);
    }
    else{
        // Serial.println("the converted value");     
        // Serial.print(convertToMag(x));
        // Serial.print("\t");
        // Serial.print(convertToMag(y));
        // Serial.print("\t");
        // Serial.print(convertToMag(z));
        // Serial.print("\t");
        // Serial.println(convertToCelsius(t));
       Serial.print("the value is ");
       norm = sqrt(pow(static_cast<float>(convertToMag(x)), 2) + pow(static_cast<float>(convertToMag(y)), 2) + pow(static_cast<float>(convertToMag(z)), 2));
       Serial.println(norm);
    }
  }

delay(500);
thing.handle();

}
//-- End of Main Program Loop --//

//-- Begin Buffer Decode Routines --//
int decodeX(int a, int b){
/* Shift all bits of register 0 to the left 4 positions.  Bit 8 becomes bit 12.  Bits 0:3 shift in as zero.
 * Determine which of bits 4:7 of register 4 are high, shift them to the right four places -- remask in case
 * they shift in as something other than 0.  bitRead and bitWrite would be a bit more elegant in next version
 * of code.
 */
  int ans = ( a << 4 ) | (((b & B11110000) >> 4) & B00001111);

  if( ans >= 2048){ ans = ans - 4096; } // Interpret bit 12 as +/-
  return ans;
  }

int decodeY(int a, int b){
/* Shift all bits of register 1 to the left 4 positions.  Bit 8 becomes bit 12.  Bits 0-3 shift in as zero.
 * Determine which of the first four bits of register 4 are true.  Add to previous answer.
 */

  int ans = (a << 4) | (b & B00001111);
  if( ans >= 2048){ ans = ans - 4096;} // Interpret bit 12 as +/-
  return ans;
}

int decodeZ(int a, int b){
/* Shift all bits of register 2 to the left 4 positions.  Bit 8 becomes bit 12.  Bits 0-3 are zero.
 * Determine which of the first four bits of register 5 are true.  Add to previous answer.
 */
  int ans = (a << 4) | (b & B00001111);
  if( ans >= 2048){ ans = ans - 4096;}
  return ans;
}

int decodeT(int a, int b){
/* Determine which of the last 4 bits of register 3 are true.  Shift all bits of register 3 to the left 
 * 4 positions.  Bit 8 becomes bit 12.  Bits 0-3 are zero.
 * Determine which of the first four bits of register 6 are true.  Add to previous answer.
 */
  int ans;
  a &= B11110000;
  ans = (a << 4) | b;
  if( ans >= 2048){ ans -= 4096;}
  return ans;
}


float convertToMag(int a){
  return a * 0.098;
}

float convertToCelsius(int a){
  return (a-320)* 1.1;
}
