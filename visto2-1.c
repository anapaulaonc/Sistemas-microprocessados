#include <msp430.h>

void debounce() {
    volatile unsigned int i;
    for (i = 0; i < 30000; i++);
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    
    P1DIR |= BIT0;
    
    P1DIR &= ~BIT3;
    P1REN |= BIT3;
    P1OUT |= BIT3;
    
    unsigned char estadoAnterior = 0;
    
    while(1) {
        if (!(P1IN & BIT3)) {
            if (estadoAnterior == 0) {
                P1OUT ^= BIT0;
                estadoAnterior = 1;
                debounce();
            }
        } else {
            if (estadoAnterior == 1) {
                debounce();
                estadoAnterior = 0;
            }
        }
    }
    
    return 0;
}