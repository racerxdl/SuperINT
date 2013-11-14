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

module PWMCore(
    input clk,
    input [15:0] period,
    input [7:0] tOn,
    input enable,
    input reset,
    output pwm
    );
//1us de clock input

//parameter tOnMax = 200;
//parameter periodMin = 2000;

reg [15:0] pwmcounter = 0;
wire rst = ~reset;

reg saida = 0;

always @(posedge clk)
begin
        if(enable & ~rst ) 
        begin
                if(pwmcounter <= tOn)
                    saida <= 1;
                else
                    saida <= 0;
                if(pwmcounter == period)
                    pwmcounter <= 0;
                else
                    pwmcounter <= pwmcounter +1;
        end
        else
            saida <= 0;
        if(rst)
        begin
            pwmcounter <= 0;
            saida <= 0;
        end
end

assign pwm = saida;

endmodule
