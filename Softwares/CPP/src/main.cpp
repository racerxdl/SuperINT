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


#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "../lib/rs232.h"

#include "Note.h"
#include "SoundManager.h"

using namespace std;

#ifndef _WIN_32
static void Sleep(unsigned short ms)    {
    sleep(ms);
}
#endif

bool done;
static sf::Image    ball, base, block_high, block_low, track_pitch, track_volume;
static sf::Sprite    ballspr,basespr,blockhspr,blocklspr,trackpitchspr,trackvolspr;
static sf::Font        BaseFont;
static sf::String    MidiPortString, SerialPortString, NotaString, tOnString, VolString, PitchString, VolValString, PitchValString, NoteTmpVal, ActiveChannel;
static short Pitch = 0;
static unsigned short volume = 127;
static void finish(int ignore){ done = true; }
static bool busy[4]; 
static int serialport = 0;
static int CH = 0;

static int midiport_number;
bool InitMIDI(RtMidiIn    *midi)    {
        unsigned int nPorts = midi->getPortCount();
        if ( nPorts == 0 ) {
            std::cout << "No available port!\n";
            return false;
        }
        
        midi->openPort( midiport_number );
        midi->ignoreTypes( false, false, false );
        done = false;
        return true;
}
void CheckMidi(RtMidiIn *midi, SoundManager *sm)    {
    std::vector<unsigned char> message;
    int nBytes, i;
    //double stamp;
    midi->getMessage( &message );
    nBytes = message.size();
    if(nBytes >    0)    {
        int m = (int) message[0];
        if ( m == (0x90 + CH))
            sm->PlayNote(serialport,     (unsigned short)message[1], (unsigned short)message[2]     );
        else if (m == (0x80 + CH))
            sm->StopNote(serialport,     (unsigned short)message[1], (unsigned short)message[2]     );
        else if (m == (0xE0 + CH))
            sm->PitchNote(serialport,    (unsigned short)message[1], (unsigned short)message[2]    );
        else if (m == (0xB0 + CH))
            sm->ChannelControl(serialport, (unsigned short)message[1], (unsigned short)message[2] );
        else{
                for ( i=0; i<nBytes; i++ )
                    std::cout << "Byte " << i << " = " << (int)message[i] << endl;    
        }
    }
}

void BurstMode(unsigned short freqpulse, unsigned short periodon, unsigned short periodoff, unsigned short tOn, unsigned short repeat)    {
    unsigned short repeated = 0;
    while(repeated != repeat)    {
        SendSlaveData(serialport, 0, (unsigned short)((1.0/freqpulse)*1e6), tOn, true);
        Sleep(periodon);
        SendSlaveData(serialport, 0, (unsigned short)((1.0/freqpulse)*1e6), tOn, false);
        Sleep(periodoff);
        repeated++;
    }
}

void tOnRamp(unsigned short freqpulse, unsigned short interval, unsigned short starttOn, unsigned short endtOn)    {
    unsigned short tOn     =    starttOn;
    while(tOn != endtOn)    {
        SendSlaveData(serialport, 0, (unsigned short)((1.0/freqpulse)*1e6), tOn, true);
        Sleep(interval);
        tOn++;
    }
    SendSlaveData(serialport, 0, (unsigned short)((1.0/freqpulse)*1e6), tOn, false);
}


static RtMidiIn *midiin;
static SoundManager *sm;

static inline std::string int2str(int x)    {
    std::ostringstream type;
    type << x;
    return type.str();
}
void InitResources()    {
    BaseFont.LoadFromFile("visual/AUGUSTUS.TTF", 16);
    
    base.LoadFromFile("visual/base.jpg");
    ball.LoadFromFile("visual/ball.png");
    block_high.LoadFromFile("visual/block_high.png");
    block_low.LoadFromFile("visual/block_low.png");
    track_pitch.LoadFromFile("visual/track_pitch.png");
    track_volume.LoadFromFile("visual/track_volume.png");

    basespr.SetImage(base);
    basespr.Resize(800, 600);
    ballspr.SetImage(ball);
    ballspr.Resize(32,32);
    blockhspr.SetImage(block_high);
    blocklspr.SetImage(block_low);
    trackpitchspr.SetImage(track_pitch);
    trackpitchspr.Resize(187,14);
    trackvolspr.SetImage(track_volume);
    trackvolspr.Resize(187,14);
    trackvolspr.SetPosition(310.f, 328.f);
    trackpitchspr.SetPosition(310.f, 360.f);
    
        
    MidiPortString = sf::String("MIDI Port: MIDI LOOPBACK", BaseFont, 16);
    MidiPortString.Move(24.f,70.f);
    MidiPortString.SetColor(sf::Color(0,0,0));
    
    SerialPortString = sf::String("Serial Port: COM10", BaseFont, 16);
    SerialPortString.Move(24.f,86.f);
    SerialPortString.SetColor(sf::Color(0,0,0));
    
    NotaString = sf::String("Note:", BaseFont, 16);
    NotaString.Move(208.f,236.f);
    NotaString.SetColor(sf::Color(0,0,0));
    
    tOnString = sf::String("tOn:", BaseFont, 16);
    tOnString.Move(208.f,278.f);
    tOnString.SetColor(sf::Color(0,0,0));
    
    VolString = sf::String("Volume: ", BaseFont, 16);
    VolString.Move(208.f,325.f);
    VolString.SetColor(sf::Color(0,0,0));
    
    PitchString = sf::String("Pitch: ", BaseFont, 16);
    PitchString.Move(208.f,360.f);
    PitchString.SetColor(sf::Color(0,0,0));
    
    VolValString = sf::String("100%", BaseFont, 16);
    VolValString.Move(530.f,325.f);
    VolValString.SetColor(sf::Color(0,0,0));
    
    PitchValString = sf::String("0", BaseFont, 16);
    PitchValString.Move(530.f,360.f);
    PitchValString.SetColor(sf::Color(0,0,0));
    
    NoteTmpVal= sf::String("", BaseFont, 16);
    NoteTmpVal.SetColor(sf::Color(0,0,0));

    ActiveChannel = sf::String("Active Channel: 0", BaseFont, 16);
    ActiveChannel.Move(24.f, 102.f);
    ActiveChannel.SetColor(sf::Color(0,0,0));
    
    midiin = new RtMidiIn();
    sm        =    new SoundManager();
    CH = 0;
}
int main(int argc, char* argv[])
{
    sf::RenderWindow App(sf::VideoMode(800, 600, 32), "SuperINT");
    InitResources();

    if(argc == 3)    {
        midiport_number =    atoi(argv[1]);
        serialport        =    atoi(argv[2]);
    }else{    
        unsigned int nPorts = midiin->getPortCount();
        std::cout << "Exists " << nPorts << " MIDI Ports Available.\nSelect one:\n";
        std::string portName;
        for ( unsigned int i=0; i<nPorts; i++ ) {
            try {
                portName = midiin->getPortName(i);
            }
            catch ( RtError &error ) {
                error.printMessage();
            }
            std::cout << "    " << i << ": " << portName << '\n';
        }
        cout << "Input the number of the MIDI Port to be used." << endl;
        string input = "";
        getline(cin,input);
        midiport_number = atoi(input.c_str());
        cout << "MIDI Port " << midiport_number << " selected" << endl;
        string tmp = "Porta MIDI:       ";
        tmp += midiin->getPortName(midiport_number);
        MidiPortString.SetText(tmp);
        cout << "Input the number of the serial port to be used." << endl;
        cout << "Example 0 to COM1 - /dev/ttyS0" << endl;
        input = "";
        getline(cin,input);
        serialport = atoi(input.c_str());
    }
    (void) signal(SIGINT, finish);
    if(!InitMIDI(midiin))    {
        delete midiin;
        delete sm;
        return 1;
    }
    if(OpenComport(serialport ,115200))    {
        cout << "Fail to open port " << serialport << endl;
        return 1;
    }else{
        cout << "Port " << serialport << " is open" << endl;
        string tmp = "Serial Port:       ";
        tmp += PortName(serialport);
        SerialPortString.SetText(tmp);
    }
    // Start main loop
    while (App.IsOpened())
    {
        CheckMidi(midiin,sm);
        *busy = sm->GetBusyFlag();
        volume = sm->GetVolume();
        Pitch = sm->GetPitch();
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            if (Event.Type == sf::Event::Closed)
                App.Close();

                if ((Event.Type == sf::Event::KeyPressed)) {
                    switch(Event.Key.Code)    {
                        case sf::Key::Escape: App.Close(); break;
                        case sf::Key::R:     
                            cout << "Reseting Notes" << endl;
                            sm->ShutNotesOff(serialport);                        
                            break;            
                        case sf::Key::Num0:
                            cout << "Changing Channel to 0, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 0;
                            break;            
                        case sf::Key::Num1:
                            cout << "Changing Channel to 1, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 1;
                            break;            
                        case sf::Key::Num2:
                            cout << "Changing Channel to 2, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 2;
                            break;            
                        case sf::Key::Num3:
                            cout << "Changing Channel to 3, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 3;
                            break;            
                        case sf::Key::Num4:
                            cout << "Changing Channel to 4, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 4;
                            break;            
                        case sf::Key::Num5:
                            cout << "Changing Channel to 5, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 5;
                            break;            
                        case sf::Key::Num6:
                            cout << "Changing Channel to 6, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 6;
                            break;            
                        case sf::Key::Num7:
                            cout << "Changing Channel to 7, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 7;
                            break;            
                        case sf::Key::Num8:
                            cout << "Changing Channel to 8, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 8;
                            break;            
                        case sf::Key::Num9:
                            cout << "Changing Channel to 9, reseting notes." << endl;
                            sm->ShutNotesOff(serialport);
                            CH = 9;
                            break;
                        default:    
                            cout << "Hotkey not found." << endl;
                    }
                }
        }

        
        App.Draw(basespr);
        App.Draw(trackvolspr);
        App.Draw(trackpitchspr);

        App.Draw(MidiPortString);
        App.Draw(SerialPortString);
        App.Draw(tOnString);
        App.Draw(NotaString);
        App.Draw(VolString);
        App.Draw(PitchString);

        ballspr.SetPosition((175.f / 127.f) * volume + 300, 319.f);
        App.Draw(ballspr);
        ballspr.SetPosition((175.f / 16384.f) * (Pitch+8192) + 300, 351.f);
        App.Draw(ballspr);
        VolValString.SetText(int2str((volume/127.f)*100)+"%");
        App.Draw(VolValString);
        PitchValString.SetText(int2str(Pitch));
        App.Draw(PitchValString);
        ActiveChannel.SetText("Active Channel: "+int2str(CH));

        App.Draw(ActiveChannel);
        for(int i=0;i < 4;i++)    {
            if(sm->GetBusyFlag()[i])    {
                Note tmp = sm->GetNote(i);
                blockhspr.SetPosition(269.f+81.f*i, 228.f);
                App.Draw(blockhspr);
                blockhspr.SetPosition(269.f+81.f*i, 268.f);
                App.Draw(blockhspr);
                NoteTmpVal.SetText(int2str(tmp.GetNote()));
                NoteTmpVal.SetPosition(300.f+81.f*i,238.f);
                App.Draw(NoteTmpVal);
                NoteTmpVal.SetText(int2str(tmp.GettOn()));
                NoteTmpVal.SetPosition(294.f+81.f*i,278.f);
                App.Draw(NoteTmpVal);
            }else{
                blocklspr.SetPosition(269.f+81.f*i, 228.f);
                App.Draw(blocklspr);    
                blocklspr.SetPosition(269.f+81.f*i, 268.f);
                App.Draw(blocklspr);    
                NoteTmpVal.SetText("Off");
                NoteTmpVal.SetPosition(294.f+81.f*i,238.f);
                App.Draw(NoteTmpVal);
                NoteTmpVal.SetText("0");
                 NoteTmpVal.SetPosition(298.f+81.f*i,278.f);
                App.Draw(NoteTmpVal);                
            }
        }
    
        App.Display();
    }
    delete midiin;    
    delete sm;
    return EXIT_SUCCESS;
}
