#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile unsigned int adc_manual=0;
volatile unsigned int adc_motor=0;
volatile unsigned char canal=0;
volatile char c=0;

void config_USART(void){
  UBRR0=103;
  UCSR0B|=(1<<TXEN0);
  UCSR0C|=(1<<UCSZ01)|(1<<UCSZ00);
}

ISR(USART_UDRE_vect){
  UDR0=c;
  UCSR0B&=~(1<<UDRIE0);
}

void enviar_texto(const char* texto){
  while(*texto){
    c=*texto++;
    UCSR0B|=(1<<UDRIE0);
    _delay_ms(1);
  }
}

void enviar_entero(int valor){
  if(valor>=100){
    c='0'+(valor/100);
    UCSR0B|=(1<<UDRIE0);
    _delay_ms(1);
  }
  if(valor>=10){
    c='0'+((valor/10)%10);
    UCSR0B|=(1<<UDRIE0);
    _delay_ms(1);
  }
  else{
    c='0';
    UCSR0B|=(1<<UDRIE0);
    _delay_ms(1);
  }
  c='0'+(valor%10);
  UCSR0B|=(1<<UDRIE0);
  _delay_ms(1);
}

//manual=PC0 y motor=PC1
void config_ADC(void){
  ADMUX|=(1<<REFS0);
  ADCSRA|=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2);
  canal=0;
  ADMUX=(ADMUX&0xF0)|0x00;
  ADCSRA|=(1<<ADSC);
}

ISR(ADC_vect){
  if(canal==0){
    adc_manual=ADC;
    ADMUX=(ADMUX&0xF0)|0x01;
    canal=1;
  }
  else{
    adc_motor=ADC;
    ADMUX=(ADMUX&0xF0)|0x00;
    canal=0;
  }
  ADCSRA|=(1<<ADSC);
}

//PWM en PD6
void config_PWM(void){
  DDRD|=0x40;
  TCCR0A|=(1<<COM0A1)|(1<<WGM01)|(1<<WGM00);
  TCCR0B|=(1<<CS02)|(1<<CS00);//prescalador de 1024
  OCR0A=200;//velocidad(0–255)
}

//motor en PD2 y PD3
void config_motor(void){
  DDRD|=0x0C;
}

void avanzar(void){
  PORTD|=0x04;
  PORTD&=~0x08;
}

void retroceder(void){
  PORTD|=0x08;
  PORTD&=~0x04;
}

void detener(void){
  PORTD&=~0x0C;
}

float adc_a_grados(unsigned int valor_adc){
  return valor_adc*(270.0/1023.0);
}

//grados reales (45°-225°) a 0°–180°
int convercion(float grados_reales){
  if(grados_reales<45.0||grados_reales>225.0){
    return-1;
  }
  return(int)(grados_reales-45.0);
}

int main(void){
  config_USART();
  config_ADC();
  config_motor();
  config_PWM();
  sei();

  float grados_manual=0;
  float grados_motor=0;
  int manual_convertido=0;
  int motor_convertido=0;

  while(1){
    grados_manual=adc_a_grados(adc_manual);
    grados_motor=adc_a_grados(adc_motor);

    manual_convertido=convercion(grados_manual);
    motor_convertido=convercion(grados_motor);

    if(grados_motor<45.0||grados_motor>225.0){
      detener();
      OCR0A=0;
    }
    else{
      if(grados_manual>grados_motor+2){
        avanzar();
        OCR0A=200;
      }
      else if(grados_manual<grados_motor-2){
        retroceder();
        OCR0A=200;
      }
      else{
        detener();
        OCR0A=0;
      }
    }

    enviar_texto("Manual:");
    if(manual_convertido==-1){
      enviar_texto("Fuera de rango");
    }
    else{
      enviar_entero(manual_convertido);
      enviar_texto("°");
    }
    enviar_texto(" | Medido:");
    if(motor_convertido==-1){
      enviar_texto("Fuera de rango");
    }
    else{
      enviar_entero(motor_convertido);
      enviar_texto("°");
    }
    enviar_texto("\r\n");
    _delay_ms(1);
  }
}