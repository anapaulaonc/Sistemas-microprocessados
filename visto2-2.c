#include <msp430.h>
#include <stdint.h>

volatile uint16_t step;
volatile uint16_t duty;
volatile uint16_t period;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    // LED1 (vermelho) - conectado ao timer
    P2DIR |= BIT0;
    P2SEL |= BIT0;

    // S2
    P1DIR &= ~BIT1;
    P1OUT |= BIT1;
    P1REN |= BIT1;

    // S1
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    // Configuração para 128Hz
    period = 12500;
    duty = period / 2;  // 50% duty cycle inicial
    step = period / 8;  // 12.5% 

    TA1CCR0 = period - 1;
    TA1CCTL1 = OUTMOD_7;
    TA1CCR1 = duty;
    TA1CTL = TASSEL_2 | MC_1 | TACLR;  // SMCLK, modo UP, CLEAR timer

    while (1)
    {
        while (!(P1IN & BIT1) || !(P2IN & BIT1)) {
            
        }

        if (!(P1IN & BIT1)) {  // S2 pressionado
            if (duty + step < period) {
                duty += step;
            }
            TA1CCR1 = duty;
            __delay_cycles(50000); // Debounce
        }

        if (!(P2IN & BIT1)) {  // S1 pressionado - diminui duty cycle
            if (duty >= step) {
                duty -= step;
            }
            else {
                duty = 0;
            }
        TA1CCR1 = duty;
        __delay_cycles(50000);
        }
    }
}