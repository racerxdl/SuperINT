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
#include <iomanip>
#include "midparser.h"

MidiParser::MidiParser()    {
    is_read = false;
    bpm = 120;
}

int MidiParser::read_varlen()    {
    bool nextbyte = true;
    int value = 0;
    char chr,chr2;
    while(nextbyte)    {
        midifile.get(chr);
        chr2 = chr;
        chr &= 0x7F;
        value = value << 7;
        value += chr;
        if ( ~(chr2 & 0x80) )    
            nextbyte = false;        
    }
    return value;    
}
void MidiParser::parse_track(int size, int trknum)    {
    int tick = 0, count = 0, tmpint, tmpint2;
    unsigned short status = 0;
    char *data = new char[1];
    char txtLen;
    bool notend = true;
    mididata.tracks[trknum].name = new char[0];
    while( notend )    {
        tick = read_varlen();
        midifile.read((char *)&status,1);
        if(status == 0xFF)    {
            midifile.read(data,1);
            tmpint2 = read_varlen();
            switch(data[0])    {
                case 0x00: data = new char[3]; midifile.read(data, 3); break;
            case 0x03:
                    data = new char[tmpint2];
                    midifile.read(data, tmpint2);
                    mididata.tracks[trknum].name = new char[tmpint2];
                    strcpy(mididata.tracks[trknum].name,data);
                break;
                case 0x01: 
            case 0x02:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
               midifile.read( &txtLen, 1 );
               data = new char[txtLen];
                    midifile.read(data, txtLen);
            break;
            case 0x20: data = new char[2]; midifile.read(data, 2); break;
            case 0x2F: notend = false; break;
            case 0x51: 
                    data = new char[tmpint2];
                    midifile.read(data, tmpint2);
                    tmpint = 0;
                    for(int i=0;i<tmpint2;i++)    {
                        tmpint = tmpint << 8;
                        tmpint += data[i];
                    }
                    int tmpbpm;
                    tmpbpm = 60000000 / tmpint;
                    mididata.tracks[trknum].AddEvent((unsigned short)(status), tick, 0);
                    mididata.tracks[trknum].AddDataToLast(0x51);    
                    mididata.tracks[trknum].AddDataToLast(tmpbpm);    
                    //bpm = tmpbpm;
                break;
            case 0x54: data = new char[6]; midifile.read(data, 6); break;
            case 0x58: 
                    data = new char[5]; midifile.read(data, 5); 
                    mididata.tracks[trknum].AddEvent((unsigned short)(status), tick, 0);
                    mididata.tracks[trknum].AddDataToLast(0x58);    
                    mididata.tracks[trknum].AddDataToLast(data[1]);
                    mididata.tracks[trknum].AddDataToLast(data[2]);
                    mididata.tracks[trknum].AddDataToLast(data[3]);
                    mididata.tracks[trknum].AddDataToLast(data[4]);                        
                    break;
            case 0x59: data = new char[3]; midifile.read(data, 3); break;
            case 0x7F:
               midifile.read( &txtLen, 1 );
               data = new char[txtLen];
                    midifile.read(data, txtLen);
               break;
            default:
                cout << "Meta evento desconhecido! " << endl;

            }
        }else if( (status & 0xF0) == 0xC0) {
            data = new char[1];
            midifile.read(data, 1);        
            mididata.tracks[trknum].AddEvent((unsigned short)(status), tick, (unsigned short)(status & 0xF));
            mididata.tracks[trknum].AddDataToLast((unsigned short)data[0]);    
        }else if( (status & 0xF0) == 0xD0) {
            data = new char[1];
            midifile.read(data, 1);    
            mididata.tracks[trknum].AddEvent((unsigned short)(status), tick, (unsigned short)(status & 0xF));
            mididata.tracks[trknum].AddDataToLast((unsigned short)data[0]);    
        }else {
            data = new char[2];
            midifile.read(data, 2);
            mididata.tracks[trknum].AddEvent((unsigned short)(status), tick, (unsigned short)(status & 0xF));
            mididata.tracks[trknum].AddDataToLast((unsigned short)data[0]);
            mididata.tracks[trknum].AddDataToLast((unsigned short)data[1]);
        }
        count ++;
    }
    cout << "Track [" << mididata.tracks[trknum].name << "] adicionada com " << dec << count << " eventos." << endl;
}

void MidiParser::Open(char filename[])    {
    char *buffer;
    header     =    new char [4];
    buffer    =    new char [10];
    short trackcount = 0;
    int size, timedivision;
    midifile.open(filename, ios::in|ios::binary|ios::ate);
    if(midifile.is_open())    {
        midifile.seekg (0, ios::beg);
        midifile.read(header, 4);
        if(strcmp(header, "MThd") != 0)
            cout << "Invalid file!" << endl;
        else{
                //Valid
                midifile.read(header, 4);    // 0x6
                midifile.read(buffer, 2);    // Format
                format = buffer[0] * 256 + buffer[1];
                midifile.read(buffer, 2);    // Number of Tracks
                numtracks = buffer[0] * 256 + buffer[1];
    
                midifile.read(buffer, 2);    // Time Division
                timedivision = buffer[0] * 256 + buffer[1];
                isTicksPerBeat = ~(timedivision & 0x8000);
                timedivision &= 0x7FFF;
                cout << "MIDI Version: " << format << endl << " Number of Tracks: " << numtracks << endl;
                if(isTicksPerBeat)    {
                    cout << " TimeDivision: " << timedivision << " Ticks per Beat" << endl;
                    tpb = timedivision;
                }else{
                    fps = ( ((unsigned char)buffer[0]) & 0x7F);
                    tpf = (unsigned char) buffer[1];
                    cout << " TimeDivision: " << fps << " Frames Per Second - Ticks Per Frame: " << tpf << endl;
                }
                cout << "Adding " << numtracks << " tracks. " <<endl;
                for(int i=0; i < numtracks; i++)    
                    mididata.AddTrack();
                cout << "Tracks on pattern: " << mididata.numtracks() << endl;
                while(trackcount < numtracks)    {
                    midifile.read(header, 4);
                    if(strcmp(header, "MTrk") != 0)    {
                        cout << "Invalid Track!" << endl;
                        break;
                    }
                    midifile.read(header, 4);
                    size = 0xFFFFFF * header[0] + 0xFFFF * header[1] + 0xFF * header[2] + header[3];
                    parse_track(size,trackcount);
                    trackcount++;
                }
                mididata.make_ticks_abs();    
                mididata.SortEvents();
        }
        midifile.close();
    }else
        cout << "Error reading file " << filename << endl;
}
