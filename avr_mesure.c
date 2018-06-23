#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>

#include "avr_mesure.h"
#include "usb_serial.h"

volatile _Bool adc_startconv = 0;

ISR(TIMER1_COMPA_vect)
{
    adc_startconv = 1;
    PORTD ^= _BV(PD6); // Toggle LED
}

/* Need for an empty ISR since we only want Noise reduction mode */
ISR(ADC_vect)
{}

int main (void)
{
    uint8_t adc_channel = 0;
    uint16_t adc_data[4] = {0, 0, 0, 0};
    unsigned char outbuf[] = "0000,0000,0000,0000\n";

    CPU_PRESCALE(CPU_125kHz);
    _delay_ms(1);        // allow slow power supply startup
    CPU_PRESCALE(CPU_16MHz); // set for 16 MHz clock

    // Initialize USB
    usb_init();
    while (!usb_configured()) /* wait */ ;
    _delay_ms(1000);

    // DEBUG outputs
    DDRD = _BV(PD5) | _BV(PD6);

    // ADC configuration
    set_sleep_mode(SLEEP_MODE_ADC); // Sleeping starts ADC conversion
    ADMUX = _BV(REFS0) | _BV(REFS1); // Internal 2.56V reference, select channel 0
    ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // division factor 128. ADC frequency = 16 MHz / 128 = 125 kHz
    ADCSRA |= _BV(ADEN) | _BV(ADIE); // enable ADC and Interrupt

    // Timer 1 configuration
    OCR1A = 7999; // clock 16 MHz, timer 2000 Hz
    TIMSK1 = _BV(OCIE1A); // Enable CTC interrupt
    TCCR1B = _BV(CS10) | _BV(WGM12); // CTC mode, no prescaler

    sei();

    for (;;)
    {
        if(adc_startconv)
        {
            adc_startconv = 0;

            //ADCSRA |= _BV(ADSC);
            sleep_enable();
            do
            {
                sei();
                sleep_cpu();
                cli(); // ensure the following comparision runs uninterrupted
            } while(ADCSRA & _BV(ADSC));
            sleep_disable();
            sei();

            adc_data[adc_channel] = ADCW;

            adc_channel++;
            if(adc_channel >= 4)
                adc_channel = 0;

            // set the channel for the next iteration
            ADMUX = _BV(REFS0) | _BV(REFS1) | adc_channel;

            // if adc_channel is 0, we got our 4 values, time to output
            if(adc_channel == 0)
            {
                snprintf(outbuf, sizeof(outbuf), "%04d,%04d,%04d,%04d\n", adc_data[0], adc_data[1], adc_data[2], adc_data[3]);
                usb_serial_write(outbuf, sizeof(outbuf) - 1);
                PORTD ^= _BV(PD5);
            }
        }
    }
}
