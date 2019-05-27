/*
 * GccApplication1.c
 *
 * Created: 20/05/2019 08:19:50
 * Author : Etudiant
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/iom32.h>
#include <avr/interrupt.h>


//_______________________________________DEBUT PROTOTYPES_______________________________________________________________________________________//

//Proto Option 1

//Fonctions élémentaires Option 1
unsigned char Lecture_Vitesse (void);
unsigned char Ecriture_vitesse(unsigned char  vitesse);
void  Affichage_vitesse(unsigned char  vitesse_pourcent);

//Fonctions élémentaires Option 2
void decompte(void);
void chrono (void);
void InitTimerInterrupt(void);

//Fonctions élémentaires Option 3
unsigned char lecture_capteur_fourche(void);
void ecriture_nb_tour(void);

//Fonctions secondaires reprises du cours et modifiées/créees/améliorées
void InitUSART(void);
char putCharToLCD(char codeASCII);
void InitLCD(void);
int sendOneChar( char codeASCII, FILE *unused);
void AffPrecis(unsigned char colonne, unsigned char ligne);
FILE lcd_connexion = FDEV_SETUP_STREAM(sendOneChar, NULL, _FDEV_SETUP_WRITE);

volatile unsigned int	centi;
volatile unsigned char	seconde ;
volatile unsigned char	minute ;

int main(void)
{
	cli();
	
	stdout = &lcd_connexion;
	InitUSART();
	InitLCD();
	InitTimerInterrupt();
	//Déclaration des ports 
	//En output : PD2 à PD7 , PC0, PC2 à PC5 et PB7
	//En input : PA0 à PA7, PC1,6,7, PB0 à PB6, PD0,1
	DDRA=0x00; PORTA=0x00;
	DDRB=0x80; PORTB=0x00;
	DDRC=0x3C; PORTC=0x00;
	DDRD=0xFC; PORTD=0x00;
	
	//Déclaration des variables
	unsigned char vitesse;
    unsigned char pourcent;
	unsigned char depart_lance = 0;
	
	
	sei();
	
	
	//_______________________________________DEBUT MAIN_______________________________________________________________________________________//
    while (1) 
    {	
		if((PINB & 1<<PB2)& (depart_lance==0))		//Si l'on n'a pas encore fait de départ lancé et qu'on appuie sur Menu
		{
			depart_lance++;
			decompte();								//Lancement du decompte
		}
		printf("%c%c",0xA3,0x01);					//Clear de l'écran et remise à zéro du curseur
		
		vitesse = Lecture_Vitesse(); 
		pourcent = Ecriture_vitesse(vitesse);
		Affichage_vitesse(pourcent);
		//chrono();
		//ecriture_nb_tour();
		InitLCD();
    }
}



//_______________________________________DEBUT FONCTIONS_______________________________________________________________________________________//


//Fonctions secondaires

void InitUSART(void)
{
	UBRRH = 0x00; UBRRL = 25;		// vitesse de transmission 19200 bits/sec
	UCSRB = 1 << TXEN;              // clear du flag
	UCSRC = 0x86;					// configurer la lecture avec charactères 8 bits, 1 bit de stop, pas de parité
}

char putCharToLCD(char codeASCII)
{
	 while (!(UCSRA &(1<<UDRE)));	// attendre fin de la conversion 
	 UDR=codeASCII;					// déposerle caractère dansl’USART
	 return(codeASCII);				// retourner le caractère envoyé
}

void InitLCD(void)
{
	putCharToLCD(0xA3);
	putCharToLCD(0x01);				//Reset écran et position
	putCharToLCD(0xA3);				
	putCharToLCD(0x0C);				//Désactivation du curseur
	_delay_ms(20);
}

int sendOneChar( char codeASCII, FILE *unused)
{
	return putCharToLCD(codeASCII);				//envoi d’un caractère au LCD
}

void AffPrecis(unsigned char colonne, unsigned char ligne)
{
	printf("%c%c%c",0xA1,colonne,ligne); //Positionnement du curseur selon la ligne et colonne souhaitées
	_delay_ms(20);
}


//Fonctions élémentaires

unsigned char Lecture_Vitesse (void)
{
	//Potentiometer en PA0, convertir analogique en numérique

	//Config
	ADMUX=(1<<REFS0);					//ADLAR à 0 et tension de ref à 5V et ADC0 en entrée
	ADCSRA|=(1<<ADPS2)|(1<<ADEN);		//Prédiviseur à 16 pour 500Khz de freq de conversion, la plus rapide possible, Alimentation activée

	//Lancement de la conversion
	ADCSRA |= (1<<ADSC);				//Activation de la conversion
	while(!(ADCSRA & (1<<ADIF)));		//Tant que le flag de fin de conversion n'est pas activé, ne rien faire
	ADCSRA|=(1<<ADIF);					//Clean du flag en lui appliquan un 1
	return ADCL;						//Retour du résultat avec les 8 bits LSB
}

unsigned char Ecriture_vitesse(unsigned char  vitesse)
{
	
	//Utilisation du mode Fast PWM qui compte jusqu'à la valeur vitesse, définie sur 255
	
	//Config
	DDRD|=(1<<PD7);
	if (vitesse > 255) vitesse = 255;					//Sécurité si la sortie venait à dépasser 255
	TCCR0|=(1<<COM01)|(1<<WGM00)|(1<<WGM01)|(1<<CS01);	//Fast PWM, Clear OCR0 en compare et se réinitialise à 0 sans inversement avec une freq de 3.9kHz au max, soit 8MHz/256 (temps de comptage)/3.9kHz=8 de prescaler
	OCR0=vitesse;										//On veut que TCCR0 soit comparé à la vitesse sur OCR0
		
	//Action
	//while (TCCR0!=1) PORTA|=(1<<PA7);					//Tant que le flag n'est pas activé, la sortie est activée
	//PORTA&=~(1<<PA7);									//On reset la sortie
	
	//Conversion de la vitesse (0-255) en valeur %
	unsigned char pourcentage;
	pourcentage = (vitesse/255)*100;
	return pourcentage;
	
}

void  Affichage_vitesse(unsigned char  vitesse_pourcent)
{
	//Opérations
	unsigned char V = 0;								//Valeur comprise entre 0 et 5 donc usage d'un uchar suffisant
	unsigned int mV = 0;								//Usage de int car la valeur peut dépasser 255 (0-999)
	//V=(vitesse_pourcent/100*5);						//On prend la valeur en V de la vitesse, si 100%, alors tension = 5V, si 50%, tension=2V
	//mV = ((vitesse_pourcent/100*5)%1)*100;			//Ici on s'occupe des mV, on prend le reste de la division par 1 de notre tension, si tension de base = 2.5V, on a un reste de 0.5, que l'on multiplie par 100 pour correspondre aux mV.
	
	//Affichage
	AffPrecis(0,0);
	//printf("%c%c",0xA3,0x01);								//Clear de l'écran
	//printf("%c %c V %i mV  %c",0xA2,V,mV,0x00);			//Utile lors des phases de test
	printf("%cVitesse :%c%%%c",0xA2,vitesse_pourcent,0x00);	//Affichage de la vitesse
	_delay_ms(50);
}

void decompte(void)
{
	
		for(int i=3; i>1; i--)					//Pour i allant de 3 à 1
		{
			AffPrecis(4,1);						//Affichage au centre de la deuxieme ligne
			printf("%c %d...%c",0xA2,i,0x00);	//3... 2... 1...
			_delay_ms(1000);					//1 seconde de delai entre chaque itération
		}
		
	AffPrecis(4,1);								//Affichage au centre de la 2eme ligne					
	printf("%cGO!!!%c",0xA2,0x00);				//GO!!!
}


void InitTimerInterrupt(void)
{
	TCCR2= (1<<CS02) | (1<<CS00) | (1<<WGM01);	//Prédiviseur à 1024 avec CTC
	OCR2 = 77;									//Nombre d'actions pour effectuer (OCR2+1)*(Tcpu*n)=0.010, donc on a OCR2=77 qui donne 0.009984s (résultat le plus proche de 0.01s, sans compter les temps de cycle pour effectuer les opérations précédentes)
	TIMSK = (1<<OCIE2);							//Autorise les interruptions
}


ISR (TIMER2_COMP_vect)
{
		
		centi++;								//Toutes les 0.01s, centi s'incrémente
		
		if (centi==100)							//Si 100*centi, alors on incrémente le compteur de secondes et reset centi
		{
			centi=0;
			seconde++;
		}
		
		if (seconde==60)						//Même procédé avec les secondes qui incrémentent les minutes
		{
			seconde=0;
			minute++;
		}
}

void chrono(void)
{
	AffPrecis(0,1);								//Affichage sur la deuxième ligne à la 5eme colonne
	printf("%c%c'%c''%c%c",0xA2,minute,seconde,centi,0x00);
}

unsigned char lecture_capteur_fourche(void)
{
	if (PINB & (1<<PB4))
	{
		return 1;
	}
	else return 0;
}

void ecriture_nb_tour(void)
{
	static char nb_tour=0;
	if(lecture_capteur_fourche())	nb_tour++;
	AffPrecis(4,1);
	printf("%c Tour %c%c",0xA2,nb_tour,0x00);	
}
