#include <msp430.h>
#include <stdint.h>

#define SLAVE_ADDR 0x27  


uint8_t i2cSend(uint8_t addr, uint8_t data) {
    UCB0I2CSA = addr;

    UCB0CTL1 |= UCTR + UCTXSTT;

    while (!(UCB0IFG & UCTXIFG)); 

    UCB0TXBUF = data;

    while (UCB0CTL1 & UCTXSTT); 

    uint8_t nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;

    if (!nack) {
        while (!(UCB0IFG & UCTXIFG)); 
    }

    UCB0CTL1 |= UCTXSTP;

    while (UCB0CTL1 & UCTXSTP); 

    return nack;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;  

    UCB0CTL1 |= UCSWRST;

    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;  
    UCB0CTL1 = UCSSEL_2 + UCSWRST;        
    UCB0BRW = 100;                         

    P3SEL |= BIT0 + BIT1;

    P3REN |= BIT0 + BIT1;
    P3OUT |= BIT0 + BIT1;

´    UCB0CTL1 &= ~UCSWRST;

´    while (1) {
        i2cSend(SLAVE_ADDR, 0x08);
        __delay_cycles(1000000);  

        i2cSend(SLAVE_ADDR, 0x00);
        __delay_cycles(1000000);  
    }
}