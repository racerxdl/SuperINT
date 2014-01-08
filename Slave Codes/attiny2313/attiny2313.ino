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
int count = 0;
unsigned char buff[5];
static unsigned char  addr;

void setup()  {
    pinMode(9, OUTPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, OUTPUT);
    TCCR1A = (1 << COM1A1);    
    TCCR1B = _BV(WGM13) | 0x02 ; 
    TCNT1 = 0;
    OCR1A = 0;
    ICR1 = 1000;
    addr = ((digitalRead(4) << 2) | (digitalRead(3) << 1) | (digitalRead(2)) +1);
    Serial.begin(40000); 
    Serial.println("Started");
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
       if(buff[0] == addr)  {
          unsigned short period   =  ( (buff[2]*256) + buff[3] );
          unsigned short tOn      =  ( buff[4] );
          switch(buff[1])  {
            case 0x00:  //All Notes off
              digitalWrite(5, LOW);
              setPWM(1000, 0);
              break;
            case 0x01:
              digitalWrite(5, HIGH);
              setPWM(period, tOn);
              break;
            case 0x02:
              digitalWrite(5, LOW);
              setPWM(1000,0);
              break;
          }
        }
        count = 0;
    }
    if (Serial.available() > 0) {
     buff[count] = Serial.read();
     count++; 
    }   
    addr = ((digitalRead(4) << 2) | (digitalRead(3) << 1) | (digitalRead(2)) +1); 
}

