#include "GyverTimers.h"
#include<Wire.h>
#define MCP4725 0x60


long num;// то, что отправляем
long cnt = 0, cntPr = 0; // счетчик
bool startAn = false;
int posVal;
int pack[10];
int i = 0;
int maxDAC = 1222;

volatile int counter = 9;

unsigned int adc;
byte buffer[3];

//позиция в отсчетах
long pos = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  attachInterrupt(0, packageSetup, CHANGE);
  attachInterrupt(1, setNull, RISING);
  Wire.begin();
  analogReference(INTERNAL);
  pinMode(6, INPUT);
  startCount();
  Timer2.setPeriod(25);

  pinMode(4, OUTPUT);

  PORTD |= 1 << 4;
}

void setNull() {
  Serial.println("null set");
  adc = map(0, 0, 80, 0, maxDAC);
  cnt = 10;
  /*buffer[0] = 0b01000000;            //записываем в buffer0 контрольный байт (010-Sets in Write mode)
    buffer[1] = adc >> 4;              //записываем наиболее значимые биты
    buffer[2] = adc << 4;              //записываем наименее значимые биты
    Wire.beginTransmission(MCP4725);         //присоединяемся к шине I2C с MCP4725 с адресом 0x61
    Wire.write(buffer[0]);            //передаем контрольный байт с помощью протокола I2C
    Wire.write(buffer[1]);            //передаем наиболее значимые биты с помощью протокола I2C
    Wire.write(buffer[2]);            // передаем наименее значимые биты с помощью протокола I2C
    Wire.endTransmission();*/
}
void packageSetup() {
  cnt = 0;
  startAn = false;
  Timer2.disableISR(CHANNEL_B);
  num = TCNT1 * 25, 714; //домножать тут!
  //Serial.println(num);
  for (int i = 0; i < 4; i++) {
    posVal = bitRead(num, 0) * (1 << 0) + bitRead(num, 1) * (1 << 1) + bitRead(num, 2) * (1 << 2) + bitRead(num, 3) * (1 << 3);
    pack[2 * i + 1] = posVal + 7;
    pack[2 * i + 2] = posVal + 7;
    num >>= 4;
  }
  pack[9] = 7;
  pack[0] = 1;
  counter = 9;
  Timer2.enableISR(CHANNEL_B);
}

ISR(TIMER2_B) {
  if (counter >= 0) {
    if (pack[9 - counter] > 0) {
      pack[9 - counter] --;
    }
    else {
      PORTD = PORTD ^ 1 << 4;
      counter --;
    }
  }
  else {
    Timer2.disableISR(CHANNEL_B);
    PORTD |= 1 << 4;
  }
}

//=================================================================
void loop()
{
  //TCNT1++;
  //if (TCNT1 >= 80) TCNT1=0;
  if (digitalRead(6) == HIGH)  startCount();
  //Serial.println(TCNT1);
  buffer[0] = 0b01000000;            //записываем в buffer0 контрольный байт (010-Sets in Write mode)
  buffer[1] = adc >> 4;              //записываем наиболее значимые биты
  buffer[2] = adc << 4;              //записываем наименее значимые биты
  Wire.beginTransmission(MCP4725);         //присоединяемся к шине I2C с MCP4725 с адресом 0x61
  Wire.write(buffer[0]);            //передаем контрольный байт с помощью протокола I2C
  Wire.write(buffer[1]);            //передаем наиболее значимые биты с помощью протокола I2C
  Wire.write(buffer[2]);            // передаем наименее значимые биты с помощью протокола I2C
  Wire.endTransmission();
  if (cnt > 0) {
    adc = 0;
    cnt--;
  }
  else adc = map(TCNT1, 0, 80, 0, maxDAC);
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
