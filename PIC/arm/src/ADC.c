#include "maindefs.h"
#include "ADC.h"
#include "debug.h"

//Initialize the ADC module

void ADC_Init() {
    TRISAbits.TRISA0 = 1; //Set RA0 as input

    ANCON0bits.PCFG0 = 0;

    ADCON0bits.VCFG0 = 0;
    ADCON0bits.VCFG1 = 0;
    ADCON0bits.CHS = 3;

    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCAL = 0;
    ADCON1bits.ACQT = 1;
    ADCON1bits.ADCS = 6;

    ADCON0bits.ADON = 1;


    /* Pic 3
    TRISA = 0x0F;       //Set RA0 - RA3 as inputs
    ADCON1 = 0x0B;      //Set AN0 - AN3 as analog inputs
    ADCON0 = 0x00;      //ADC is initially idle
    ADCON2 = 0x95;      //Set acquisition time and conversion clock
    ADCON0 |= 0x01;     //Enable the ADC
     */
}

//Get the result of the ADC conversion
unsigned int ADC_Read(unsigned char ch)
{
    if (ch > 13) return 0;      //invalid channel number
    //ADCON0 |= (ch << 2);        //set the channel number to get input from

    //Set debug pin high to show ADC acquisition and conversion has started
    DEBUG_ON(ADC_READ);
    ADCON0 |= 0x02;             //set the go bit to start conversion
    while(ADCON0bits.GO == 1);  //wait until conversion is complete
    DEBUG_OFF(ADC_READ);        //Set debug pin back to low once ADC is finished

    return ADRES;               //return the result of the conversion
}
