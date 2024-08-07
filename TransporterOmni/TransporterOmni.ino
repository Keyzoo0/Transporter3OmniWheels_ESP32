#include <Ps3Controller.h>

#define PWMA  13
#define ENA1  12
#define ENA2  14
#define PWMB  25
#define ENB1  27
#define ENB2  26
#define PWMC  15
#define ENC1  2
#define ENC2  4
#define PWMD  5
#define END1  16
#define END2  17
#define SERVO 19

#define TIM_M1  2
#define TIM_M2  3
#define TIM_M3  4
#define TIM_M4  5
#define res     10
#define freq    1000

#define lambda 5
#define d2r(x) x*(PI/180)

#define LengthAlpha 0.1 // Ubah Disini untuk panjang dari sumbu roda ke tengah roda

int16_t x;
int16_t y;
int16_t th;

float atanVal;
int xR, yR;

void notify() {
  if ( Ps3.event.analog_changed.stick.rx != 0 || Ps3.event.analog_changed.stick.ry != 0 ) {
    atanVal = atan2(Ps3.data.analog.stick.rx, Ps3.data.analog.stick.ry);
    atanVal = (atanVal * 180 / PI);
    Serial.println(atanVal);
  }

  if ( Ps3.event.analog_changed.stick.lx != 0 || Ps3.event.analog_changed.stick.ly != 0) {
    xR = Ps3.data.analog.stick.lx;
    yR = Ps3.data.analog.stick.ly;
  }
}

void onConnect(){
    Serial.println("Connected.");
}

void initialize(){
  pinMode(PWMA, OUTPUT);
  pinMode(ENA1, OUTPUT);
  pinMode(ENA2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(ENB1, OUTPUT);
  pinMode(ENB2, OUTPUT);
  pinMode(PWMC, OUTPUT);
  pinMode(ENC1, OUTPUT);
  pinMode(ENC2, OUTPUT);
  pinMode(PWMD, OUTPUT);
  pinMode(END1, OUTPUT);
  pinMode(END2, OUTPUT);
  ledcAttachPin(PWMA, TIM_M1);
  ledcSetup(TIM_M1, freq, res);
  ledcAttachPin(PWMB, TIM_M2);
  ledcSetup(TIM_M2, freq, res);
  ledcAttachPin(PWMC, TIM_M3);
  ledcSetup(TIM_M3, freq, res);
  ledcAttachPin(PWMD, TIM_M4);
  ledcSetup(TIM_M4, freq, res);
}

void setup() {
  Serial.begin(115200);
  initialize();
  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("00:1A:7D:DA:71:13"); // Masukan MAC ADDRESS Ps Kalian
  Serial.println("Ready.");
}

void loop()
{
  if(!Ps3.isConnected()) return;

  kinMotor(yR, -xR, (atanVal*-1) * 400);
}

void kinMotor(int x, int y, int th) {
  int m1, m2, m3;
  m1 = lambda * (cos(d2r(135))*x + sin(d2r(135))*y + LengthAlpha*d2r(th));
  m2 = lambda * (cos(d2r(-90))*x + sin(d2r(-90))*y + LengthAlpha*d2r(th));
  m3 = lambda * (cos(d2r(45))*x + sin(d2r(45))*y + LengthAlpha*d2r(th));
  
  set_pwm(TIM_M1, m1);
  set_pwm(TIM_M2, m2);
  set_pwm(TIM_M3, m3);
}


void set_pwm(byte _chann, int _val) {
  switch (_chann) {
    case TIM_M1:
      digitalWrite(ENA1, _val < 0);
      digitalWrite(ENA2, _val >= 0);
      if (_val < 0) _val = _val * -1;
      ledcWrite(_chann, _val);
      break;
    case TIM_M2:
      digitalWrite(ENB1, _val < 0);
      digitalWrite(ENB2, _val >= 0);
      if (_val < 0) _val = _val * -1;
      ledcWrite(_chann, _val);
      break;
    case TIM_M3:
      digitalWrite(ENC1, _val < 0);
      digitalWrite(ENC2, _val >= 0);
      if (_val < 0) _val = _val * -1;
      ledcWrite(_chann, _val);
      break;
    case TIM_M4:
      digitalWrite(END1, _val >= 0);
      digitalWrite(END2, _val < 0);
      if (_val < 0) _val = _val * -1;
      ledcWrite(_chann, _val);
      break;
  }
}
