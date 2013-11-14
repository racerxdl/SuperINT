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
#include <fstream>
#include "midcontainers.h"
#include <stdio.h>
#include <string.h>

#define DEFAULT_MIDI_HEADER_SIZE 14
#define MICROSECONDS_PER_MINUTE 60000000
using namespace std;

class MidiParser    {
    ifstream midifile;
    char *header;

    public:
        MidiParser();
        void Open(char filename[]);
        int read_varlen();
        Pattern mididata;
        void parse_track(int, int);

        bool is_read;
        unsigned short format;
        unsigned short numtracks;
        bool isTicksPerBeat;
        unsigned int bpm, fps, tpb, tpf;
};


