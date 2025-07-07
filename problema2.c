#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <msp430.h>

#define LOW_PULSE_THRESHOLD 1700
#define HIGH_PULSE_THRESHOLD 3000
#define TOTAL_BITS 32
#define BYTE_SIZE 8

const char hex_lookup[] = "0123456789ABCDEF";

volatile char raw_ir_data[TOTAL_BITS];
char device_address[BYTE_SIZE];
char device_addr_hex[3];
char command_code[BYTE_SIZE];
char command_hex[3];
char address_complement[BYTE_SIZE];
char addr_comp_hex[3];
char command_complement[BYTE_SIZE];
char cmd_comp_hex[3];

volatile unsigned int bit_counter = 0;
volatile int data_ready_flag = 0;
unsigned int loop_index;

typedef struct {
    const char* button_name;
    const char* hex_code;
} button_mapping_t;

static const button_mapping_t remote_buttons[] = {
    {"1", "A2"}, {"2", "62"}, {"3", "E2"}, {"4", "22"},
    {"5", "02"}, {"6", "C2"}, {"7", "E0"}, {"8", "A8"},
    {"9", "90"}, {"0", "98"}, {"*", "68"}, {"#", "B0"},
    {"^", "18"}, {"V", "4A"}, {"<", "10"}, {">", "5A"},
    {"OK", "38"}
};

bool check_button_press(const char* button_id) {
    for (int i = 0; i < sizeof(remote_buttons)/sizeof(remote_buttons[0]); i++) {
        if (strcmp(button_id, remote_buttons[i].button_name) == 0) {
            return memcmp(command_hex, remote_buttons[i].hex_code, 2) == 0;
        }
    }
    return false;
}

bool convert_bits_to_hex(const char binary_data[BYTE_SIZE], char hex_output[3]) {
    if (!binary_data || !hex_output) {
        return false;
    }
    
    uint8_t result_byte = 0;
    
    for (int bit_pos = 0; bit_pos < BYTE_SIZE; bit_pos++) {
        if (binary_data[bit_pos] == 'U') {
            result_byte |= (1 << (BYTE_SIZE - 1 - bit_pos));
        } else if (binary_data[bit_pos] != 'Z') {
            hex_output[0] = '\0';
            return false;
        }
    }
    
    hex_output[0] = hex_lookup[result_byte >> 4];
    hex_output[1] = hex_lookup[result_byte & 0x0F];
    hex_output[2] = '\0';
    
    return true;
}

void decode_ir_signal() {
    for (loop_index = 0; loop_index < BYTE_SIZE; ++loop_index) {
        device_address[loop_index] = raw_ir_data[loop_index];
        address_complement[loop_index] = raw_ir_data[loop_index + BYTE_SIZE];
        command_code[loop_index] = raw_ir_data[loop_index + (BYTE_SIZE * 2)];
        command_complement[loop_index] = raw_ir_data[loop_index + (BYTE_SIZE * 3)];
    }

    convert_bits_to_hex(device_address, device_addr_hex);
    convert_bits_to_hex(command_code, command_hex);
    convert_bits_to_hex(address_complement, addr_comp_hex);
    convert_bits_to_hex(command_complement, cmd_comp_hex);
}

void configure_hardware() {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= BIT0;
    P1REN &= ~BIT0;
    P1OUT |= BIT0;

    P4DIR |= BIT7;
    P4REN &= ~BIT7;
    P4OUT |= BIT7;

    P2DIR &= ~BIT0;
    P2SEL |= BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;

    TA1CTL = 0;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
    TA1CTL = TASSEL_2 | MC_2 | ID_0 | TACLR;
    TA1CCTL1 = CM_2 | CCIS_0 | SCS | CAP | CCIE;
}

void control_leds(const char* pressed_button) {
    if (check_button_press("0")) {
        P4OUT &= ~BIT7;
        P1OUT &= ~BIT0;
    } else if (check_button_press("1")) {
        P4OUT |= BIT7;
        P1OUT &= ~BIT0;
    } else if (check_button_press("2")) {
        P4OUT &= ~BIT7;
        P1OUT |= BIT0;
    } else if (check_button_press("3")) {
        P4OUT |= BIT7;
        P1OUT |= BIT0;
    } else if (check_button_press("4") || check_button_press("5") || 
               check_button_press("6") || check_button_press("7") || 
               check_button_press("8") || check_button_press("9") || 
               check_button_press("<") || check_button_press(">") || 
               check_button_press("^") || check_button_press("V") || 
               check_button_press("OK")) {
        P4OUT ^= BIT7;
        P1OUT ^= BIT0;
    }
}

void main(void) {
    configure_hardware();
    
    P4OUT &= ~BIT7;
    P1OUT &= ~BIT0;

    __delay_cycles(500000);
    __enable_interrupt();

    while (1) {
        if (data_ready_flag) {
            decode_ir_signal();
            control_leds(NULL);
            
            data_ready_flag = 0;
            bit_counter = 0;
            TA1CCTL1 &= ~CCIFG;
            TA1CCTL1 |= CCIE;
        }
    }
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer_a1_interrupt_handler(void) {
    switch (__even_in_range(TA1IV, TA1IV_TAIFG)) {
        case TA1IV_TACCR1:
            if (TA1CCR1 < LOW_PULSE_THRESHOLD) {
                raw_ir_data[bit_counter++] = 'Z';
            } else if (TA1CCR1 < HIGH_PULSE_THRESHOLD) {
                raw_ir_data[bit_counter++] = 'U';
            } else {
                bit_counter = 0;
            }
            TA1CTL |= TACLR;
            break;

        case TA1IV_TAIFG:
            TA1CTL |= TACLR;
            break;
    }
    
    TA1CCTL1 &= ~CCIFG;
    
    if (bit_counter >= TOTAL_BITS) {
        TA1CCTL1 &= ~CCIE;
        data_ready_flag = 1;
    }
}