/*
 * File:   main.c
 * Author: Eduardo Pires
 *
 */

#include <xc.h>           //***Inclusao da biblioteca do compilador
#include <pic16f877a.h>   //***Inclusao da biblioteca do chip espec?fico
#include <stdio.h>        //***Inclusao da biblioteca standard padrao "C"
#include "lcd.h"          //***Inclusao da biblioteca do LCD
#include "dht11.h"        //***Inclusao da biblioteca do sensor temp/umidade
#include "UART.h"         //***Inclusao da biblioteca protocolo UART de comunicacao    
    
//*** Configuracoes de pre-processamento ***
#pragma config WDTE = ON       // Watchdog Timer Enable bit 
#pragma config PWRTE = ON       // Power-up Timer Enable bit ~ Reset ao ligar
#pragma config BOREN = ON       // Brown Out Reset Selection bits ~ Reset para problemas com variacao de tensao
#pragma config FOSC = HS        // Cofigura clock externo 4 ate 20Mhz

#define _XTAL_FREQ 8000000          // Define a frequencia do clock, 8Mhz neste caso
#define SETUP PORTBbits.RB0         // Chave de SETUP
#define INCREMENTAR PORTBbits.RB1   // Botao de incremento
#define DECREMENTAR PORTBbits.RB2   // Botao de decremento
#define VALVULA_OX PORTAbits.RA1    // Ventoinha para controle do fluxo de ar
#define AQUECEDOR PORTAbits.RA2     // Aquecedor para controle de temperatura
#define UMIDIFICADOR PORTAbits.RA3  // Umidificador para controle de umidade
#define DISSIPADOR PORTAbits.RA5     // Dissipador para controle de temperatura

//**Variaveis auxiliares
char tempBuffer[8] = "";                        // Buffer temperatura
char infoTempBuffer[8] = "";                    // Buffer informativo temperatura ideal
char umidadeBuffer[8] = "";                     // Buffer umidade
int contador = 0;                               // Contador para a interrupcao       
__bit enviarDadosTerminal = 0;                  // Verifica a necessidade de envio de dados para o terminal
__bit aquecedorLigado = 0;                      // Verifica o estado do aquecedor
int oxigValor = 0;                              // Controla o nivel de oxigenio
int tempValor = 0;                              // Controla a temperatura
int umidadeValor = 0;                           // Controla a umidade relativa
unsigned char temperaturaIdeal = 25;            // Controla a temperatura ideal

void read_dht11_data() 
{
    if (checkBit) {
        rhByte1 = read_dht11();     // Parte alta da umidade
        rhByte2 = read_dht11();     // Parte baixa da umidade
        tempByte1 = read_dht11();   // Parte alta da temperatura
        tempByte2 = read_dht11();   // Parte baixa da temperatura  
        sumation = read_dht11();    // Checksum
        //Verifica integridade dos dados
        if (sumation == (rhByte1 + rhByte2 + tempByte1 + tempByte2)) {
            RH = rhByte1;           // Umidade em porcentagem
            temper = tempByte1;     // Temperatura em °C
        }
        else 
        {
            Lcd_Set_Cursor(1, 1);
//            Lcd_Write_String("Erro de leitura!!");  
//            __delay_ms(1000);
        }
    }
}

void display_dht11_data() 
{
    Lcd_Clear();  // Limpa o LCD

    // Exibe a umidade
    Lcd_Set_Cursor(1, 1);
    sprintf(umidadeBuffer, "Umidade: %i%%", RH);  // Converte umidade para string
    Lcd_Write_String(umidadeBuffer);

    // Exibe a temperatura
    Lcd_Set_Cursor(2, 1);
    sprintf(tempBuffer, "Temp: %iC", temper); // Converte temperatura para string
    Lcd_Write_String(tempBuffer);
}

void definir_temperatura()
{
    Lcd_Clear();  // Limpa o LCD
    
    // Exibe info.
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Temp. ideal");
    Lcd_Set_Cursor(2, 1);
    sprintf(infoTempBuffer, "%iC", temperaturaIdeal); // Converte temperatura para string
    Lcd_Write_String(infoTempBuffer);
}

void incrementarTemperatura()
{
    if (temperaturaIdeal < 35)
    {
        temperaturaIdeal = temperaturaIdeal + 1;
    }
}

void decrementarTemperatura()
{
    if (temperaturaIdeal > 20)
    {
        temperaturaIdeal = temperaturaIdeal - 1;
    }
}
void display_terminal_data() 
{
    sprintf(umidadeBuffer, "Umidade: %i%%", RH);
    UART_Write_Text(umidadeBuffer);
    UART_Write_Text("\r\n");
    
    sprintf(tempBuffer, "Temp: %iC", temper);
    UART_Write_Text(tempBuffer);
    UART_Write_Text("\r\n\r\n");
}

void __interrupt() TrataInt(void) //chama interrupcao a cada 100ms
{    
  if (TMR1IF)  //foi a interrupcao de estouro do timer1 que chamou a int?
     {  
        PIR1bits.TMR1IF = 0; //reseta o flag da interrupcao
        TMR1H = 0x9E; //reinicia a contagem
        TMR1L = 0x58; //reinicia a contagem        
        
        //comandos pra tratar a interrupcao
        contador++;
        if (contador == 50) //cada 5s
        {
            enviarDadosTerminal = 1; //solicita o envio dos dados
            contador = 0; //zera o contador
        }
  }
  return;
}

int main()
{ 
    //*** Configuracao I/O
    TRISD = 0x00;               // Configura PORTD como saida. Local onde deve estar o LCD
    OPTION_REGbits.nRBPU = 0;   // Ativa resistores de pull-ups, portB
    TRISBbits.TRISB0 = 1;       // Pino da chave de SETUP, input
    TRISBbits.TRISB1 = 1;       // Pino do botao de incremento, input
    TRISBbits.TRISB2 = 1;       // Pino do botao de decremento, input    
    TRISCbits.TRISC2 = 1;       // Pino do DHT11, input
    TRISCbits.TRISC6 = 0;       // Pino TX terminal, output
    TRISCbits.TRISC7 = 1;       // Pino RX terminal, input
    TRISAbits.TRISA0 = 1;       // Pino do conversor AD, input 
    TRISAbits.TRISA1 = 0;       // Pino da ventoinha, output 
    TRISAbits.TRISA2 = 0;       // Pino de aquecedor, output 
    TRISAbits.TRISA3 = 0;       // Pino do umidificador, output 
    TRISAbits.TRISA5 = 0;       // Pino do dissipador, output
    
    //*** Configuracao interrupcoes
    INTCONbits.GIE = 1;     // Habilitar interrupcao
    INTCONbits.PEIE = 1;    // Habilita a int dos perifericos
    PIE1bits.TMR1IE = 1;    // Habilita int do timer 1
      
    //*** Configuracao do timer 1 
    T1CONbits.TMR1CS = 0;   // Define timer 1 como temporizador (Fosc/4)
    T1CONbits.T1CKPS0 = 1;  // Bit pra configurar pre-escaler, neste caso 1:8
    T1CONbits.T1CKPS1 = 1;  // Bit pra configurar pre-escaler, neste caso 1:8
    T1CONbits.TMR1ON = 1;   // Liga o timer
    // 8MHz/4 = 2MHz
    // 2MHz/8 = 250kHz ~ periodo = 4us 
    // para 100ms dividimos por 4us ~ 0,1s/4us = 25.000 pulsos
    // carga do valor inicial no contador (65536-25000)
    // 40536. Quando estourar contou 25000 pulsos, logo passou 0,1s   
    TMR1H = 0x9E;           // Parte mais significativa (9E58 ~ 40536)
    TMR1L = 0x58;           // Parte menos significativa (9E58 ~ 40536)

    //*** Configuracao do conversor analogico/digital
    //AN0 unica entrada analogica
    ADCON1bits.PCFG0 = 0;   // Configura as entradas analogicas
    ADCON1bits.PCFG1 = 1;   // Configura as entradas analogicas
    ADCON1bits.PCFG2 = 1;   // Configura as entradas analogicas
    ADCON1bits.PCFG3 = 1;   // Configura as entradas analogicas
    //Define o clock de conversao
    ADCON0bits.ADCS0 = 0;   // Confirmando default Fosc/2 ~ 1Mhz
    ADCON0bits.ADCS1 = 0;   // Confirmando default Fosc/2 ~ 1Mhz
    ADCON1bits.ADFM = 0;    // Dados em 8 bits
    //Inicializa registradores do AD
    ADRESL = 0x00;          // Inicializar valor analogico com 0 ~ seguranca
    ADRESH = 0x00;          // Inicializar valor analogico com 0 ~ seguranca
    ADCON0bits.ADON = 1;    // Liga AD
    //Seleciona canal de entrada 0 como entrada analogica
    ADCON0bits.CHS0 = 0;    // Configura canal 0 como entrada analogica
    ADCON0bits.CHS1 = 0;    // Configura canal 0 como entrada analogica
    ADCON0bits.CHS2 = 0;    // Configura canal 0 como entrada analogica
    
    //*** Configuracao WatchDog
        OPTION_REGbits.PSA = 1;         //define pre-escalar para WDT, por reset ja fica assim
    //configura a taxa de temporizacao do WatchDog Time(WDT)1:128 neste caso 1152ms
    OPTION_REGbits.PS0 = 0;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS2 = 1;
    CLRWDT();                           //reseta a contagem do WDT
    
    Lcd_Init();             // Inicializa o LCD
    UART_Init(9600);        //Inicializa a comunicacao com o terminal, 9600 baud rate
    VALVULA_OX = 0;         
    AQUECEDOR = 0;
    UMIDIFICADOR = 0;
    
    while(1) {  
        CLRWDT();
        dht11_init();            // Inicializa o sensor
        find_response();         // Verifica resposta do sensor
        read_dht11_data();       // Lê os dados do DHT11
                
        ADCON0bits.GO = 1;       // Conversao AD habilitada
        __delay_us(10);          // Tempo de conversao padrao    
        oxigValor = ADRESH;      // Passa valores convertidos para controle
        
        tempValor = temper;      // Passa o valor inteiro da temperatura vinda do DHT11
        umidadeValor = RH;       // Passa o valor inteiro da umidade relativa vinda do DHT11
        
        if (SETUP)              // Verifica se o botao setup esta pressionado
        {
            display_dht11_data();// Habilita os dados do DHT11 no LCD
        }
        else
        {
            definir_temperatura();
            if (!INCREMENTAR)
            {
                __delay_us(25); // Debounce
                incrementarTemperatura();
            }
            
            if (!DECREMENTAR)
            {
                __delay_us(25); // Debounce
                decrementarTemperatura();
            }
        }
        
        if (enviarDadosTerminal) // Verifica a necessidade de enviar os dados para o terminal
        {
            display_terminal_data();
            enviarDadosTerminal = 0;
        }
        
        if (oxigValor <= 64)        //~25% de 256, concentracao de Oxigenio
        {
            VALVULA_OX = 1;         // Libera a valvula de oxigenio
        }
        else if (oxigValor >= 75)   // nivel de oxigenio ~30%, ideal
        {
            VALVULA_OX = 0;         // Para a valvula de oxigenio
        }
        
        if (tempValor <= temperaturaIdeal - 3) // Se a temperatura estiver abaixo do valor desejado
        {
            AQUECEDOR = 1;
            aquecedorLigado = 1;
        }
        else if (tempValor == temperaturaIdeal)   // Se a temperatura for estabilizada
        {
            AQUECEDOR = 0;
            aquecedorLigado = 0;
        }
        
        if (umidadeValor < 45)      // Se a umidade estiver abaixo do ideal  
        {
            UMIDIFICADOR = 1;
        }
        else if (umidadeValor >= 55 && umidadeValor <= 60) // Se a umidade estiver ideal
        {
            UMIDIFICADOR = 0;
            if (!aquecedorLigado)
            {
                AQUECEDOR = 0;
            }
        }
        else if (umidadeValor > 60)
        {
            AQUECEDOR = 1;
        }
        
        if (tempValor >= temperaturaIdeal + 3)
        {
            DISSIPADOR = 1;
        }
        else if (tempValor == temperaturaIdeal)
        {
            DISSIPADOR = 0;
        }
        __delay_ms(500);           // Atualiza os dados a cada meio segundo
     }
    return 0;
}
