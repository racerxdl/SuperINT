#include "Note.h"

Note::Note    ()    {
    freq = 0;
    period = 0;
    tOn = 0;
    note = 0;
}
Note::Note    (unsigned short note_in, unsigned short velocity)    {
    freq    =    round(27.5 * pow(2.0, (note_in-21)/12.0) * 10000) / 10000.0 ;
    period    =    round(1.0 / freq * 1e6);
    tOn        =    velocity * 2;
    note    =    note_in;
}
void    Note::ResetNote()    {
    freq    =    round(27.5 * pow(2.0, (note-21)/12.0) * 10000) / 10000.0 ;
    period    =    round(1.0 / freq * 1e6);
}

void SendSlaveData(int sp, int address, unsigned short period, unsigned short tOn, bool state)    {
    unsigned char cmd[5];
    unsigned short tmpperiod = period;
    if(state)    {    //    Turn on Note
        cmd[0]    =    (unsigned char) (address+1);                    //    Address
        cmd[1]    =    0x01;                                            //    Turn on
        cmd[2]    =    (unsigned char)    ((tmpperiod >> 8)    & 0xFF);    //    Period MSB
        cmd[3]    =    (unsigned char)    (tmpperiod & 0xFF);                //    Period LSB
        cmd[4]    =    (unsigned char)    (tOn & 0xFF);                    //    tOn
    }else{
        cmd[0]    =    (unsigned char) (address+1);
        cmd[1]    =    0x02;
        cmd[2]    =    (unsigned char)    ((tmpperiod >> 8)    & 0xFF);
        cmd[3]    =    (unsigned char)    (tmpperiod & 0xFF);
        cmd[4]    =    (unsigned char)    (tOn & 0xFF);
    }
    SendBuf(sp, cmd, 5);
}
