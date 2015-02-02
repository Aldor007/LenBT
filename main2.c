
#define  F_CPU  8000000ul

#include <avr/io.h>

#include <avr/delay.h>
#include <avr/interrupt.h>





//MODUL BT.c////////////

const int  BT_UART_DELAY=  15;
#define USART_BAUD 19200ul
#define USART_UBBR_VALUE ((F_CPU/(USART_BAUD<<4))-1)


///////kamod.h//////////////
#define BT_ERR            0
#define BT_OK            1

#undef BT_USE_nRES

#ifdef BT_USE_nRES
#define BT_nRES_DIR        DDRB
#define BT_nRES_PORT     PORTB
#define BT_nRES_PIN        PB2
#endif // BT_USE_nRES

int KAmodBT_Initialize();
int KAmodBT_SetName(char * Name);
int KAmodBT_EnterATMode();
int KAmodBT_GetName(char * Name);
int KAmodBT_GetPin(char * Name);

////////////kamod.h end//////

//*******************************
// USART functions

void USART_vInit(void)

{
    UBRRH = (uint8_t)(USART_UBBR_VALUE>>8);
    UBRRL = (uint8_t)USART_UBBR_VALUE;

    UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);

    UCSRB = (1<<RXEN)|(1<<TXEN);
}


void USART_vSendByte(uint8_t u8Data)

{
    while((UCSRA&(1<<UDRE)) == 0);
    UDR = u8Data;
}


uint8_t USART_vReceiveByte()

{
    while((UCSRA&(1<<RXC)) == 0);
    return UDR;
}



// USART_vSendBuffer sends string with delays of del ms between characters
void USART_vSendBuffer(char * buffer, const int del)
{
    int i = 0;
    while(buffer[i]!=0) { USART_vSendByte(buffer[i]); _delay_ms(del); i++; }
}

// USART_vReceiveBuffer gets string with delays of del ms between characters
void USART_vReceiveBuffer(char * buffer, const int del)
{
    int i = 0;
    while (1)
    {
        buffer[i] = USART_vReceiveByte();
        if ((buffer[i]=='\r') | (buffer[i]=='\n'))
        {
            buffer[i]=0;
            return;
        }
        _delay_ms(del);
        i++;
    }
}

//************************************



//************************************
// Functions for KAmodBTM222
//************************************

// Initialization of communication interface (UART + module's reset)
int KAmodBT_Initialize()
{
#ifdef BT_USE_nRES
    // Configure nRES
    BT_nRES_DIR = BT_nRES_DIR | (1 << BT_nRES_PIN);
    BT_nRES_PORT |= (1 << BT_nRES_PIN);
#endif // BT_USE_nRES

    // Initialize USART
    USART_vInit();

    return BT_OK;
}


// Function for enabling module's AT mode and some more
int KAmodBT_EnterATMode()
{
    char buf[32];

    // Send +++ for AT mode enabling
    USART_vSendBuffer("+++\r", 50);
    USART_vReceiveBuffer(buf, BT_UART_DELAY);


    //Disable echo, it speeds up communication, because, you don't have to wait for echo
    USART_vSendBuffer("ATE0\r", 50);
    USART_vReceiveBuffer(buf, BT_UART_DELAY);

    if (strcmp(buf, "OK")==0) return BT_OK; else return BT_ERR;
}

int KAmodBT_SetName(char * Name)
{
    char buf[32];

    sprintf(buf, "ATN=%s\r", Name);
    USART_vSendBuffer(buf, BT_UART_DELAY);
    USART_vReceiveBuffer(buf, BT_UART_DELAY);

    if (strcmp(buf, "OK")==0) return BT_OK; else return BT_ERR;
}


// KAmodBT_GetName reads module's name
int KAmodBT_GetName(char * Name)
{
    char resBuffer[16];
    USART_vSendBuffer("ATN?\r", BT_UART_DELAY);
    USART_vReceiveBuffer(Name, 0);
    USART_vReceiveBuffer(Name, 0);
    USART_vReceiveBuffer(resBuffer, 0);
    return BT_OK;
}


// KAmodBT_GetPin reads PIN (that you need to connect module to another device)
int KAmodBT_GetPin(char * Name)
{
    char resBuffer[16];
    USART_vSendBuffer("ATP?\r", BT_UART_DELAY);
    USART_vReceiveBuffer(Name, 0);
    USART_vReceiveBuffer(Name, 0);
    USART_vReceiveBuffer(resBuffer, 0);
    return BT_OK;
}


// Reset module function
void KAmodBT_Reset()
{
#ifdef BT_USE_nRES
    BT_nRES_PORT &= !(1 << BT_nRES_PIN);
    _delay_ms(50);
    BT_nRES_PORT |= (1 << BT_nRES_PIN);
    _delay_ms(50);
#endif // BT_USE_nRES
}


// Sends string to module
void KAmodBT_SendBuffer(char * buffer)
{
    USART_vSendBuffer(buffer, BT_UART_DELAY);
  //USART_vSendBuffer(buffer, 0);
}

/////////kamod.c end//////////////


void onPD1(){ PORTD = 0b01000000;}         //dioda LED1 swieci PB0 = 1

void offPD1() {PORTD = 0x0;};
//DIODY//
void leds_set(){
    onPD1();

}

void leds_clr(){
    offPD1();

}
//pc0 in 3 10 pin
//pd7 in 4 1

void direction (int k)
{
    switch(k)
    {
    case 2://przod
        PORTD = 0b01110100;
        PORTC = 0b00000001;
    break;
    case 8://tyl
        PORTD = 0b11111000;
        PORTC = 0b00000000;
    break;
    case 4://lewo
        PORTD = 0b11110100;
        PORTC = 0b00000000;
    break;
    case 6://prawo
        PORTD = 0b01111000;
        PORTC = 0b00000001;
    break;
    case 5://prawo
        PORTD = 0;
        PORTC = 0;
    break;
}

}
/* from 50 to 950 */
uint16_t duty_cycle;

/* IRQ routine at 500hz this is called 500 times x second */
ISR(TIMER1_OVF_vect)
{
    /* if the duty is at 95% the restart from 5% */
    if (duty_cycle > 950)
        duty_cycle = 50;

    /* increment the duty cycle by 1 step at the time wich is
      0.1%, in this way I will complete 100% in 2 seconds (1000 step) */
    duty_cycle++;

    /* set the level where the output pin OC1A will toggle */
    OCR1A = duty_cycle;

    /* set the level where the output pin OC1B will toggle
      this will be the inverse of OC1A level */
    OCR1B = 1000 - duty_cycle;
}

void counter_setup(void)
{
        /* Clear OC1A & B on compare match, set it at BOTTOM.
      Waveform generation mode on 14, see datasheet */
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12);

    /* TOP value*/
    ICR1 = 1000;

    /* start with a duty of 50% */
    OCR1A = 300;
    OCR1B = 300;

    /* enable interrupt on timer overflow */
    TIMSK = _BV(TOIE1);
}

int main()
{
    int i=0;
    uint8_t byte;
/* prepare everything */
    counter_setup();

    /* start with a duty cycle of 50% */
    duty_cycle = 400;

    /* enable interrupt */
    sei();

    /* start the counter with prescaler = 8 */
    TCCR1B |= _BV(CS11);

	DDRC = 0b00000011;
	DDRD = 0b11111100;


    for (i=0;i<3;i++) {
        leds_set();
        _delay_ms(250);
        leds_clr();
        _delay_ms(250);
    }



    // Initialize communication
    if (KAmodBT_Initialize()!=BT_OK)
    {
        onPD1();
    }

    while (1) {

        for (i=0;i<10;i++)
       {
            if (UCSRA&(1<<RXC)) {
                byte = USART_vReceiveByte();

                switch (byte) {
                case '0':
                    leds_clr();

                    break;
                case '1':

                  leds_set();
                    break;
                case '2':

					direction(2);
                    break;
                 case '8':
                    direction(8);
                    break;

				case '4':
                    direction(4);
                    break;

				case '6':
                    direction(6);

                    break;
                    case '5':
                    direction(5);

                    break;
                default:

        leds_set();
         _delay_ms(250);
        leds_clr();


                break;
				}

            }

        }
        _delay_ms(250);
    }
	cli();
}
