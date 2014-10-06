#include "maindefs.h"
#include "ADC.h"
#include "debug.h"
#include <plib/adc.h>

//Initialize the ADC module
void ADC_Init()
{
//    TRISAbits.TRISA0 = 1;       //Set RA0 as input
//    TRISAbits.TRISA2 = 1;
//    TRISAbits.TRISA3 = 1;
//
//    ANCON0bits.PCFG0 = 0;
//
//    ADCON0bits.VCFG0 = 1;
//    ADCON0bits.VCFG1 = 1;
//    ADCON0bits.CHS = 3;
//
//    ADCON1bits.ADFM = 1;
//    ADCON1bits.ADCAL = 0;
//    ADCON1bits.ACQT = 1;
//    ADCON1bits.ADCS = 6;
//
//    ADCON0bits.ADON = 1;

    // Pic 3
    //TRISA = 0x0F;       //Set RA0 - RA3 as inputs
    TRISAbits.RA0 = 1;      //AN0 = input
    TRISAbits.RA1 = 1;      //AN1 = input
    TRISAbits.RA2 = 1;      //AN2 = input
    TRISAbits.RA3 = 1;      //AN3 = input
    TRISAbits.RA5 = 1;      //AN4 = input
    TRISEbits.RE0 = 1;      //AN5 = input
    TRISEbits.RE1 = 1;      //AN6 = input
    TRISEbits.RE2 = 1;      //AN7 = input
    TRISBbits.RB2 = 1;      //AN8 = input
    TRISBbits.RB3 = 1;      //AN9 = input

    ADCON1 = 0x00;      //Set AN0 - AN12 as analog inputs
    ADCON0 = 0x00;      //ADC is initially idle
    ADCON2 = 0x8C;      //Set acquisition time and conversion clock
    ADCON0 |= 0x01;     //Enable the ADC
    PIE1bits.ADIE = 1;  //Enable interrupts 
}

//Get the result of the ADC conversion
int ADC_Start(unsigned char ch)
{
    if (ch > 13) return 0;      //invalid channel number
    ADCON0 |= (ch << 2);        //set the channel number to get input from

    //clear ADIF bit
    PIR1bits.ADIF = 0;
    //set ADIE bit
    //PIE1bits.ADIE = 1;
    //set GIE bit
    //INTCONbits.GIE = 1;
    //Set debug pin high to show ADC acquisition and conversion has started
    DEBUG_ON(ADC_START);
    ADCON0 |= 0x02;             //set the go bit to start conversion
    return 1;
}
