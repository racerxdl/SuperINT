/**************************************************************/
/*     _____                            _          _          */
/*    | ____|_ __   ___ _ __ __ _ _   _| |    __ _| |__  ___  */
/*    |  _| | '_ \ / _ \ '__/ _` | | | | |   / _` | '_ \/ __| */
/*    | |___| | | |  __/ | | (_| | |_| | |__| (_| | |_) \__ \ */
/*    |_____|_| |_|\___|_|  \__, |\__, |_____\__,_|_.__/|___/ */
/*                         |___/ |___/                        */
/**************************************************************/
/*       Por: Lucas Teske - lucas at teske dot com dot br     */
/*              See link below for more info                  */
/*           https://github.com/racerxdl/SuperINT             */
/**************************************************************/
/*     The code is BIG for attiny2313, so I removed the part  */
/*  that does an timeout for the data. Should work fine.      */
/**************************************************************/

// Code optimzed for ATTINY2313
#define ADDRESS 0  //  Use this to define the chip Address

int count = 0;
int softint = 0;
unsigned char buff[5];
//static unsigned char  addr = ADDRESS;

void setup()  {
    pinMode(PIN_B3, OUTPUT);
    pinMode(PIN_B4, OUTPUT);
    //pinMode(CORE_PWM1_PIN, OUTPUT);
    //pinMode(CORE_PWM2_PIN, OUTPUT);
    //pinMode(CORE_PWM3_PIN, OUTPUT);
    //pinMode(PIN_B1, INPUT);
    //pinMode(PIN_B0, INPUT);
    //pinMode(PIN_D6, INPUT);
    pinMode(PIN_D2, OUTPUT);
    TCCR1A = (1 << COM1A1);    
    TCCR1B = _BV(WGM13) | 0x02 ; 
    TCNT1 = 0;
    OCR1A = 0;
    ICR1 = 1000;
    //addr = ADDRESS;
    //addr = ((digitalRead(4) << 2) | (digitalRead(3) << 1) | (digitalRead(2)) +1);
    Serial.begin(38400); 
    Serial.println("Started");
    softint = millis();
}

void setPWM(unsigned period, unsigned short tOn)  {
  OCR1A = tOn;
  ICR1 = period;
}

void loop()  {
    if(count==5)  {
       TCCR0B = 0x00;
       TCNT0 = 0x00;
       TIFR  = 0x00; 
       if(buff[0] == ADDRESS)  {
          unsigned short period   =  ( (buff[2]*256) + buff[3] );
          unsigned short tOn      =  ( buff[4] );
          switch(buff[1])  {
            case 0x00:  //All Notes off
              //Serial.println("AOFF");
              digitalWrite(PIN_D2, LOW);
              setPWM(1000, 0);
              break;
            case 0x01:
              //Serial.println("ON");
              digitalWrite(PIN_D2, HIGH);
              setPWM(period, tOn);
              break;
            case 0x02:
              //Serial.println("OFF");
              digitalWrite(PIN_D2, LOW);
              setPWM(1000,0);
              break;
          }
        }
        count = 0;
    }
    if (Serial.available() > 0) {
     buff[count] = Serial.read();
     count++; 
     //softint = millis();
    }   
    if(millis()-softint > 300)  {
     count = 0; 
     softint = millis();
    }
    //addr = ((digitalRead(4) << 2) | (digitalRead(3) << 1) | (digitalRead(2)) +1); 
}

