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

using namespace std;

class Event    {
    public:
        Event(unsigned short, int, unsigned short);
        unsigned short statusmsg;
        bool played;
        int tick;
        unsigned short channel;
        vector<unsigned short> data;
        void AddData(unsigned short);
        bool operator < (Event);
        bool operator > (Event);
        bool operator == (Event);
};

class Track    {
    public:
        char *name;
        void make_ticks_abs();
        void make_ticks_rel();
        vector<Event> events;
        void SortEvents();
        void AddEvent(unsigned short, int, unsigned short);
        void AddDataToLast(unsigned short);
};



class Pattern    {
    public:
        int resolution;
        short format;
        Pattern    (const int _resolution=220, const short _format=1);
        void Init(int, short);
        vector<Track> tracks;
        void make_ticks_abs();
        void make_ticks_rel();
        void AddTrack();
        void SortEvents();
        int numtracks();
};



