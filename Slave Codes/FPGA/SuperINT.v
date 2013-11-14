`timescale 1ns / 1ps

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

module SuperINT(
    input clk,
    input SerialRX,
    input Reset,
    output SerialTX,
    output saida
);
//Parameters
parameter    FirmwareVersion     =  6;             //    Firmware Version
parameter    FirmwareYear        =  12;            //    Firmware Year
parameter    FirmwareMonth       =  07;            //    Firmware Month

//Microsseconds divider    

reg     [3:0]     contus         = 0;               //  Counter for division by 16
wire              clk1us         = (contus > 7);    //  Clock 1us

always @(posedge clk)
    contus <= contus +1;

//Main Circuit

wire [23:0] FrequencyCounter;
wire outprot;

//Led Control
reg    [7:0]      ledpwm0           =     0;
reg    [7:0]      ledpwm1           =     0;
reg    [7:0]      ledpwm2           =     0;
reg    [7:0]      ledpwm3           =     0;

//PWM Registers
reg     [15:0]    period0           =    65535;
reg     [15:0]    period1           =    65535;
reg     [15:0]    period2           =    65535;
reg     [15:0]    period3           =    65535;

reg     [7:0]     tOn0              =    0;
reg     [7:0]     tOn1              =    0;
reg     [7:0]     tOn2              =    0;
reg     [7:0]     tOn3              =    0;

reg     [3:0]     EnablePWM         =    0;
reg     [3:0]     ResetPWM          =    0;
wire    [3:0]     SaidaPWM;

//Serial TX Registers

reg    [7:0]        TXData              [64:0];            //    Output buffer
reg    [5:0]        TXByteCount     =    0;                //    Bytes to Send
reg    [5:0]        TXByteSent      =    0;                //    Bytes sent
reg                 TXBusy          =    0;                //    Send Started Flag
reg    [7:0]        TXSendBuffer    =    0;                //    Send Buffer
reg                 TXSend          =    0;                //    Send Flag
wire                TXTransmitting;                        //    Send Busy Flag

//Serial RX Registers

wire                RXDataReady;                           //    Received Data Flag
wire   [7:0]        RXData;                                //    Data Received
reg    [3:0]        RXCount        =    0;                 //    Received Byte Count
reg    [7:0]        RXMS           =    0;                 //    Reception Core State

reg    [7:0]        CMD            =    0;                 //    Command received
reg    [7:0]        BYTE0          =    0;                 //    First Byte
reg    [7:0]        BYTE1          =    0;                 //    Second Byte
reg    [7:0]        BYTE2          =    0;                 //    Third Byte

// Receive State Machine Core
/*
The communication is based on 4 bytes

BYTE0        BYTE1        BYTE2        BYTE3
 CMD         DATA1        DATA2        DATA3
 
Comandos:

0x00    (X,X,X)         -    Read firmware version
0x01    (D0,X,X)        -    Resets PWM D0
0x02    (X,X,X)         -    Reads Frequencymeter value.
0x03    (D0,X,X)        -    Configure Output Protector
0x04    (00,RS,D)       -    Writes to LCD Buffer (TODO)
0x04    (01,X,X)        -    Start Writting to LCD (TODO)
0x04    (02,X,X)        -    Read if LCD Buffer is busy (TODO)
0xAX    (X,X,X)         -    Start          PWM X
0xBX    (X,X,X)         -    Stop           PWM X
0xCX    (D0,D1,D2)      -    Configure      PWM X with period defined by D0-D1 and tOn as D2 
0xDX    (X,X,X)         -    Turn On        LED X (0-4)
0xEX    (X,X,X)         -    Turn Off       LED X    (0-4)
0xFX    (D0,X,X)        -    Turn On        LED X (0-4) width brightness by D0 (0 is off, 255 is full on)

Machine states:
0x00    -    Waiting for Data
0x01    -    Executing Command
*/

// Send State Machine Core
/*

Comunication:
BYTE0        BYTE1        BYTE2        BYTE3        ...    BYTEn
SIZE         CMD         D0         D1                  Dn-2
*/

reg    [10:0]    SerialTimeout           =    0;
reg              SerialTimeoutReset      =    0;
reg              ResetSerialReset        =    0;
reg              doReset                 =    0;
wire             ResetAll                =    doReset | Reset | ( SerialTimeout == 2047 & ~RXDataReady );
reg              outprotreset            =    0;
reg    [7:0]     maxtOn                  =    30;

always @(posedge clk1us)
begin
        if(SerialTimeoutReset)
        begin
            SerialTimeout        <=    0;
            ResetSerialReset     <=    1;
        end
        else
        begin
            SerialTimeout    <=    SerialTimeout + 1;
            ResetSerialReset     <=    0;
        end
end

always @(posedge clk)
begin
        if(ResetSerialReset)
                SerialTimeoutReset <= 0;
        if(ResetAll)
        begin
            CMD             <=    0;
            BYTE0           <=    0;
            BYTE1           <=    0;
            BYTE2           <=    0;
            RXMS            <=    0;
            RXCount         <=    0;
            outprotreset    <=    1;
        end
        else
        begin
            outprotreset    <=    0;
        end
        
        //Send Machine
        if(TXBusy)    //    Send Initialized
        begin
            if(~TXTransmitting)    //    Checks if the TX isnt sending anything
            begin
                if(~TXSend)
                begin
                    if(TXByteCount > 0)    //    Still has bytes to send
                    begin
                            TXSend             <=    1;
                            TXSendBuffer       <=    TXData[TXByteSent];
                            TXByteCount        <=    TXByteCount -1;
                            TXByteSent         <=    TXByteSent + 1;
                    end
                    else
                    begin    //    No more bytes to send
                            TXBusy        <=    0;
                            TXByteSent    <=    0;
                            TXSend        <=    0;
                    end
                end
            end
            else
                TXSend    <=    0;
        end
        
        //Receive Machine
        case(RXMS)
            0:
            begin
                //Data Capture
                if(RXDataReady)
                begin
                    case(RXCount)
                        0:    CMD      <=    RXData;
                        1:    BYTE0    <=    RXData;
                        2:    BYTE1    <=    RXData;
                        3:    
                        begin
                            BYTE2    <=    RXData;
                            RXMS     <=    1;
                        end
                    endcase    
                    if(RXCount == 3)
                        RXCount <= 0;
                    else
                        RXCount <= RXCount +1;    
                    SerialTimeoutReset <= 1;
                end
            end
            1:
            begin
                // Data Processing
                casex (CMD)
                    0:                //    Sends the firmware version
                    begin
                    //0x04 0x00 FirmwareVersion FirmwareYear FirmwareMonth
                        if(~TXBusy)
                        begin
                            TXData[0]       <=    4;
                            TXData[1]       <=    0;
                            TXData[2]       <=    FirmwareVersion;
                            TXData[3]       <=    FirmwareYear;
                            TXData[4]       <=    FirmwareMonth;
                            TXByteCount     <=    5;
                            TXBusy          <=    1;
                            
                            CMD             <=    0;
                            BYTE0           <=    0;
                            BYTE1           <=    0;
                            BYTE2           <=    0;
                            RXMS            <=    0;
                            RXCount         <=    0;
                        end
                    end
                    1:            //Resets PWM BYTE0
                    begin
                            ResetPWM[BYTE0] <=    0;
                            CMD             <=    0;
                            BYTE0           <=    0;
                            BYTE1           <=    0;
                            BYTE2           <=    0;
                            RXMS            <=    0;
                            RXCount         <=    0;
                    end
                    2:            //    Read Frequencymeter State
                    begin
                            TXData[0]       <=    4;
                            TXData[1]       <=    1;
                            TXData[2]       <=    FrequencyCounter[23:16];
                            TXData[3]       <=    FrequencyCounter[15:8];
                            TXData[4]       <=    FrequencyCounter[7:0];
                            TXByteCount     <=    5;
                            TXBusy          <=    1;
                            
                            CMD             <=    0;
                            BYTE0           <=    0;
                            BYTE1           <=    0;
                            BYTE2           <=    0;
                            RXMS            <=    0;
                            RXCount         <=    0;                    
                    end
                    3:
                    begin        //    Configure Output Protector with tOn Max as BYTE0
                            maxtOn          <=    BYTE0;
                            CMD             <=    0;
                            BYTE0           <=    0;
                            BYTE1           <=    0;
                            BYTE2           <=    0;
                            RXMS            <=    0;
                            RXCount         <=    0;    
                    end
                    4:              //    LCD Control: TODO
                    begin
                                  
                    end
                    8'hAx:          //    Start PWM
                    begin
                            EnablePWM[CMD[3:0]] <=    1;
                            ResetPWM[CMD[3:0]]  <=    1;
                            CMD                 <=    0;
                            BYTE0               <=    0;
                            BYTE1               <=    0;
                            BYTE2               <=    0;
                            RXMS                <=    0;
                            RXCount             <=    0;
                    end
                    8'hBx:          //    Stop PWM
                    begin
                            EnablePWM[CMD[3:0]] <=    0;
                            CMD                 <=    0;
                            BYTE0               <=    0;
                            BYTE1               <=    0;
                            BYTE2               <=    0;
                            RXMS                <=    0;
                            RXCount             <=    0;        
                    end
                    8'hCx:          //    Configure PWM X
                    begin
                            case(CMD[3:0])
                                0:
                                begin
                                    period0     <=    { BYTE0, BYTE1 };
                                    tOn0        <=    BYTE2;
                                    CMD         <=    0;
                                    BYTE0       <=    0;
                                    BYTE1       <=    0;
                                    BYTE2       <=    0;
                                    RXMS        <=    0;
                                    RXCount     <=    0;
                                end
                                1:
                                begin
                                    period1    <=    { BYTE0, BYTE1 };
                                    tOn1        <=    BYTE2;
                                    CMD        <=    0;
                                    BYTE0        <=    0;
                                    BYTE1        <=    0;
                                    BYTE2        <= 0;
                                    RXMS        <=    0;
                                    RXCount    <=    0;
                                end                                
                                2:
                                begin
                                    period2     <=    { BYTE0, BYTE1 };
                                    tOn2        <=    BYTE2;
                                    CMD         <=    0;
                                    BYTE0       <=    0;
                                    BYTE1       <=    0;
                                    BYTE2       <=    0;
                                    RXMS        <=    0;
                                    RXCount     <=    0;
                                end                                
                                3:
                                begin
                                    period3     <=    { BYTE0, BYTE1 };
                                    tOn3        <=    BYTE2;
                                    CMD         <=    0;
                                    BYTE0       <=    0;
                                    BYTE1       <=    0;
                                    BYTE2       <=    0;
                                    RXMS        <=    0;
                                    RXCount     <=    0;
                                end
                            endcase
                    end
                    8'hDx:    //    Turn On LED
                    begin
                        case(CMD[3:0])
                            0:    ledpwm0    <=    255;
                            1:    ledpwm1    <=    255;
                            2:    ledpwm2    <=    255;
                            3:    ledpwm3    <=    255;
                        endcase
                        CMD          <=    0;
                        BYTE0        <=    0;
                        BYTE1        <=    0;
                        BYTE2        <=    0;
                        RXMS         <=    0;
                        RXCount      <=    0;
                    end
                    8'hEx:    //    Turn Off led
                    begin
                        case(CMD[3:0])
                            0:    ledpwm0    <=    0;
                            1:    ledpwm1    <=    0;
                            2:    ledpwm2    <=    0;
                            3:    ledpwm3    <=    0;
                        endcase
                        CMD          <=    0;
                        BYTE0        <=    0;
                        BYTE1        <=    0;
                        BYTE2        <=    0;
                        RXMS         <=    0;
                        RXCount      <=    0;
                    end
                    8'hFx:    //    Turn On Led with PWM BYTE0
                    begin
                        case(CMD[3:0])
                            0:    ledpwm0    <=    BYTE0;
                            1:    ledpwm1    <=    BYTE0;
                            2:    ledpwm2    <=    BYTE0;
                            3:    ledpwm3    <=    BYTE0;
                        endcase
                        CMD          <=    0;
                        BYTE0        <=    0;
                        BYTE1        <=    0;
                        BYTE2        <=    0;
                        RXMS         <=    0;
                        RXCount      <=    0;                            
                    end                    
                    default:
                    begin
                        CMD          <=    0;
                        BYTE0        <=    0;
                        BYTE1        <=    0;
                        BYTE2        <=    0;
                        RXMS         <=    0;
                        RXCount      <=    0;
                        ResetPWM     <=    15;
                    end
                endcase
            end
            endcase
end

// PWMs
PWMCore pwm0 (
    .clk(clk1us), 
    .period(period0), 
    .tOn(tOn0), 
    .enable(EnablePWM[0]), 
    .reset(ResetPWM[0]), 
    .pwm(SaidaPWM[0])
    );
PWMCore pwm1 (
    .clk(clk1us), 
    .period(period1), 
    .tOn(tOn1), 
    .enable(EnablePWM[1]), 
    .reset(ResetPWM[1]), 
    .pwm(SaidaPWM[1])
    );
PWMCore pwm2 (
    .clk(clk1us), 
    .period(period2), 
    .tOn(tOn2), 
    .enable(EnablePWM[2]), 
    .reset(ResetPWM[2]), 
    .pwm(SaidaPWM[2])
    );
PWMCore pwm3 (
    .clk(clk1us), 
    .period(period3), 
    .tOn(tOn3), 
    .enable(EnablePWM[3]), 
    .reset(ResetPWM[3]), 
    .pwm(SaidaPWM[3])
    );
     
LedPWM led0 (
    .clk(clk1us), 
    .value(ledpwm0), 
    .out(ledout0)
    );
LedPWM led1 (
    .clk(clk1us), 
    .value(ledpwm1), 
    .out(ledout1)
    );
LedPWM led2 (
    .clk(clk1us), 
    .value(ledpwm2), 
    .out(ledout2)
    );
LedPWM led3 (
    .clk(clk1us), 
    .value(ledpwm3), 
    .out(ledout3)
    );

FrequencyCounter fc (
    .clk(clk), 
    .freqin(clk1us), 
    .frequency(FrequencyCounter)
    );



// RX/TX Serial     
SerialRX RS232_Receiver (
    .clk(clk), 
    .RxD(SerialRX), 
    .RxD_data_ready(RXDataReady), 
    .RxD_data(RXData)
    );

SerialTX RS232_Transmitter (
    .clk(clk), 
    .TxD_start(TXSend), 
    .TxD_data(TXSendBuffer), 
    .TxD(SerialTX),
    .TxD_busy(TXTransmitting)
    );
     
OutputProtector OutputProt (
    .clk(clk1us), 
    .signal(SaidaPWM[0] | SaidaPWM[1] | SaidaPWM[2] | SaidaPWM[3]), 
    .reset(~outprotreset), 
    .saida(outprot), 
    .maxtOn(maxtOn)
    );
     
assign led    =    { ledout0, ledout1, ledout2, ledout3 };
assign saida = outprot;
endmodule
