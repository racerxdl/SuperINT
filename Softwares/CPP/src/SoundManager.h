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
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <signal.h>
#include <math.h>
#include "Note.h"

using namespace std;

#ifndef SOUND_MANAGER
#define SOUND_MANAGER

class SoundManager    {
    Note channel[MAX_NOTES];
    bool busy[MAX_NOTES];
    unsigned short playing;
    unsigned short volume;
    short pitch;
    public:    
        SoundManager    ();
        void PlayNote(int, unsigned short,unsigned short);
        void StopNote(int, unsigned short,unsigned short);
        void PitchNote(int, unsigned short,unsigned short);
        void ChannelControl(int, unsigned short, unsigned short);
        void UpdateNotes(int);
        void ShutNotesOff(int);
        unsigned short GetPlaying()    {    return playing; };
        bool *GetBusyFlag()    {    return busy; };
        Note GetNote(unsigned short n) { return channel[n]; };
        unsigned short GetVolume()    { return volume; };
        short GetPitch()    { return pitch; };
};
#endif
