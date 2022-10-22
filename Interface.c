/************************************************************************************************************/
/*                                                                                                          */
/*                                              Interface.c                                                 */
/*                                                                                                          */
/*                                                                                       2020. 1. 1.        */
/************************************************************************************************************/
#include "Interface.h"

volatile unsigned char psd[10], g_psd[10]; // �̸� ���� 
volatile unsigned char rx1_flg=0, rx1_buff=0;


unsigned char read_gyro(unsigned char addr){
	unsigned char data=0;

	// ����
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
	while(!(TWCR & (1 << TWINT)));
	
	// �ּ� ����
	TWDR = 0x05 << 1;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	TWDR = addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
	while(!(TWCR & (1 << TWINT)));
	
	TWDR = (0x05 << 1)|0x01;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	data = TWDR;

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	

	return data;
}

void write_gyro(unsigned char addr, unsigned char data){
	// ����
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
	while(!(TWCR & (1 << TWINT)));
	
	// �ּ� ����
	TWDR = (0x05 << 1)|0x00;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));

	//data = TWDR;
	TWDR=addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	//data = TWDR;
	TWDR=data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	// ����
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

//***********************************************************************************************************

// Function  : LCD, LED, Buzzer, Switch�� ����ϱ� ���� �ʱ�ȭ
// Parameter : ����
// Return    : ����
void Interface_init(void)
{
    // LCD / EX I/O Expander
	TWDR = 0xFF;
	TWBR = 0x41; /// �̰� 41 = 100khz�� 

	LM629_HW_Reset();
	Sensor_init();
	MCU_init();
	Motor_init();

	TCCR1A=0x00; TCCR1B=0x05;
	TCNT1H=0xFF; TCNT1L=0x70;
	TIMSK=0x04;
	sei();
	
	DDRB=0x0F;
	DDRD&=~0x0F;

	DDRC |= 0xf0; ///// �߰� 
	PORTB = 0; ///// �߰� 
}

// Function  : MCU �ʱ�ȭ.
// Parameter : ����
// Return    : ����
void MCU_init(void)
{
	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	UCSR1A=0x00;
	UCSR1B=0x98;
	UCSR1C=0x06;
	UBRR1H=0x00;
	//UBRR1L=0x07; // USART1 Baud Rate: 115200
	UBRR1L=0x5F; // USART1 Baud Rate: 9600

	sei();
}

ISR(USART1_RX_vect){
     volatile unsigned char data;
	
     data=UDR1;
     while(!(UCSR1A&0xB8)); //���ſϷ� �� ���� ������� ������
     rx1_flg=1;              //���ſϷ� �÷��� set
     rx1_buff=data;
}

char getchar1(void)  //uart �����ϱ�
{
	volatile unsigned char data;
	
	data=rx1_buff;
	rx1_buff=0;
	cli();           //�۷ι� ���ͷ�Ʈ disable
	rx1_flg=0;       //���ſϷ� �÷��� reset
	sei();           //�۷ι� ���ͷ�Ʈ enable          
	return data;     //���ŵ����� ��ȯ
}

void putchar1(char data) //uart �۽�ȭ��
{
	while(!(UCSR1A&0x20));  //�۽� �غ� �Ϸ��
	UDR1=data;              //�۽� ���ۿ� ������ ����
}

// Function  : I2C�� ����Ͽ� LCD���� �ѹ���Ʈ ����
// Parameter :
//          data - ������ �ѹ���Ʈ ������
// Return    : ����
void lcd_write_data(unsigned char data){
     int n=0;

     // ����
     TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
     // �غ���� üũ
     _delay_us(200); ///// while�� _delay_us(200)���� ���� 
     // �ּ� ����
     TWDR = SLA << 1;
     TWCR = (1 << TWINT) | (1 << TWEN);
     n=0;
     _delay_us(200); ///// while�� _delay_us(200)���� ���� 
     TWDR = data;
     TWCR = (1 << TWINT) | (1 << TWEN);
     n=0;
     _delay_us(200); ///// while�� _delay_us(200)���� ���� 
     // ����
     TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

// Function  : ���ϴ� ���ο� ������ ���ڿ� ���
// Parameter :
//          line   - ����� ���ι�ȣ(0~3)
//          string - ����� ���ڿ�(�ִ� 20��)
// Return    : ����


//// lcd_display_str => LCD�� �����ϱ� 
void LCD(unsigned char Y_line, unsigned char X_line,char *string) //lcd ��Ʈ�������� ����
{
    int i=X_line;

	if((Y_line<4) && (X_line<20)){
		lcd_write_data(0x1B);   lcd_write_data(0x4C);
		lcd_write_data(X_line); lcd_write_data(Y_line);
		_delay_us(100);
    	while(((i++)<20)&&*string){
        	lcd_write_data(*string++);
			_delay_us(40);
		}
    }
}

void write_lcd_data(unsigned char Y_line, unsigned char X_line, long data) //lcd������ ����
{
    if(Y_line<4 && X_line<20){
		lcd_write_data(0x1B);   lcd_write_data(0x4C);
		lcd_write_data(X_line); lcd_write_data(Y_line);
		_delay_us(100);
		lcd_write_data(data);
		_delay_us(40);
    }
}

void lcd_clear_screen(void){ lcd_write_data(0x1B); lcd_write_data(0x43); _delay_ms(20); } //lcd��üȭ�� Ŭ����
void display_char(unsigned char line, unsigned char col, unsigned char data) ////lcd char ������ ǥ��
{
    unsigned char val=0, val_buff=0;

	val=data;   

	val_buff=val/100;             
	write_lcd_data(line, col, val_buff+'0' );
	val=val-(100*val_buff);

	val_buff=val/10;
	write_lcd_data(line, 1+col, val_buff+'0' );
	val=val-(10*val_buff);

	write_lcd_data(line, 2+col, val+'0');
}


