#include <Servo.h> 
#include <Wire.h> 

const int pingPin = 10;
const int servoXPin = 11;
const int servoYPin = 12;
boolean roadblock = false;





Servo servoX;
Servo servoY;
                // create servo object to control a servo 
                // a maximum of eight servo objects can be created 

 
int pos = 0;    // variable to store the servo position 

int value = 0;

String inString = "";    // string to hold input

boolean sweepRoutine = false;



int compassAddress = 0x42 >> 1; // From datasheet compass address is 0x42
// shift the address 1 bit right, the Wire library only needs the 7
// most significant bits for the address
int reading = 0; 
//int WLED = 13;


void setup() {
  // initialize serial communication:

  Serial1.begin(9600);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.begin(9600);
   Wire.begin();       // join i2c bus (address optional for master) 
  servoX.attach(11);
  servoY.attach(12);
  servoY.write(0);
}

void loop()
{
 Serial.println("In Loop");
  
  //sweep();
  readCommand();
  
  if(sweepRoutine){
    sweep();
  if(roadblock){
    // false is 1
    Serial.println(1);
    Serial1.println(1);
  }
  else
  {
    // true is 0
    Serial.println(0);Serial1.println(0);
  }
  }
  
  
}

void readCompass()
{
// step 1: instruct sensor to read echoes 
  Wire.beginTransmission(compassAddress);  // transmit to device
  // the address specified in the datasheet is 66 (0x42) 
  // but i2c adressing uses the high 7 bits so it's 33 
  Wire.write('A');        // command sensor to measure angle  
  Wire.endTransmission(); // stop transmitting 

  // step 2: wait for readings to happen 
  delay(10); // datasheet suggests at least 6000 microseconds 

  // step 3: request reading from sensor 
  Wire.requestFrom(compassAddress, 2); // request 2 bytes from slave device #33 

  // step 4: receive reading from sensor 
  if (2 <= Wire.available()) // if two bytes were received 
  { 
    reading = Wire.read();  // receive high byte (overwrites previous reading) 
    reading = reading << 8; // shift high byte to be high 8 bits 
    reading += Wire.read(); // receive low byte as lower 8 bits 
    reading /= 10;
    Serial.println(reading); // print the reading
  } 

  delay(500); // wait for half a second
}

void readCommand()
{
 // Read serial input:
  while (Serial1.available() > 0) {
    int inChar = Serial1.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char 
      // and add it to the string:
      Serial.println((char)inChar);
      inString += (char)inChar; 
    }
    // if you get a newline, print the string,
    // then the string's value:
    if (inChar == '\n') {
      if(inString.toInt() == 0)
      {
        Serial.println("Command: Start Sweep");
        sweepRoutine = true;
      }
      else if(inString.toInt() == 1)
      {
        Serial.println("Command: Stop Sweep");
        sweepRoutine = false;
      }
      else if(inString.toInt() == 2)
      {
        Serial.println("Command: Read Compass");
        readCompass();
      }
      /*
      Serial.print("Value:");
      Serial.println(inString.toInt());
      servoX.write(inString.toInt());
      Serial.print("String: ");
      Serial.println(inString);
      */
      // clear the string for new input:
      inString = ""; 
    }
  }
}

void sweep()
{
  roadblock = false;
  //sweeping = true;
  servoX.write(50);
for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    //servoX.write(pos);              // tell servo to go to position in variable 'pos' 
    
    
    servoY.write(pos);
    if(pos == 1 || pos == 90 || pos == 178)
    {
      Serial.println("Begin ping");
      ping();
      Serial.println("Ping complete");
    }
    
    delay(5);                       // waits 15ms for the servo to reach the position 
  } 
  //delay(500);
  /*for(pos = 180; pos>=1; pos-= 1)     // goes from 180 degrees to 0 degrees 
  {                                
    //servoX.write(pos);              // tell servo to go to position in variable 'pos' 
    servoY.write(pos);
    ping();
    delay(5);                       // waits 15ms for the servo to reach the position 
  } */
  //sweeping = false;
}

void ping()
{
// establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  
  if(cm < 10){
    if(cm == 0)
    {
      Serial.println("Bad ping -- Reping");
    ping();
    Serial.println("Reping complete");
    }
    else{
    Serial.println("Road block detected");
    roadblock = true;
    }
  }
  
  
  //delay(100);
  
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
