#include <Wire.h>
#define R 1

byte last_channel_1, last_channel_2, last_channel_3;
int X_channel, Y_channel, W_channel;
unsigned long timer_1, timer_2, timer_3;
byte payload1[2], payload2[2], payload3[2], payload4[2];
float Ang1, Ang2, Ang3, Ang4, X1, X2, X3, X4, Y1, Y2, Y3, Y4, M1, M2, M3, M4;
unsigned long loop_timer=0; //used to measure total loop time if desired
float transAm, totalAm, X_norm, Y_norm, W_norm, rotPriority, transPriority, max_XY, max_Mov, total_Mov;

void setup() {
  PCICR |= (1 << PCIE0);    // set PCIE0 to enable PCMSK0 scan
  PCMSK0 |= (1 << PCINT0);  // set PCINT0 (digital input 8) to trigger an interrupt on state change
  PCMSK0 |= (1 << PCINT1);  // set PCINT1 (digital input 9)to trigger an interrupt on state change
  PCMSK0 |= (1 << PCINT2);  // set PCINT2 (digital input 10)to trigger an interrupt on state change
  PCMSK0 |= (1 << PCINT3);  // set PCINT3 (digital input 11)to trigger an interrupt on state change

  delay(10);
  Wire.begin(); //initialize I2C for Master Device
  loop_timer=micros();
  Serial.begin(9600);
}

void loop() {
  
  if(X_channel < 12 && X_channel > -12) //Dead zone for each channel
  {X_channel=0;}
  if(Y_channel < 12 && Y_channel > -12)
  {Y_channel=0;}
  if(W_channel < 12 && W_channel > -12)
  {W_channel=0;}

  X_norm=((float)X_channel-1500)/500; //Channel 1 = X axis translation -1<0<1
  Y_norm=((float)Y_channel-1500)/500; //Channel 2 = Y axis translation -1<0<1
  W_norm=((float)W_channel-1500)/500; //Channel 3 = Rotation rate -1<0<1
  max_XY=max(abs(X_norm), abs(Y_norm));
  total_Mov=max(max_XY, abs(W_norm));
  //Serial.println(X_norm);
  //Serial.println(X_channel);
  
  transPriority=max_XY/(abs(W_norm)+max_XY);
  rotPriority=1-transPriority;
  /*
  Serial.print("total_Mov: ");
  Serial.println(total_Mov); 
  */
  /*
  Serial.print("transPriority: ");
  Serial.println(transPriority);
  Serial.print("total_Mov: ");
  Serial.println(rotPriority);
  */
  X1=X_norm*transPriority+0.707*W_norm*rotPriority; //Vector math from swerve drive superposition analysis (See Swerve_Analysis)
  Y1=Y_norm*transPriority+0.707*W_norm*rotPriority;
  X2=X1;
  Y2=Y_norm*transPriority-0.707*W_norm*rotPriority;
  X3=X_norm*transPriority-0.707*W_norm*rotPriority;
  Y3=Y2;
  X4=X3;
  Y4=Y1;
  
  Ang1=atan2(Y1, X1)*180/3.1416; //Calculating anlges from respective vector components (See Swerve_Analysis)
  Ang2=atan2(Y2, X2)*180/3.1416;
  Ang3=atan2(Y3, X3)*180/3.1416;
  Ang4=atan2(Y4, X4)*180/3.1416;
/*
  Serial.print("Angle 1: ");
  Serial.println(Ang1);
  Serial.print("Angle 2: ");
  Serial.println(Ang2);
  Serial.print("Angle 3: ");
  Serial.println(Ang3);
  Serial.print("Angle 4: ");
  Serial.println(Ang4);  
  */
  M1=sqrt(sq(X1)+sq(Y1)); //Motor speeds from vector magnitudes, not normalized (See Swerve_Analysis)           
  M2=sqrt(sq(X1)+sq(Y2));
  M3=sqrt(sq(X3)+sq(Y3));
  M4=sqrt(sq(X4)+sq(Y4));

  max_Mov=max(max(M1,M2), max(M3,M4)); //Normalizing motor speeds and multiplying by totalmove (angles do not change)
  M1=(M1/max_Mov)*total_Mov;
  M2=(M2/max_Mov)*total_Mov;
  M3=(M3/max_Mov)*total_Mov;
  M4=(M4/max_Mov)*total_Mov;
/*
  Serial.print("M1: ");
  Serial.println(M1);
  Serial.print("M2: ");
  Serial.println(M2);
  Serial.print("M3: ");
  Serial.println(M3);
  Serial.print("M4: ");
  Serial.println(M4);  
  */
/*
  Serial.print("X_norm: ");
  Serial.println(X_norm);
  Serial.print("Y_norm: ");
  Serial.println(Y_norm);
  Serial.print("W_norm: ");
  Serial.println(W_norm);
  */

  /*
  if(M1>500) M1 = 500; //Need to fix this - square of sums can be greater than motor max (Need functions from Andrew)
  if(M2>500) M2 = 500;
  if(M3>500) M3 = 500;
  if(M4>500) M4 = 500;
  */
  
  payload1[0] = map(Ang1, -180, 180, 0, 255); //payload[0] is angles for swerve modules 1-4 (Need to map to 0-255 as a BYTE)
  payload2[0] = map(Ang2, -180, 180, 0, 255);
  payload3[0] = map(Ang3, -180, 180, 0, 255);
  payload4[0] = map(Ang4, -180, 180, 0, 255);
  
  payload1[1] = (byte)(M1*255); //payload[1] is motor speeds for swerve modules 1-4 (Need to map to 0-255 as a BYTE)
  payload2[1] = (byte)(M2*255);
  payload3[1] = (byte)(M3*255);
  payload4[1] = (byte)(M4*255);

 
  Wire.beginTransmission(8); //sending payloads as a byte for angle and a byte for speed (2  bytes total for each slave)
  Wire.write(payload1,2);
  Wire.endTransmission();
  Wire.beginTransmission(9);
  Wire.write(payload2,2);
  Wire.endTransmission();
  Wire.beginTransmission(10);
  Wire.write(payload3,2);
  Wire.endTransmission();
  Wire.beginTransmission(11);
  Wire.write(payload4,2);
  Wire.endTransmission();
  
  loop_timer=micros()-loop_timer;
  Serial.println(loop_timer);
  delay(1000);
  loop_timer=micros();

}
//This routine is called every time input 8, 9, 10 or 11 changed state and is used to measure pulse length of receiver channels
ISR(PCINT0_vect){
  //Channel 1=========================================
  if(last_channel_1 == 0 && PINB & B00000001 ){         //Input 8 changed from 0 to 1
    last_channel_1 = 1;                                 //Remember current input state
    timer_1 = micros();                                 //Set timer_1 to micros()
  }
  else if(last_channel_1 == 1 && !(PINB & B00000001)){  //Input 8 changed from 1 to 0 (4)
    last_channel_1 = 0;                                 //Remember current input state
    X_channel = micros() - timer_1;      //X_channel is micros() - timer_1
  }
  //Channel 2=========================================
  if(last_channel_2 == 0 && PINB & B00000010 ){         //Input 9 changed from 0 to 1
    last_channel_2 = 1;                                 //Remember current input state
    timer_2 = micros();                                 //Set timer_2 to micros()
  }
  else if(last_channel_2 == 1 && !(PINB & B00000010)){  //Input 9 changed from 1 to 0 (2)
    last_channel_2 = 0;                                 //Remember current input state
    Y_channel = micros() - timer_2;      //Y_channel is micros() - timer_2
  }
  //Channel 3=========================================
  if(last_channel_3 == 0 && PINB & B00000100 ){         //Input 10 changed from 0 to 1
    last_channel_3 = 1;                                 //Remember current input state
    timer_3 = micros();                                 //Set timer_3 to micros()
  }
  else if(last_channel_3 == 1 && !(PINB & B00000100)){  //Input 10 changed from 1 to 0
    last_channel_3 = 0;                                 //Remember current input state
    W_channel = micros() - timer_3;      //W_Channel is micros() - timer_3
  }
}

/*
  transAm=sqrt(sq(X_norm)+sq(Y_norm));
  totalAm=sqrt(sq(X_norm)+sq(Y_norm)+sq(W_norm));
  rotPriority=abs(W_norm)/(transAm+abs(W_norm)+(1-(totalAm/1.732))); //1.732 = sqrt(3)
  transPriority=transAm/(transAm+abs(W_norm)+(1-(totalAm/1.732))); //1.732 = sqrt(3)


  X1=X_norm*transPriority+0.707*W_norm*rotPriority; //Vector math from swerve drive superposition analysis (See Swerve_Analysis)
  Y1=Y_norm*transPriority+0.707*W_norm*rotPriority;
  X2=X1;
  Y2=Y_norm*transPriority-0.707*W_norm*rotPriority;
  X3=X_norm*transPriority-0.707*W_norm*rotPriority;
  Y3=Y2;
  X4=X3;
  Y4=Y1;
  */

