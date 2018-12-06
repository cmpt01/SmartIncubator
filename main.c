
/**
 *  @file    main.c
 *  @author  Kevin Andoni, Engin Bajrami
 *  @date    06,12, 2018
 *  @brief   A smart incubator that consists of a Warming Lamp, Relay Switch, DHT12 and an MCU
 */


#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "twi.h"
#include "uart.h"


#define HATCHING_TEMP 38
#define TOLERANCE 2
#define UART_BAUD_RATE 9600
#define DHT12_ADDRESS 0x5c
#define UPPER_LIMIT 40
#define DOWN_LIMIT 36

struct values{
    uint8_t humidity_integer;
    uint8_t humidity_decimal;
    uint8_t temperature_integer;
    uint8_t temperature_decimal;
};
struct values DHT12_values;


void setup(void);

void fsm_twi_scanner(void);

typedef enum {
    IDLE_STATE = 1,
    HUMIDITY_STATE,
    TEMPERATURE_STATE,
    UART_STATE,

} state_t;

state_t twi_state = IDLE_STATE;

int main(void){
    setup();

    sei();

    while (1) {

       }
    return 0;
}

void setup(void){

    DDRB |=_BV(PB5);

    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));

    twi_init();

    /* Timer/Counter1: update FSM state */
    /* Clock prescaler 64 => overflows every 262 ms */
    TCCR1B |= _BV(CS11) | _BV(CS10);
    /* Overflow interrupt enable */
    TIMSK1 |= _BV(TOIE1);
}


ISR(TIMER1_OVF_vect)
{
    fsm_twi_scanner();
}


void fsm_twi_scanner(void)
{

    uint8_t twi_status;

    switch (twi_state) {

        case IDLE_STATE:
            twi_state = HUMIDITY_STATE;
            break;

        case HUMIDITY_STATE:
            twi_status = twi_start((DHT12_ADDRESS<<1) + TWI_WRITE);

            if (twi_status==0){

                twi_write (0x00);
                twi_stop ();
                twi_start((DHT12_ADDRESS<<1) + TWI_READ);
                DHT12_values.humidity_integer = twi_read_ack();
                DHT12_values.humidity_decimal = twi_read_nack();
                twi_stop ();
                twi_state = TEMPERATURE_STATE;
            }
            else{
                twi_state = IDLE_STATE;
            }
            break;

        case TEMPERATURE_STATE:

              twi_status = twi_start((DHT12_ADDRESS<<1) + TWI_WRITE);
              if (twi_status==0){
              twi_write (0x02);
              twi_stop ();
              twi_start((DHT12_ADDRESS<<1) + TWI_READ);
              DHT12_values.temperature_integer = twi_read_ack();
              DHT12_values.temperature_decimal = twi_read_nack();
              twi_stop ();
              twi_state = UART_STATE;
              }
              else{
                  twi_state = IDLE_STATE;
              }
              break;

        case UART_STATE:

            if((DHT12_values.temperature_integer < UPPER_LIMIT)&&(DHT12_values.temperature_integer > DOWN_LIMIT)){
                PORTB |= _BV(PB5);
              }
            else{
                PORTB &= ~_BV(PB5);
              }

            twi_state = IDLE_STATE;
            break;

        default:
            twi_state = IDLE_STATE;
        }

}
/* END OF FILE ****************************************************************/
