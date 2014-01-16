'''
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

This is an OLD version of the script for the FPGA Slave Version.
This basicly opens a MIDI Port and redirects to a Timidity output + FPGA Slave

Needs PyPM

'''

import time
import serial
import array
import math
import pypm

PitchSensitive  =    1      #   Pitch Bend Sensibility in Semitones
Client          =    129    #   This is the MIDI Client ID for Timidity    
Port            =    0      #   And the port
ADDR            =   0
def GetFreq(note):
    '''
        Given a Midi Note Number, return its Frequency
    '''
    return round(27.5 * math.pow(2.0,(note-21)/12.0)*10000)/10000

def GetPeriod(freq):
    '''
        Get rounded period to 6 decimal cases from frequency
    '''
    return round(1/freq * 1e6)

def GetPitchNote(note,pitch):
    '''
        Get the note pitched by pitch
    '''
    df        =    100.0 / 8192.0 / 1200.0
    return round(27.5 * math.pow(2.0,((note-21)/12.0)+(PitchSensitive*df*pitch))*10000)/10000
    

# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial('/dev/ttyUSB1', 38400)
ser.close()
ser.open()
ser.isOpen()

def ConfigMaxtOn(max):
    '''
        Sets the Maximum tOn on slave
    '''
    if max > 255:
        max = 255
    SendByte        =    array.array('B', [3, max, 0xFF, 0xFF]).tostring();
    ser.write(SendByte)

def ConfigPWM(number, period, tOn):
    '''
        Sets the PWM Period and tOn on the slave
    '''
    if period > 16383:
        period = 16383
    if tOn > 255:
        tOn = 255
    UpperNibble    =    (int) (period / 256)
    LowerNibble    =    (int) (period % 256)
    Channel        =    192 + number
    print "Sending (%s,%s,%s,%s)"%(hex(number),hex(UpperNibble),hex(LowerNibble),hex(tOn))
    SendByte        =    array.array('B', [number, 0x01, UpperNibble, LowerNibble, tOn]).tostring();
    ser.write(SendByte)

def StopPWM(number):
    '''
        Stops the PWM on Slave
    '''
    Channel        =    176 + number
    SendByte       =    array.array('B', [number, 0x02, 0xFF, 0xFF, 0xFF]).tostring()
    ser.write(SendByte)

def ResetPWM(number):
    '''
        Resets a PWM Channel
    '''
    SendByte    =    array.array('B', [number, 0x02, 0xFF, 0xFF]).tostring()
    ser.write(SendByte)

def ResetAll():
    '''
        Resets a PWM Channel
    '''
    SendByte    =    array.array('B', [0x00, 0x00, 0xFF, 0xFF]).tostring()
    ser.write(SendByte)
    
    
def InitAll():
    '''
        Initializes the system
    '''
    '''
    GetFirmware()
    for i in range(0,4):
        StopPWM(i)
        ResetPWM(i)
        ConfigPWM(i, 65535, 0)
        LedControl(i,1)
    state = False
    for i in range(0,100,4):
        LedControl(0,(int) (i*(255/100)))
        LedControl(1,(int) (i*(255/100)))
        LedControl(2,(int) (i*(255/100)))
        LedControl(3,(int) (i*(255/100)))
    for i in range(100,0,-4):
        LedControl(0,(int) (i*(255/100)))
        LedControl(1,(int) (i*(255/100)))
        LedControl(2,(int) (i*(255/100)))
        LedControl(3,(int) (i*(255/100)))

    LedControl(0,0)
    LedControl(1,0)
    LedControl(2,0)
    LedControl(3,0)
    for i in range(1,4):
        state = ~state
        ConfigPWM(0, GetPeriod(GetFreq(65+i)), 64)
        StartPWM(0)
        time.sleep(0.2)
        StopPWM(0)
    ConfigMaxtOn(255)
    '''
    ResetAll()
    time.sleep(1)

print "Starting"
InitAll()

mt = None
going = True

#   PWM Channels
PWM    =    [{'note':0,'period':0,'tOn':0,'busy':False},{'note':0,'period':0,'tOn':0,'busy':False},{'note':0,'period':0,'tOn':0,'busy':False},{'note':0,'period':0,'tOn':0,'busy':False}]

INPUT = 0
OUTPUT = 1
def PrintDevices(InOrOut):
    for loop in range(pypm.CountDevices()):
        interf,name,inp,outp,opened = pypm.GetDeviceInfo(loop)
        if ((InOrOut == INPUT) & (inp == 1) |
            (InOrOut == OUTPUT) & (outp ==1)):
            print loop, name," ",
            if (inp == 1): print "(input) ",
            else: print "(output) ",
            if (opened == 1): print "(opened)"
            else: print "(unopened)"
    print
PrintDevices(INPUT)
MidiIn = pypm.Input(1)
PrintDevices(OUTPUT)
print "Opening Timidity++"
MidiOut    =    pypm.Output(2,0)

print "Midi started!"
while True:
        if MidiIn.Poll():
                MidiData = MidiIn.Read(1)
                funcao    =    MidiData[0][0][0]
                fbyte     =    MidiData[0][0][1]
                sbyte        =    MidiData[0][0][2]
                if funcao == 0x90 or funcao == 0x80:
                    freq =    GetFreq(fbyte)
                    if funcao == 0x90:
                        #   This Starts a note fbyte with velocity (a.k.a. volume) sbyte
                        for x in range(0, len(PWM)):
                            if not PWM[x]["busy"]:
                                # tOn = (255 / 127) * Velocity
                                if    2*sbyte >= 10:
                                    tOn = 2*sbyte
                                else:
                                    tOn = 0
                                ConfigPWM(x, GetPeriod(freq), tOn)
                                if tOn > 0:
                                    #StartPWM(x)
                                    PWM[x]["busy"]        =    True


                                PWM[x]["note"]     =    fbyte
                                PWM[x]["period"]    =    GetPeriod(freq)
                                PWM[x]["tOn"]        =    tOn
                                break
                    elif funcao == 0x80:
                        #   This Stops a note fbyte
                        for x in range(0, len(PWM)):
                            if PWM[x]["busy"] == True and PWM[x]["note"] == fbyte:
                                StopPWM(x)    
                                PWM[x]["busy"]        =    False
                                break
                elif    funcao == 0xE0:
                    #   This is the Pitch Bend Function
                    pitch =  sbyte * 128 + fbyte - 8192
                    for x in range(0, len(PWM)):
                            if PWM[x]["busy"] == True:
                                PWM[x]["period"] = GetPeriod(GetPitchNote(PWM[x]["note"],pitch))
                                ConfigPWM(x, PWM[x]["period"], PWM[x]["tOn"])
                else:
                    #   We dont know this Midi Call, so we just print it.
                    print "Function "+hex(funcao)+" Data0 "+hex(fbyte)+" Data1: "+hex(sbyte)
                    MidiOut.Write([[[funcao,fbyte,sbyte],pypm.Time()]])
del MidiIn
del MidiOut
            
ser.close()
exit()
