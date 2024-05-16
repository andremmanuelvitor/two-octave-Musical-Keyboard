#include <stm32f10x.h>

void lcd_init(void); //Iniciar o LCD
void lcd_command(unsigned char cmd); //Envia comandos para o LCD
void lcd_print(char *str); //Envia uma String para o LCD
void lcd_data(unsigned char data); //Envia dados para o LCD em ASCII
void lcd_putValue(unsigned char value); //Função interna do LCD
void delay_ms(uint16_t t); //Gera atraso em ms
void delay_us(uint16_t t); //Gera atraso em us
void pot_ADC(void);
void set_psc(unsigned int psc);
void ciclo_trabalho(void);

#define BitTst(arg,bit) ((arg) & (1<<bit)) //Retorna o valor do 'bit' do 'arg' -> Se apertado retorna 0, se levantando retorna 1.

//Pinagem padrao LCD
#define LCD_RS 15 //PA15
#define LCD_EN 12 //P12
#define LCD4 8 		// PA8
#define LCD5 6 		// PA6
#define LCD6 5 		// PA5
#define LCD7 11 	// PA11

// Global variables
static uint16_t valor_pot_ADC;

int main() {
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //Habilita clock para I/O alternados 
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; //Desabilita interface JTAG 
	RCC->APB2ENR |= 0xFC|(1<<9)|(1<<14); //Habilita clock das portas GPIOs e pot: ADC1 clock, usart1
	RCC->APB1ENR |= (1<<1); //Habilita clock do TIMER3
	
	GPIOA->CRL = 0x43344333;	//Configura PA7/PA4/PA3 como entradas de chaves/Restante como saídas
	GPIOA->CRH = 0x33333333;	//Configura PB8:PB15 como saídas, para o LCD
	GPIOB->CRL = 0x4444440B; 	//Configura buzzer como saída alternada e os restantes como entradas para chaves e pot: PB1 = analog input
	GPIOB->CRH = 0x44444444; 	//Configura as chaves como PB8:PB15 como entradas 
	GPIOC->CRH = 0x44444444;	//Apesar do valor de reset, foi declarado como controle de entradas para chaves
	
	//Inicializa o LCD
  lcd_init();
	
  //Inicialização do PWM1	
  TIM3->CCMR2 = 0x0060;		//Configura modo PWM1
	TIM3->ARR = 36 - 1;			//Definindo ARR como 35, arbitrariamente
	
	//Inicializa o potenciômetro
	ADC1->CR2 = 1;
	ADC1->SMPR2 = 1<<27;
	delay_us(1);
	
	
	unsigned char oitava = 1; //Teclado começa na primeira oitava por padrão
	lcd_command(0x80); //Cmd -> Força o cursor na primeira posição da primeira linha
	lcd_print("Oitava: 1");	//Printa informação no LCD

	//Inicializa o ciclo de trabalho em 25%
	lcd_command(0xC0);
	lcd_print("Ciclo: 25%"); //Print a informação no LCD
	TIM3->CCR3 = 9; //36*0,25=9

	while(1) {			
		while(oitava == 1) {
			//Teste do SW2 para ter a informação da segunda oitava
			if(!BitTst(GPIOB->IDR,13)) { //TsT PB13 -> 0 = pressionado
				lcd_command(0x80);
				lcd_print("Oitava: 2");
				oitava = 2;
			}
			
			//Testa se o SW3 foi pressionado, chama a função do ciclo de trabalho
			if(!BitTst(GPIOB->IDR,14)) //TsT PB14 -> 0 = pressionado
        ciclo_trabalho();
			
			//Testa qual nota musical foi pressionada: //Enquanto a tecla está pressionada, o programa fica preso num loop
			while(!BitTst(GPIOB->IDR,5)) 
        set_psc(15150);                //C      
			while(!BitTst(GPIOB->IDR,10)) 
        set_psc(14285);                //C#     
			while(!BitTst(GPIOB->IDR,4)) 
        set_psc(13512);                //D
			while(!BitTst(GPIOA->IDR,7)) 
        set_psc(12738);                //D#
			while(!BitTst(GPIOB->IDR,3)) 
        set_psc(12047);                //E
			while(!BitTst(GPIOA->IDR,3)) 
        set_psc(11363);                //F
			while(!BitTst(GPIOC->IDR,15)) 
        set_psc(10694);                //F#
			while(!BitTst(GPIOA->IDR,4)) 
        set_psc(10100);                //G
			while(!BitTst(GPIOC->IDR,14)) 
        set_psc(9568);                 //G#
			while(!BitTst(GPIOB->IDR,8)) 
        set_psc(9008);                 //A
			while(!BitTst(GPIOC->IDR,13)) 
        set_psc(8510);                 //A#
			while(!BitTst(GPIOB->IDR,9)) 
        set_psc(8031);                 //B
			while(!BitTst(GPIOB->IDR,11)) 
        set_psc(7575);                 //C
			
			//Ao soltar a tecla e sair do loop da tecla, desliga o som
			TIM3->CCER = 0;
			TIM3->CR1 = 0;			
		}
		while(oitava == 2) {
			
			////Teste do SW1 para ter a informação da primeira oitava
			if(!BitTst(GPIOB->IDR,12)) { //TsT PB12 -> 0 = pressionado
				lcd_command(0x80);
				lcd_print("Oitava: 1");
				oitava = 1;
			}
			
			//Testa se o SW3 foi pressionado, chama a função do ciclo de trabalho
			if(!BitTst(GPIOB->IDR,14)) //TsT PB14 -> 0 = pressionado
        ciclo_trabalho();
			
			//Testa qual nota musical foi pressionada:
			while(!BitTst(GPIOB->IDR,5)) 
        set_psc(7575);                //C      
			while(!BitTst(GPIOB->IDR,10)) 
        set_psc(7142);                //C#     
			while(!BitTst(GPIOB->IDR,4)) 
        set_psc(6756);                //D
			while(!BitTst(GPIOA->IDR,7)) 
        set_psc(6368);                //D#
			while(!BitTst(GPIOB->IDR,3)) 
        set_psc(6023);                //E
			while(!BitTst(GPIOA->IDR,3)) 
        set_psc(5681);                //F
			while(!BitTst(GPIOC->IDR,15)) 
        set_psc(5346);                //F#
			while(!BitTst(GPIOA->IDR,4)) 
        set_psc(5049);                //G
			while(!BitTst(GPIOC->IDR,14)) 
        set_psc(4784);                //G#
			while(!BitTst(GPIOB->IDR,8)) 
        set_psc(4503);                //A
			while(!BitTst(GPIOC->IDR,13)) 
        set_psc(4254);                //A#
			while(!BitTst(GPIOB->IDR,9)) 
        set_psc(4015);                //B
			while(!BitTst(GPIOB->IDR,11)) 
        set_psc(3787);                //C
			//Ao soltar a tecla e sair do loop da tecla, desliga o som
			TIM3->CCER = 0;
			TIM3->CR1 = 0;
		}	
	}
}

void pot_ADC(void)
{
	ADC1->SQR3 = 9;
	ADC1->CR2 = 1;
	while((ADC1->SR&(1<<1)) == 0);
	valor_pot_ADC = ADC1->DR;          //Seta o valor do valor_pot_ADC que é uma variável global
}

void set_psc(unsigned int psc)
{						
	pot_ADC();								    		 //Chama a função pot_ADC, p/ verificar o potenciômetro
	TIM3->CCER |= (1 << 8); 					 //Configura canal 3 como ativo alto e habilita saída
	TIM3->PSC = psc-valor_pot_ADC/4;   //Seta o valor de prescaler, referente a nota e modulado também pelo valor_pot_ADC do Pot.
	TIM3->CR1 = 1;						    	   //Habilita o TIMER3
	delay_ms (200);						    		 //delay de 200ms para garantir que o som será emitido
																		 //Emitindo a frequêcia da nota pressionada.
}

void ciclo_trabalho(void)
{	
	switch(TIM3->CCR3)
	{
		case 27: 	//Se estiver em 75% -> muda para 25%
			lcd_command(0xC0); //Cmd -> Força o cursor na primeira posição da segunda linha
			lcd_print("Ciclo: 25%");
			TIM3->CCR3 = 9; //36*0,25=9
		  	delay_ms (100); //Delay evitando do debounce
			break;
		case 9: 	//Se estiver em 25%  -> muda para 50%
			lcd_command(0xC0);
			lcd_print("Ciclo: 50%");
			TIM3->CCR3 = 18; //36*0,50=18
		    delay_ms (100);
			break;
		case 18: 	//Se estiver em 50%  -> muda para 75%
			lcd_command(0xC0);
			lcd_print("Ciclo: 75%");
			TIM3->CCR3 = 27; //36*0,75=27 
			delay_ms (100);
			break;
	}													
}		

void lcd_init()
{
	delay_ms (15);
	GPIOA->ODR &= ~(1 << LCD_EN); //LCD_EN = 0
	delay_ms (3); //wait 3 ms 
	lcd_command (0x33); //lcd init .
	delay_ms (5);
	lcd_command (0x32); //lcd init .
	delay_us (3000);
	lcd_command (0x28); //4 - bit mode , 1 line and 5x8 charactere set
	delay_ms (3);
	lcd_command (0x0E); //display on , cursor on
	delay_ms (3);
	lcd_command (0x01); //display clear
	delay_ms (3);
	lcd_command (0x06); //move right
	delay_ms (3);
}

void lcd_putValue (unsigned char value)
{
	uint16_t aux ; //variable to help to build appropriate data out
	aux = 0x0000 ; //clear aux
	GPIOA->BRR = (1 << 5)|(1 << 6)|(1 << 8)|(1 << 11); //clear data lines
	aux = value & 0xF0 ;
	aux = aux >> 4;
	GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002 ) << 5) | (( aux & 0x0001 ) << 8);
	GPIOA->ODR |= (1 << LCD_EN ); /* EN = 1 for H - to - L pulse */
	delay_ms (3); /* make EN pulse wider */
	GPIOA->ODR &= ~(1 << LCD_EN ); /* EN = 0 for H - to - L pulse */
	delay_ms (1); /* wait */
	GPIOA->BRR = (1 << 5)|(1 << 6)|(1 << 8)|(1 << 11); // clear data lines
	aux = 0x0000; // clear aux
	aux = value & 0x0F;
	GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002) << 5) | ((aux & 0x0001) << 8);
	GPIOA-> ODR |= (1 << LCD_EN ); /* EN = 1 for H - to - L pulse */
	delay_ms (3); /* make EN pulse wider */
	GPIOA->ODR &= ~(1 <<LCD_EN ); /* EN = 0 for H - to - L pulse */
	delay_ms (1); /* wait */
}

void lcd_command (unsigned char cmd)
{
	GPIOA -> ODR &= ~(1 <<LCD_RS); /* RS = 0 para comando */
	lcd_putValue (cmd);
}

void lcd_data (unsigned char data)
{
	GPIOA->ODR |= (1 << LCD_RS ); /* RS = 1 para dados */
	lcd_putValue (data);
}

void lcd_print (char *str)
{
	unsigned char i = 0;
	while (str[i] != 0)
	{
		lcd_data (str[i]);
		i++;
	}
}

void delay_ms(uint16_t t)
{
	volatile unsigned long l = 0;
	for (uint16_t i = 0; i < t ; i ++)
	{
		for ( l = 0; l < 6000; l ++);
	}
}

void delay_us(uint16_t t)
{
	volatile unsigned long l = 0;
	for (uint16_t i = 0; i < t ; i ++)
	{
		for ( l = 0; l < 6; l ++);
	}
}