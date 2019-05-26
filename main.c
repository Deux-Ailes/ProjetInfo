/*
 * GccApplication1.c
 *
 * Created: 20/05/2019 08:19:50
 * Author : Etudiant
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>


//Proto
unsigned char Lecture_Vitesse (void);

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
		
    }
}


unsigned char Lecture_Vitesse (void){
//Potentiometer en PA0, convertir analogique en num�rique

//Config
ADMUX=0x00; //ADLAR � 0 et tension de ref � 0 et ADC0 en entr�e
ADCSRA|=(1<<ADPS2); //Pr�diviseur � 16 pour 500Khz de freq de conversion, la plus rapide possible
ADCSRA|=(1<<ADEN); //Alimentation activ�e

//Lancement de la conversion
ADCSRA |= (1<<ADSC); //Activation de la conversion
while(!(ADCSRA & (1<<ADIF))); //Tant que le flag de fin de conversion n'est pas activ�, ne rien faire
ADCSRA|=(1<<ADIF); //Clean du flag en lui appliquan un 1
return (ADCL); //Retour du r�sultat avec les 8 bits MSB
}

