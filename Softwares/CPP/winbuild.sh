#!/bin/bash

#/**************************************************************/
#/*     _____                            _          _          */
#/*    | ____|_ __   ___ _ __ __ _ _   _| |    __ _| |__  ___  */
#/*    |  _| | '_ \ / _ \ '__/ _` | | | | |   / _` | '_ \/ __| */
#/*    | |___| | | |  __/ | | (_| | |_| | |__| (_| | |_) \__ \ */
#/*    |_____|_| |_|\___|_|  \__, |\__, |_____\__,_|_.__/|___/ */
#/*                         |___/ |___/                        */
#/**************************************************************/
#/*       Por: Lucas Teske - lucas at teske dot com dot br     */
#/*              See link below for more info                  */
#/*           https://github.com/racerxdl/SuperINT             */
#/**************************************************************/

#   This is for Cygwin

echo "Compilando rs232"
gcc -c lib/rs232.c -Wall -O2 

ar rcs librs232.a rs232.o

mv *.o *.a lib/unix

g++ src/main.cpp -o SuperInt.exe -Llib/win32/ -lrtmidi -lwinmm -lrs232 -lsfml-graphics -lsfml-window -lsfml-system

echo "Compiling Note"
g++ -c src/Note.cpp -Wall -O2 -o obj/Note.o
echo "Compiling SoundManager"
g++ -c src/SoundManager.cpp -Wall -O2 -o obj/SoundManager.o
echo "Compiling MidContainers"
g++ -c src/midcontainers.cpp -Wall -O2 -o obj/midcontainers.o
echo "Compiling MidiParser"
g++ -c src/midparser.cpp -Wall -O2 -o obj/midparser.o

echo "Compiling MidiParser Tester"
g++ -O3 -Wall src/miditester.cpp obj/midparser.o obj/midcontainers.o lib/rtmidi-2.0.1/tests/Release/RtMidi.o -D__LINUX_ALSA__ -lpthread -lasound -lsfml-graphics -lsfml-window -lsfml-system -o midtester

echo "Compiling SuperINT"

g++ -O3 -Wall src/main.cpp lib/rtmidi-2.0.1/tests/Release/RtMidi.o obj/SoundManager.o obj/Note.o -Llib/win32/ -Ilib/rtmidi-2.0.1 -Ilib/rtmidi-2.0.1/include -lsfml-graphics -lsfml-window -lsfml-system -lrs232 -lpthread -lasound -o SuperINT.exe
