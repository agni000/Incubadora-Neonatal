#define _XTAL_FREQ 8000000

// Função para inicializar a UART
void UART_Init(long baud_rate) {
    SPBRG = (_XTAL_FREQ/(64 * baud_rate)) - 1;  // Configura baud rate
    TXSTAbits.BRGH = 0;                         // Baud rate em baixa velocidade
    TXSTAbits.TXEN = 1;                         // Habilita transmissão
    RCSTAbits.SPEN = 1;                         // Habilita módulo UART
}

// Função para enviar um caractere pela UART
void UART_Write(char data) {
    while (!TXSTAbits.TRMT);   // Aguarda buffer estar vazio
    TXREG = data;              // Envia o caractere
}

// Função para enviar uma string pela UART
void UART_Write_Text(char *text) {
    while (*text) {
        UART_Write(*text++);
    }
}
