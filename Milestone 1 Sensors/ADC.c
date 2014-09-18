#include "maindefs.h"
#include "ADC.h"

//Initialize the ADC module
void ADC_Init()
{
    TRISA = 0x0F;       //Set RA0 - RA3 as inputs
    ADCON1 = 0x0B;      //Set AN0 - AN3 as analog inputs
    ADCON0 = 0x00;      //ADC is initially idle
    ADCON2 = 0x95;      //Set acquisition time and conversion clock
    ADCON0 |= 0x01;     //Enable the ADC
}

//Get the result of the ADC conversion
unsigned int ADC_Read(unsigned char ch)
{
    if (ch > 13) return 0;      //invalid channel number

    ADCON0 |= (ch << 2);        //set the channel number to get input from
    ADCON0 |= 0x02;             //set the go bit to start conversion
    while(ADCON0bits.GO == 1);  //wait until conversion is complete
    //TRISB = 0x00;
    //PORTB = ADRESL;
    return ADRES;               //return the result of the conversion
}
