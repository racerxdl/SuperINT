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
#include "../lib/RtMidi.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "midparser.h"
static bool done = false;


double timecount, tickspersecond;
sf::Clock timer;
unsigned int lastevent = 0, nPorts;
bool pass = true, run = true;
static vector<unsigned char> message;
static MidiParser parser;

static double tickcount = 0;
static bool SequencerRunning = false;

static void finish(int ignore){ SequencerRunning = false; done = true; exit(1); }


class Sequencer : public sf::Thread    {
    public :
       void StartSequencer(int num, Track *_track, int midiport)    {
            track = _track;
            lastevent = 0;
            threadnum = num;
            midiout = new RtMidiOut();
            midiout->openPort( midiport );
            Launch();
        }

private :
    Track *track;
    int threadnum;
    RtMidiOut *midiout;
    unsigned int lastevent;
    virtual void Run()    {
        cout << "Thread " << threadnum << " running." << endl;
        while(SequencerRunning)    {
            for(unsigned int eventnum=lastevent;eventnum<track->events.size();eventnum++)    {
                Event *tmp = &track->events[eventnum];
                if(tmp->tick > tickcount)
                    break;
                if(!tmp->played)    {
                    if(tmp->statusmsg != 0xFF )    {
                        if(tmp->statusmsg >= 0x80)    {
                              message.push_back(tmp->statusmsg);
                            if(tmp->data.size() >= 1)
                                message.push_back(tmp->data[0]);
                              if(tmp->data.size() == 2)
                                message.push_back(tmp->data[1]);
                             midiout->sendMessage( &message );
                            message.erase(message.begin(),message.end());
                            message.resize(0);
                            message.clear();
                            cout << "Tick: " << tickcount << " - Out (0x" << hex << static_cast<int>(tmp->statusmsg) << ")" << endl;
                        }
                    }else{
                        switch(tmp->data[0])    {
                            case 0x51:
                                parser.bpm = tmp->data[1];
                                tickspersecond = ( (parser.bpm / 30.0) * ( parser.tpb));    
                                cout << "BPM Changed to " << parser.bpm << " - Ticks per second: " << tickspersecond << endl;
                                break;
                            case 0x58:
                                int numerador, denominador, clocks, notated;
                                numerador     =     tmp->data[1];
                                denominador =     tmp->data[2];
                                clocks        =    tmp->data[3];
                                notated        =    tmp->data[4];
                                cout << "Metronome: " << numerador << "/" << denominador << " - Clocks: " << clocks << " Notated: " << notated << endl;
                                break;
                            default:
                                cout << "Sem handler" << endl;
                        }    
                    }
                    tmp->played = true;
                    lastevent = eventnum;                            
                }
            }
        }
        delete  midiout;
        cout << "Thread " << threadnum << " closed." << endl;
    }
};

int main(int argc, char* argv[])
{


    char filename[] = "doom-e1m1.mid";
      RtMidiOut *midiout = new RtMidiOut();
    string portName;
    
    nPorts = midiout->getPortCount();
 
    if(argc < 2)    {
        if ( nPorts == 0 )     cout << "No ports available!\n";
        for ( unsigned int i=0; i<nPorts; i++ ) {
            try { portName = midiout->getPortName(i);    }
             catch (RtError &error) {error.printMessage(); }
            cout << "  Output Port #" << i+1 << ": " << portName << '\n';
        }
    }else{
        int port = atoi(argv[1]);
          //midiout->openPort( port );

        cout << "Opening midi" << endl;    
        char *filef = argv[2];
        if(argc == 3)
            parser.Open(filef);
        else
            parser.Open(filename);
    
        tickspersecond = ( (parser.bpm / 30.0) * ( parser.tpb));
        cout << " Ticks per second: " << tickspersecond << endl;
        
        (void) signal(SIGINT, finish);
        tickcount = 0;

        Sequencer sequencers[parser.numtracks];
        SequencerRunning = true;
        for(int i=0;i<parser.numtracks;i++)    {
            sequencers[i].StartSequencer(i, &parser.mididata.tracks[i], port);
            sf::Sleep(0.01f);
        }
        sf::Sleep(1);
        timer.Reset();
        while(run)    {
            timecount = timer.GetElapsedTime();
            tickcount += timecount * tickspersecond;
            timer.Reset();
            //sf::Sleep(0.0001f);
        }
    }
    delete midiout;    
    finish(0);
    return 0;
}

