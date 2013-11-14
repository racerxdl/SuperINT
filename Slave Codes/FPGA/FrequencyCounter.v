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
module FrequencyCounter(
    input clk,
    input freqin,
    output [23:0] frequency
    );

parameter        SecondCount    =    8001800;          //    Number of cycles of clk to 1 second

reg    [23:0]    counter        =    0;                //    Counter for input freqin
reg    [23:0]    freq           =    0;                //    Last frequency value
reg    [23:0]    secondcounter  =    0;                //    Counter to one second
reg              stopin         =    0;                //    Stop Input Counter
reg              inreseted      =    0;                //    Reset Input Counter

always @(posedge clk)    
begin
        if(secondcounter == SecondCount)
        begin
            secondcounter     <= 0;
            stopin             <= 1;
            freq                 <=    counter*2;
        end
        else 
            if(~stopin)
                secondcounter <= secondcounter + 1;
        
        if(inreseted)
            stopin <= 0;
end

always @(negedge freqin)
begin
        if(~stopin)
        begin
            counter    <=    counter + 1;
            inreseted <= 0;
        end
        else
        begin
            counter <= 0;
            inreseted <= 1;
        end
end

assign frequency = freq;

endmodule
