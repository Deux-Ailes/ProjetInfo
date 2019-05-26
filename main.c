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
//Potentiometer en PA0, convertir analogique en numérique

//Config
ADMUX=0x00; //ADLAR à 0 et tension de ref à 0 et ADC0 en entrée
ADCSRA|=(1<<ADPS2); //Prédiviseur à 16 pour 500Khz de freq de conversion, la plus rapide possible
ADCSRA|=(1<<ADEN); //Alimentation activée

//Lancement de la conversion
ADCSRA |= (1<<ADSC); //Activation de la conversion
while(!(ADCSRA & (1<<ADIF))); //Tant que le flag de fin de conversion n'est pas activé, ne rien faire
ADCSRA|=(1<<ADIF); //Clean du flag en lui appliquan un 1
return (ADCL); //Retour du résultat avec les 8 bits MSB
}

