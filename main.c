#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <math.h>

/******************************************************************************/

volatile int16_t pos=0;
volatile int16_t oldpos=0;
volatile int16_t p=10;
volatile int16_t setPoint;
volatile int16_t init_value = 460; //mark
volatile int16_t t=1;
volatile int16_t speed=1;
volatile bool reading = true;

/******************************************************************************/

void init(void){

	//DDRD |= (1<<DDD7);
	
	//DDRD &=~(1<<PD6);DDRD &=~(1<<PD5);DDRC &=~(1<<PC0);
	//DDRC &=~(1<<PC1);DDRC &=~(1<<PC2);DDRC &=~(1<<PC3);
	//DDRC &=~(1<<PC4);DDRC &=~(1<<PC5);//8 light barriers are input

	PORTC |= 255 ;  
	PORTD |= 255 ;  // pullup On
	
	DDRB |= (1<< DDB1); 
	PORTB &= ~(1<<PB1); //servo motor

	TCCR1A |= (1<<WGM11)|(1 << COM1A1);		//timer1//Fast PWM //non-inverting mode
	TCCR1B |= (1<<WGM13)|(1<<WGM12)|(1<<CS11);	//prescaler: 8 //top: ICR1

	TCCR0 = (1<<CS00)|(1<<CS02); //1024 for timer0
	TIMSK = (1<<TOIE0);//timer0 overflow interrupt

	TIMSK |= (1<<TICIE1);

	//ICR1 = 20000UL;
	ICR1 = 7000UL;

	sei();//enable global interrupts	
}

/******************************************************************************/

void ball_pass()//mark
{   
	if( (PINC & (1<<PC1)) && (PINC & (1<<PC2)) && (PINC & (1<<PC3)) && (PINC & (1<<PC0)) && (PIND & (1<<PD5)) && (PINC & (1<<PC4)) && (PIND & (1<<PD6)) && (PINC & (1<<PC5))  ) //条件没问题
		{   
			reading=true;
		}

	if(reading){	
		if(~PINC & (1<<PC1)) {   oldpos=pos; pos = -101; }
		if(~PINC & (1<<PC2)) {   oldpos=pos; pos =  101; }
		if(~PINC & (1<<PC0)) {   oldpos=pos; pos = -102; }
		if(~PINC & (1<<PC3)) {   oldpos=pos; pos =  102; }
		if(~PIND & (1<<PD5)) {   oldpos=pos; pos = -103; }
		if(~PINC & (1<<PC4)) {   oldpos=pos; pos =  103; }
		if(~PIND & (1<<PD6)) {   oldpos=pos; pos = -104; }
		if(~PINC & (1<<PC5)) {   oldpos=pos; pos =  104; }	
	}
}

/******************************************************************************/

void getspeed()
{   
	if(reading){
		if( (~PINC & (1<<PC1)) || (~PINC & (1<<PC2)) || (~PINC & (1<<PC3)) || (~PINC & (1<<PC0)) || (~PIND & (1<<PD5)) || (~PINC & (1<<PC4)) || (~PIND & (1<<PD6)) || (~PINC & (1<<PC5))  ) 
		{
		speed=300/t;  //speed=40就很快了
	    t=1;
	    reading=false;
		}	
	}
}

/******************************************************************************/

void controller()
{
	
if(oldpos+pos==0){
			setPoint = init_value  +  speed*pos/10; //这个数值需要调
			//_delay_ms(50); //时间需要调
			//setPoint = init_value; //或许这行和上面一行就不要了
}else{

	if (pos>0)
	{
		   if(oldpos==(pos+1))
		     {setPoint = init_value;}else{setPoint = init_value  + speed*pos*2/3;}
	}else{
		   if(oldpos==(pos-1))
		     {setPoint = init_value;}else{setPoint = init_value  + speed*pos*2/3;}
	     }	

    }

	if(setPoint > 1620) setPoint = 1620;
	else if(setPoint < 13) setPoint = 13;//mark

}

/******************************************************************************/

int main(void)
{
	init();
	OCR1A = init_value; //duty cycle//水平位置
	setPoint = OCR1A ;

  while(1){
	ball_pass();
	getspeed();
	controller();

	if(speed>5)
    {
	PORTD |= (1<<PD7);
    }else
	{ 
		PORTD &= ~(1<<PD7);
	}

    _delay_ms(50);
	     }

	return 0;
}

/******************************************************************************/

ISR(TIMER1_CAPT_vect) 
{
	if(setPoint != OCR1A)
	{
		if(setPoint > OCR1A)//mark
			{OCR1A++;OCR1A++;OCR1A++;OCR1A++;OCR1A++;OCR1A++;}
		else
			{OCR1A--;OCR1A--;OCR1A--;OCR1A--;OCR1A--;OCR1A--;}
	}
}

/******************************************************************************/

ISR(TIMER0_OVF_vect) //overflow中断 工作
{
	t++;//1second，t=30.5
}

/******************************************************************************/
