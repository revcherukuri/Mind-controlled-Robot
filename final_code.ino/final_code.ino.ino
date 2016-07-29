/*
 MIND CONTROLLED ARDUINO
 by: Revanth Cherukuri

 date: February 26, 2013

 */

//includes

#include <Servo.h>
#include <SoftwareSerial.h>

#define DEBUGOUTPUT 1
Servo servoLeft; //setting left servo
Servo servoRight; //setting right servo
int ledPin = 5; //initializing a led pin to pin 5
int ledPin2 = 11; //initilaizing another led to pin 11

byte payloadData[256] = {0}; //initializing payloadData to 0
byte generatedChecksum = 0; //initializing generatedCheckcum to 0
byte checksum = 0; //initializing checksum to 0
int payloadLength = 0; //initializing payloadLength to 0
//byte payloadData[256] = {0};
byte poorQuality = 0; //initializing poorQuality to 0
byte attention = 0; //initializing attention to 0
byte meditation = 0; //initilaizing meditation to 0
int maxAttention = 0; //initializing maxAttention to 0
int servoUnit = 0; //initializng servoUnit to 0
int moveUp = 0; //initializing moveUp to 0

// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;
int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3
int batteryLevel = 0;

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup()
{
  //  servoRight.attach(A0);
  //  servoLeft.attach(A1);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  bluetooth.begin(57600);  // Start bluetooth serial at 576000
  delay(50);
  bluetooth.write(0x0B);
}
byte ReadOneByte() {
  int byteRead;

  while (!bluetooth.available()) {
    //    Serial.print("Waiting on bluetooth...\n");
  }
  byteRead = bluetooth.read();

  //  Serial.print((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
  //  Serial.print("\n");

  return byteRead;
}


void loop() {
  //  Serial.print("in loop\n");
  ledDisplay(); //call led method

  while (1) {
    if (ReadOneByte() != 170) {
      continue;
    }
    if (ReadOneByte() != 170) {
      continue;
    }
    break;
  }
  //Serial.print("ready to read length:");
  payloadLength = ReadOneByte();
//  while (payloadLength == 170)
//  {
//    payloadLength = ReadOneByte();
//  }
  //  Serial.print("Pay load length = ");
  //  Serial.println(payloadLength, DEC);
  //Serial.println(payloadLength, DEC);
  if (payloadLength > 169) {                 //Payload length can not be greater than 169
    //    Serial.print("Wrong payload length:");a
    //    Serial.println((int)payloadLength);
    return;
  }

  generatedChecksum = 0; //sets the sum of the bytes of a certain part of the array to 0
  for (int i = 0; i < payloadLength; i++) {
    int data = ReadOneByte();
    payloadData[i] = data; //ReadOneByte();            //Read payload into memory
    generatedChecksum += payloadData[i]; //generated checksum will increase until it is equal to payloadLength
    //    Serial.print(payloadData[i], HEX);
    //    Serial.print(",");
  }
  //  Serial.print("\n");
  checksum = ReadOneByte();         //Read checksum byte from stream
  generatedChecksum = 255 - generatedChecksum;
  //  generatedChecksum &= 0xFF;
  //  generatedChecksum = ~generatedChecksum & 0xFF;
  if (checksum != generatedChecksum) {
    //    Serial.println("wrong checksum");
    return;
  }
  if (checksum == generatedChecksum) { //the checksum recently read has to be equal to generated checksum
    //Serial.print("Checksum working\n");
    for (int i = 0; i < payloadLength; i++) { //until i is = to payload's length, execute loop
      switch (payloadData[i]) { //switching 256 to the i value
        case 0x01: //specific hexadecimal value corresponding to an action by the headset (battery level)
          i++;
          batteryLevel = payloadData[i];
          Serial.print("BATTERY LEVEL= ");
          Serial.println(batteryLevel, DEC);
          break;
        case 0x2: //(signal quality)
          i++;
          poorQuality = payloadData[i];
          bigPacket = true;
          Serial.print("poorQuality= ");
          Serial.println(poorQuality, DEC);
          break;
        case 0x4: //attention
          i++;
          attention = payloadData[i];
          ledDisplay(); //call led method, to display attention value
          Serial.print("attention= ");//display attention value
          Serial.println(attention, DEC);
          moveServo(); //call servo method
          break;
        case 0x5: //meditation
          i++;
          meditation = payloadData[i];
          ledDisplay(); //call led method, to display meditation value
//          digitalWrite(9, LOW);
          Serial.print("meditation= ");
          Serial.println(meditation, DEC);
          moveServo(); //unnessecary at the moment, because robot only moves to attention values. 
          break;
        case 0x80: //single big-endian 16 bit value
          i = i + 3;
          break;
        case 0x81: //eight bid-endian 4 byte point values representing different brainwaves
          i = i + 33;
          break;
        case 0x83: //eight big-endian 3 byte unsigned integer values representing different brainwaves
//          Serial.println("got brain wave");
          i = i + 25;
          break;
        case 0x86: //two byte big-endian unsigned integer
          i = i + 3;
          break;
        default:
          break;
      }
    }
  }
}
void ledDisplay() {
  if (attention <= 100 && attention > 0) //if attention is less than or equal to 100 and greater than 0, then the new variable attn equals to the attention value multiplied by 255 and divided by 100
  {
      int attn = attention * 255 / 100;
//    Serial.print("attn = ");
//    Serial.println(attn);
    analogWrite(ledPin, attn); //YELLOW LED SHOWS ATTN VALUE
//    delay(100);
//    analogWrite(ledPin, 0);
  }
  if(meditation <= 100 && meditation > 0) //if meditation value is less than or equal to 100 and greater than 0, then new variable medit = meditation multiplied by 255 and divided by 100, then added to meditation times 2
  {
    int medit = (meditation * 255 / 100) + (meditation * 2);
    analogWrite(ledPin2, medit); //led showing mediation value
  }
    
}

void moveServo() {
  if (attention > 100 || attention <= 0) { //if attention is less than 100 and greater than or equal to 0, then return the value. 
    return;
  }
  if (attention  > maxAttention) { //if attention ids greater than the max attention, carry out conditional
    //digitalWrite(ledPin2, 100); //GREEN LED SHOWS THAT ATTENTION IS GREATER THAN MAX ATTENTION
    moveUp = (attention - maxAttention) / 10; //variable moveUp is the number of segments robot moves up. This is calculated by the amount of space between the current max attention value, and the new max value you just recieved. The space between these two is divided by 10, for a single digit number.
    maxAttention = attention; //the new max atteniton value
    Serial.print("moveUp: ");
    Serial.println(moveUp);
    servoLeft.attach(A0);//begins to move servo
    servoRight.attach(A1);//begins to move servo
    servoLeft.write(180); //direction of servo (backwards)
    servoRight.write(0); //direction of servo(forward)
    delay(moveUp * 703); //time servos move. The robot takes 703 milliseconds to move one segment.(half the wheels' diameter). This time is multiplied by number of segments to move(moveUp).
    servoLeft.detach(); //stops servo
    servoRight.detach(); //stops servo
  }
  

}








//
//
//  if(bigPacket) {
//    if(poorQuality == 0)
//    {}
//    else
//    Serial.print("PoorQuality: ");
//    Serial.print(poorQuality, DEC);
//    Serial.print("Attention: ");
//    Serial.print(attention, DEC);
//    Serial.print(" Time since last packet: ");
//    Serial.print(millis() - lastReceivedPacket, DEC);
//    lastReceivedPacket = millis();
//    Serial.print("\n");
//  }
//    // Send any characters the bluetooth prints to the serial monitor
//    Serial.print((char)bluetooth.read());
//  }
//        bigPacket = false;
//      }
//     }
//
////  if(Serial.available())  // If stuff was typed in the serial monitor
////  {
////    // Send any characters the Serial monitor prints to the bluetooth
////    bluetooth.print((char)Serial.read());
////  }
//  // and loop forever and ever!

