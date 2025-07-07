#include <msp430.h>
#include <stdint.h>

// Endereço I2C do LCD
#define LCD_ADDR 0x27

// Máscaras de controle do PCF8574
#define LCD_RS  BIT0  // Register Select
#define LCD_RW  BIT1  // Read/Write (não usado, sempre 0)
#define LCD_EN  BIT2  // Enable
#define LCD_BL  BIT3  // Backlight (sempre ligado)

// Função que envia 1 byte para o escravo I2C
uint8_t i2cSend(uint8_t addr, uint8_t data) {
    while (UCB0CTL1 & UCTXSTP); // Espera STOP anterior
    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT; // Modo transmissor + START
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data;
    while (!(UCB0IFG & UCTXIFG));
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    return 1;
}

// Envia meio byte (nibble) para o LCD
void lcdWriteNibble(uint8_t nibble, uint8_t isChar) {
    uint8_t data = (nibble & 0xF0); // Alinha nibble nos bits D4–D7

    data |= LCD_BL; // Backlight sempre ligado
    if (isChar) data |= LCD_RS;

    // Pulso no pino EN para ativar envio
    i2cSend(LCD_ADDR, data);
    i2cSend(LCD_ADDR, data | LCD_EN);
    __delay_cycles(2000); // Delay para garantir pulso
    i2cSend(LCD_ADDR, data);
}

// Envia um byte inteiro para o LCD (2 nibbles)
void lcdWriteByte(uint8_t byte, uint8_t isChar) {
    lcdWriteNibble(byte & 0xF0, isChar);              // Parte alta
    lcdWriteNibble((byte << 4) & 0xF0, isChar);       // Parte baixa
}

// Inicializa o LCD no modo 4 bits
void lcdInit() {
    __delay_cycles(50000); // Aguarda LCD iniciar

    lcdWriteNibble(0x30, 0); // Modo 8 bits
    __delay_cycles(5000);
    lcdWriteNibble(0x30, 0); // Reforça modo 8 bits
    __delay_cycles(5000);
    lcdWriteNibble(0x30, 0);
    __delay_cycles(5000);
    lcdWriteNibble(0x20, 0); // Modo 4 bits

    // Envia comandos de configuração
    lcdWriteByte(0x28, 0); // 2 linhas, fonte 5x8
    lcdWriteByte(0x0C, 0); // Display on, cursor off
    lcdWriteByte(0x06, 0); // Incrementa cursor
    lcdWriteByte(0x01, 0); // Limpa display
    __delay_cycles(5000);  // Tempo de espera após limpar
}

// Escreve string no display
void lcdWrite(char *str) {
    while (*str) {
        if (*str == '\n') {
            lcdWriteByte(0xC0, 0); // Início da 2ª linha
        } else {
            lcdWriteByte(*str, 1); // Envia caractere
        }
        str++;
    }
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Desliga watchdog

    // Configura I2C
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;   // Modo mestre I2C síncrono
    UCB0CTL1 |= UCSSEL__SMCLK;              // Clock SMCLK
    UCB0BRW = 100;                          // Clock I2C
    P3SEL |= BIT0 | BIT1;                   // P3.0 = SDA, P3.1 = SCL
    P3DIR &= ~(BIT0 | BIT1);
    UCB0CTL1 &= ~UCSWRST;                   // Libera o módulo

    __delay_cycles(100000); // Pequeno delay antes da inicialização
    lcdInit();

    lcdWrite("Vanessa e Ana");

    while (1);
}