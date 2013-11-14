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

// Code optimized to ATMEGA 1280

#define ADDR  (0x01)	//	Address fixed to 1

unsigned char static buff[5];
unsigned char static count = 0;

void setup()  {
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(46, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    
    TCCR1A = (1 << COM1A1);     
    TCCR1B = _BV(WGM13) | 0x02 ;  
    TCNT1 = 0;
    OCR1A = 0;
    ICR1 = 1000;
    
    TCCR3A = (1 << COM1A1);  
    TCCR3B = _BV(WGM13) | 0x02 ;   
    TCNT3 = 0;
    OCR3A = 0;
    ICR3 = 1000;
    
    TCCR4A = (1 << COM1A1); 
    TCCR4B = _BV(WGM13) | 0x02 ;     
    TCNT4 = 0;
    OCR4A = 0;
    ICR4 = 1000;
    
    TCCR5A = (1 << COM1A1); 
    TCCR5B = _BV(WGM13) | 0x02 ;     
    TCNT5 = 0;
    OCR5A = 0;
    ICR5 = 1000;
    Serial.begin(115200); 
}

void setPWM(unsigned period, unsigned short tOn, unsigned short channel)  {
  switch(channel)  {
    case 0:
      OCR3A = tOn & 0xFF;
      ICR3 = period & 0xFFFF;
      break;
    case 1:
      OCR4A = tOn & 0xFF;
      ICR4 = period & 0xFFFF;
      break;   
    case 2:
      OCR5A  =  tOn & 0xFF;
      ICR5  = period & 0xFFFF;
      break;
    case 3:
      OCR1A  =  tOn & 0xFF;
      ICR1  =  period & 0xFFFF;
      break;
  }
}

void initResetTimer()  {
   TCCR2B = 0x00;
   TCNT2 = 0x00;
   TIFR2  = 0x00;
   TIMSK2 = 0x01;
   TCCR2A = 0x00;
   TCCR2B |= (1<< CS12) | (1<< CS10);
}
ISR(TIMER2_OVF_vect)
{
    if(count < 6 && Serial.available() == 0)  {
       count  =   0;    
       TCCR2B = 0x00;
       TCNT2 = 0x00;
       TIFR2  = 0x00;
    }
}

void loop()  {
    if(count==5)  {
       TCCR2B = 0x00;
       TCNT2 = 0x00;
       TIFR2  = 0x00; 
       Serial.println("OK");
        if(buff[0] >= ADDR && buff[0] <= ADDR+3)  {
          unsigned short period   =  ( (buff[2]*256) + buff[3] );
          unsigned short tOn      =  ( buff[4] );
          switch(buff[1])  {
            case 0x00:  //All Notes off
              setPWM(1000, 0, 0);
              setPWM(1000, 0, 1);
              setPWM(1000, 0, 2);
              setPWM(1000, 0, 3);
              break;
            case 0x01:
              setPWM(period, tOn,buff[0]-ADDR);
              break;
            case 0x02:
              setPWM(1000,0,buff[0]-ADDR);
              break;
          }
        }
        count = 0;
    }
    if (Serial.available() > 0) {
     initResetTimer();
     buff[count] = Serial.read();
     count++; 
    }   
}

