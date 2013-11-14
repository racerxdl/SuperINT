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
#include "SoundManager.h"

SoundManager::SoundManager    ()    {
    for(int i=0;i<MAX_NOTES;i++)    
        busy[i] = false;
    playing = 0;
    volume = 127;
    pitch = 8192;
}
void SoundManager::PlayNote(int sp, unsigned short note, unsigned short velocity)    {
    for(int i=0;i<MAX_NOTES;i++)    {
        if(!busy[i])    {
            channel[i] = Note(note,velocity);
            channel[i].SetFreq( round(27.5 * pow(2.0, ((channel[i].GetNote()-21)/12.0) + (PITCH_SENSITIVE*df*pitch)) * 10000)/10000);
            channel[i].SetPeriod(1.0 / channel[i].GetFreq() * 1e6);
            cout << "Nota ligada ["<<i<<"] - Freq: " << channel[i].GetFreq() << "Hz - Periodo: " << channel[i].GetPeriod() << "us tOn: " << channel[i].GettOn(volume) << "us" << endl;
            busy[i] = true;
            SendSlaveData(sp, i, channel[i].GetPeriod(), channel[i].GettOn(volume), true);
            playing++;
            break;
        }
    }
}

void SoundManager::StopNote(int sp, unsigned short note, unsigned short velocity)    {
    for(int i=0;i<MAX_NOTES;i++)    {
        if(channel[i].GetNote() == note && busy[i])    {
            cout << "Nota desligada ["<<i<<"]" << endl;
            busy[i] = false;
            playing--;
            SendSlaveData(sp, i, channel[i].GetPeriod(), channel[i].GettOn(volume), false);
            break;
        }
    }
}

void SoundManager::PitchNote(int sp, unsigned short byte0, unsigned short byte1)    {
    pitch    =    (byte1 * 128 + byte0) - 8192;
    for(int i=0;i<MAX_NOTES;i++)    {
        if(busy[i])    {
            channel[i].SetFreq( round(27.5 * pow(2.0, ((channel[i].GetNote()-21)/12.0) + (PITCH_SENSITIVE*df*pitch)) * 10000)/10000);
            channel[i].SetPeriod(1.0 / channel[i].GetFreq() * 1e6);
            SendSlaveData(sp, i, channel[i].GetPeriod(), channel[i].GettOn(volume), true);
        }
    }
}
void SoundManager::UpdateNotes(int sp)    {
    for(int i=0;i<MAX_NOTES;i++)    {
        if(busy[i])    {
            channel[i].SetFreq( round(27.5 * pow(2.0, ((channel[i].GetNote()-21)/12.0) + (PITCH_SENSITIVE*df*pitch)) * 10000)/10000);
            channel[i].SetPeriod(1.0 / channel[i].GetFreq() * 1e6);
            //cout << "UpdateNotes ["<<i<<"] - Freq: " << channel[i].GetFreq() << "Hz - Periodo: " << channel[i].GetPeriod() << "us tOn: " << channel[i].GettOn(volume) << "us" << endl; 
            SendSlaveData(sp, i, channel[i].GetPeriod(), channel[i].GettOn(volume), true);
        }
    }
}
void SoundManager::ChannelControl(int sp, unsigned short byte0, unsigned short byte1)    {
    switch(byte0)    {
        case 0x07:    //Main Volume
            volume = byte1;
            cout << "Novo Volume: " << volume << endl;
            UpdateNotes(sp);
            break;
        case 0x7B:
            cout << "Todas as notas desligadas!" <<endl;
            ShutNotesOff(sp);
            break;
    }
}
void SoundManager::ShutNotesOff(int sp)    {
    unsigned char cmd[] = { 0x01,0x00,0x00,0x00,0x00 };
    SendBuf(sp, cmd, 5);
    channel[0].SettOn(0);
    channel[1].SettOn(0);
    channel[2].SettOn(0);
    channel[3].SettOn(0);
    busy[0] = false;
    busy[1] = false;
    busy[2] = false;
    busy[3] = false;
}
