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

#include <vector>
#include "midcontainers.h"

using namespace std;

Event::Event(unsigned short _statusmsg, int _tick, unsigned short _channel)    {
    statusmsg     =    _statusmsg;
    tick            =    _tick;
    channel        =    _channel;    
    played         =    false;
}

bool Event::operator< (Event param)    {
    return (tick < param.tick);
}

bool Event::operator> (Event param)    {
    return (tick > param.tick);
}
bool Event::operator== (Event param)    {
    return (tick == param.tick);
}

void Event::AddData(unsigned short _data)    {
    data.push_back(_data);
      //vector<unsigned short>::iterator it;
      //it = data.begin();
      //it = data.insert ( it , _data );
}

void Track::make_ticks_abs()    {
    int running_tick = 0;
    for (std::vector<Event>::iterator it = events.begin() ; it != events.end(); ++it)    {
            it->tick += running_tick;
            running_tick = it->tick;
    }
}

void Track::make_ticks_rel()    {
    int running_tick = 0;
    for (std::vector<Event>::iterator it = events.begin() ; it != events.end(); ++it)    {
            it->tick -= running_tick;
            running_tick = it->tick;
    }
}
void Track::AddDataToLast(unsigned short _data)    {
    events.back().AddData(_data);
}
void Track::AddEvent(unsigned short _statusmsg, int _tick, unsigned short _channel) {
      //vector<Event>::iterator it;
    Event tmp (_statusmsg, _tick, _channel);
    events.push_back(tmp);
      //it = events.begin();
      //it = events.insert ( it , tmp );
}
void Track::SortEvents()    {
    for(unsigned int i = 0; i < events.size();i++)
        for(unsigned int j = i+1; j < events.size(); j++)
                if(events[i] > events[j])
                    std::swap(events[i],events[j]);            
}
Pattern::Pattern(const int _resolution, const short _format)    {
    resolution     =     _resolution;
    format        =    _format;
}

void Pattern::Init(int _resolution=220, short _format=1)    {
    resolution     =     _resolution;
    format        =    _format;
}

void Pattern::make_ticks_abs()    {
    for (std::vector<Track>::iterator it = tracks.begin() ; it != tracks.end(); ++it)    
            it->make_ticks_abs();
}
void Pattern::make_ticks_rel()    {
    for (std::vector<Track>::iterator it = tracks.begin() ; it != tracks.end(); ++it)    
            it->make_ticks_rel();
}
void Pattern::AddTrack()    {
      vector<Track>::iterator it;
    Track tmp;
      it = tracks.begin();
      it = tracks.insert ( it , tmp );
}

void Pattern::SortEvents()    {
    for (std::vector<Track>::iterator it = tracks.begin() ; it != tracks.end(); ++it)    
            it->SortEvents();
}
int Pattern::numtracks()    {
    return tracks.size();
}
