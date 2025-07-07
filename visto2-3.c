#include <msp430.h> 

void config_buttons() {
    // Configuração do botão S2 (P1.1) - decrementa
    P1DIR &= ~BIT1;     // Entrada
    P1REN |= BIT1;      // Habilita resistor
    P1OUT |= BIT1;      // Pull-up
    P1IES |= BIT1;      // Interrupção na borda de descida
    P1IFG &= ~BIT1;     // Limpa flag de interrupção
    P1IE |= BIT1;       // Habilita interrupção

    // Configuração do botão S1 (P2.1) - incrementa  
    P2DIR &= ~BIT1;     // Entrada
    P2REN |= BIT1;      // Habilita resistor
    P2OUT |= BIT1;      // Pull-up
    P2IES |= BIT1;      // Interrupção na borda de descida
    P2IFG &= ~BIT1;     // Limpa flag de interrupção
    P2IE |= BIT1;       // Habilita interrupção
}

void config_leds() {
    // LED vermelho (P1.0) - MSB
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // LED verde (P4.7) - LSB
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}

int * get_counter() {
    static int counter = 0;
    return &counter;
}

void update_counter() {
    int * counter = get_counter();
    switch (*counter) {
    case 0: // 00 - ambos apagados
        P1OUT &= ~BIT0;  // LED vermelho apagado (MSB = 0)
        P4OUT &= ~BIT7;  // LED verde apagado (LSB = 0)
        break;
    case 1: // 01 - apenas verde aceso
        P1OUT &= ~BIT0;  // LED vermelho apagado (MSB = 0)
        P4OUT |= BIT7;   // LED verde aceso (LSB = 1)
        break;
    case 2: // 10 - apenas vermelho aceso
        P1OUT |= BIT0;   // LED vermelho aceso (MSB = 1)
        P4OUT &= ~BIT7;  // LED verde apagado (LSB = 0)
        break;
    case 3: // 11 - ambos acesos
        P1OUT |= BIT0;   // LED vermelho aceso (MSB = 1)
        P4OUT |= BIT7;   // LED verde aceso (LSB = 1)
        break;
    }
}

void increment_counter() {
    int * counter = get_counter();
    *counter += 1;

    if (*counter >= 4) {
        *counter = 3;
    }

    update_counter();
}

void decrement_counter() {
    int * counter = get_counter();
    *counter -= 1;

    if (*counter < 0) {  // Corrigido: era <= -1
        *counter = 0;    // Corrigido: era = 4, agora = 3
    }

    update_counter();
}

void debounce(void){
    volatile int i = 0;
    while(i < 10000) {
        i++;
    }
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    
    config_buttons();
    config_leds();
    
    int * counter = get_counter();
    *counter = 0;
    update_counter(); // Atualiza LEDs para estado inicial

    __enable_interrupt();

    // Loop infinito - necessário para manter o programa rodando
    while(1) {
        __no_operation(); // NOP para economizar energia
    }

    return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void button_1_interrupt() {
    if (P1IFG & BIT1) {  // Corrigido: usar & ao invés de ==
        debounce();      // Debounce antes de processar
        
        // Verifica se botão ainda está pressionado após debounce
        if (!(P1IN & BIT1)) {
            decrement_counter();
        }
    }
    P1IFG &= ~BIT1;      // Limpa apenas a flag do botão específico
}

#pragma vector = PORT2_VECTOR
__interrupt void button_2_interrupt() {
    if (P2IFG & BIT1) {  // Corrigido: usar & ao invés de ==
        debounce();      // Debounce antes de processar
        
        // Verifica se botão ainda está pressionado após debounce
        if (!(P2IN & BIT1)) {
            increment_counter();
        }
    }
    P2IFG &= ~BIT1;      // Limpa apenas a flag do botão específico
}