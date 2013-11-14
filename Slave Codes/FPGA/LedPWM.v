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
module LedPWM(
    input clk,
    input [7:0] value,
    output out
    );

reg [7:0]    PWMCounter;
reg            outreg;
always @(posedge clk)
begin
        if(PWMCounter <= value & value != 0)
            outreg <= 1;
        else
            outreg <= 0;
            
        PWMCounter <= PWMCounter+1;
end

assign out = outreg;
endmodule
