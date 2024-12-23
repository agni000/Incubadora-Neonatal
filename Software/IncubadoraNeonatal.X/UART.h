#define _XTAL_FREQ 8000000

// Fun��o para inicializar a UART
void UART_Init(long baud_rate) {
    SPBRG = (_XTAL_FREQ/(64 * baud_rate)) - 1;  // Configura baud rate
    TXSTAbits.BRGH = 0;                         // Baud rate em baixa velocidade
    TXSTAbits.TXEN = 1;                         // Habilita transmiss�o
    RCSTAbits.SPEN = 1;                         // Habilita m�dulo UART
}

// Fun��o para enviar um caractere pela UART
void UART_Write(char data) {
    while (!TXSTAbits.TRMT);   // Aguarda buffer estar vazio
    TXREG = data;              // Envia o caractere
}

// Fun��o para enviar uma string pela UART
void UART_Write_Text(char *text) {
    while (*text) {
        UART_Write(*text++);
    }
}
