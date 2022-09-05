#include "GyverTimers.h"

long num;// то, что отправляем
long cnt = 0;// счетчик
int posVal;
int pack[10];

volatile int counter = 9;

//позиция в отсчетах
long pos = 0;

void setup()
{
  Serial.begin(115200);
  attachInterrupt(0, packageSetup, CHANGE);
   
  analogReference(INTERNAL);
  pinMode(6, INPUT);
  startCount();
  Timer2.setPeriod(25);
  
  pinMode(4, OUTPUT);
  
  PORTD |= 1 << 4;
}

void packageSetup(){
  Timer2.disableISR(CHANNEL_B);
  num = TCNT1*25,714; //домножать тут!
  //Serial.println(num);
  for (int i = 0; i < 4; i++){
    posVal = bitRead(num, 0)*(1<<0)+bitRead(num, 1)*(1<<1)+bitRead(num, 2)*(1<<2)+bitRead(num, 3)*(1<<3);
    pack[2*i+1] = posVal + 7;
    pack[2*i+2] = posVal + 7;
    num >>= 4;
  }
  pack[9] = 7;
  pack[0] = 1;
  counter = 9;
  Timer2.enableISR(CHANNEL_B);
}

ISR(TIMER2_B){
  if (counter >= 0){
    if (pack[9 - counter] > 0){
      pack[9 - counter] --;
    }
    else{
      PORTD = PORTD ^ 1 << 4;
      counter --;
    }
  }
  else{
    Timer2.disableISR(CHANNEL_B);
    PORTD |= 1 << 4;
  }
}

//=================================================================
void loop()
{
  if (digitalRead(6) == HIGH)  startCount();
  Serial.println(TCNT1);
}
//=================================================================
void startCount()
{
  //Timer 1: overflow interrupt due to rising edge pulses on D5
  //Timer 2: compare match interrupt every 1ms
  noInterrupts();
  TCCR1A = 0; TCCR1B = 0; //Timer 1 reset
  TIMSK1 |= 0b00000001;   //Timer 1 overflow interrupt enable
  TCNT1 = 0;    //Timer 1 & 2 counters set to zero
  TCCR1B |= 0b00000111;   //Timer 1 external clk source on pin D5
  interrupts();
}
//=================================================================
ISR(TIMER1_OVF_vect)
{
}
//=================================================================
