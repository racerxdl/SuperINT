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
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "../lib/rs232.h"

#define PITCH_SENSITIVE    2    //Semi-tones
#define MAX_NOTES 4


#ifndef NOTE_CLASS    
#define NOTE_CLASS
#define  df    100.0/8192.0/1200.0

class Note    {
    unsigned short period, tOn, note;
    float    freq;
    public:
        Note    ();
        Note    (unsigned short, unsigned short);
        float            GetFreq()                            {    return         freq;         };
        unsigned short    GetPeriod()                            {    return         period;     };
        unsigned short    GettOn()                            {    return         tOn;        };
        unsigned short    GettOn(unsigned short volume)        {    return    round((tOn/127.0)*volume);}
        unsigned short    GetNote()                            {    return         note;        };
        void            SetFreq(float freqin)                {    freq     =     freqin;     };
        void            SetPeriod(unsigned short periodin)    {     period     =     periodin;    };
        void            SettOn(unsigned short tOnIn)        {    tOn        =    tOnIn;        };
        void            SetNote(unsigned short    noteIn)        {    note    =    noteIn;        };
        void            ResetNote();
};

void SendSlaveData(int sp, int address, unsigned short period, unsigned short tOn, bool state);

#endif
