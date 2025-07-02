#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void config_timer(void){
  TCCR0A|=(1<<WGM01);
  TCCR0B|=(1<<CS01);
}
void delay_10us(void){
  OCR0A=20;
  TCNT0=0;
  while(!(TIFR0&(1<<OCF0A)));
  TIFR0|=(1<<OCF0A);
}
unsigned char a=50;
void pwm(void){
  DDRB|=0x01;
  PORTB&=~0x01;
  config_timer();
}
void pwm(unsigned char a){
  PORTB|=0x01;
  for(int i=0; i<100+a;i++){
    delay_10us();
  }
  PORTB&=~0x01;
  for(int i=0;i<1900-a;i++){
    delay_10us();
  }
}
void config_ADC(void){
  ADCSRA|=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2);
  ADMUX|=(1<<REFS0);
}
ISR (ADC_vect){
  a=ADC*(100.0/1023.0);
}
int main(void){
  config_timer();
  config_ADC();
  sei();
  while(1){
    pwm(a);
    ADCSRA|=(1<<ADSC);
  }
}
