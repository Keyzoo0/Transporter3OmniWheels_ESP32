#include <PS4Controller.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"
#include <EEPROM.h>

//
#define true 1
#define false 0

/*
addresss eeprom
max_capit1 = 0 
min_capit1 = 1
max_capit2 = 2 
min_capit2 = 3
max_ankle1 = 4 
min_ankle1 = 5
max_ankle2 = 6
min_ankle2 = 7
speed motor= 8
speed lift = 9

*/
// EEPROM


/////////////////////////

/////////////////////////
int max_capit1,
  min_capit1,
  min_capit2,
  max_capit2,
  del_capit,
  min_ankle1,
  max_ankle1,
  min_ankle2,
  max_ankle2,
  SpeedM,
  SpeedM2;

// setup actuator
/////////////////////////////////////////////////
#define servo1 32
#define servo2 23
#define servo3 18
#define servo4 19
//contruct servo
Servo capit1;
Servo capit2;
Servo siku1;
Servo siku2;
void setup_actuator() {
  capit1.attach(23);
  capit2.attach(19);
  siku1.attach(32);
  siku2.attach(18);
}
/////////////////////////////////////////////////

//setup lift
/////////////////////////////////////////////////////
#define ENL1 16
#define ENL2 17
#define PWML 5
///////////////////////////////////////////////////

//setup  motor
///////////////////////////////////////////////////

#define PWMA 13
#define ENA1 12
#define ENA2 14
#define PWMB 25
#define ENB1 27
#define ENB2 26
#define PWMC 15
#define ENC1 2
#define ENC2 4

#define TIM_M1 2
#define TIM_M2 3
#define TIM_M3 4
#define TIM_M4 5
#define res 10
#define freq 1000
#define lambda 5
#define d2r(x) x*(PI / 180)

#define LengthAlpha 0.1  // Ubah Disini untuk panjang dari sumbu roda ke tengah roda

int16_t x;
int16_t y;
int16_t th;

float atanVal;
int xR, yR;

unsigned long lastTimeStamp = 0;

int set = 0;



void setup_motor() {

  pinMode(ENA1, OUTPUT);
  pinMode(ENA2, OUTPUT);

  pinMode(ENB1, OUTPUT);
  pinMode(ENB2, OUTPUT);

  pinMode(ENC1, OUTPUT);
  pinMode(ENC2, OUTPUT);


  ledcAttachPin(PWMA, TIM_M1);
  ledcSetup(TIM_M1, freq, res);
  ledcAttachPin(PWMB, TIM_M2);
  ledcSetup(TIM_M2, freq, res);
  ledcAttachPin(PWMC, TIM_M3);
  ledcSetup(TIM_M3, freq, res);

  pinMode(ENL1, OUTPUT);
  pinMode(ENL2, OUTPUT);
  ledcAttachPin(PWML, 5);
  ledcSetup(5, 1000, 10);
}
/////////////////////////////////////////////////////

//setup lcd
///////////////////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup_lcd() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  display_eeprom();
}

void display_eeprom() {
  lcd.setCursor(0, 0);
  lcd.print("    POLINEMA    ");
  lcd.setCursor(0, 1);
  lcd.print(" EEPROM_BAROKAH ");
}
//////////////////////////////////////////////


//setup PS4 controler
///////////////////////////////////////////////////
const char* mac_address = "14:13:33:a2:b7:2e";
void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

void setup_ps4() {
  PS4.begin(mac_address);
  removePairedDevices();  // This helps to solve connection issues
}
////////////////////////////////////////////////////

bool con_start = false;

void start_display() {
  if (PS4.isConnected()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print(mac_address);
    for (int i = 10; i < 16; i++) {
      lcd.setCursor(i, 0);
      lcd.print(".");
      delay(300);
    }
    lcd.clear();
    lcd.print("   Connected!  ");
    delay(1000);
    display_eeprom();
  }
}

/////////////////////////////////////////////////////
////////////////////


void lift_control() {
  ledcWrite(5, 1023);
  if (PS4.RStickY() > 120) {
    digitalWrite(ENL1, HIGH);
    digitalWrite(ENL2, LOW);
  } else if (PS4.RStickY() < -120) {
    digitalWrite(ENL1, LOW);
    digitalWrite(ENL2, HIGH);
  } else {
    digitalWrite(ENL1, LOW);
    digitalWrite(ENL2, LOW);
  }
}


//////////////////////////



void motor_control() {
  kinMotor(yR, -xR, (atanVal * -1) * 400);
}

void kinMotor(int x, int y, int th) {
  int m1, m2, m3;
  m1 = lambda * (cos(d2r(135)) * x + sin(d2r(135)) * y + LengthAlpha * d2r(th));
  m2 = lambda * (cos(d2r(-90)) * x + sin(d2r(-90)) * y + LengthAlpha * d2r(th));
  m3 = lambda * (cos(d2r(45)) * x + sin(d2r(45)) * y + LengthAlpha * d2r(th));
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
  }
}


int inverse, ry_val;
void move_control() {
  Serial.printf("rx:%4d,ry:%4d", inverse, ry_val);
  Serial.println();

  ry_val = PS4.RStickX();
  if (PS4.RStickX() != 0 || PS4.RStickY() != 0) {
    if (PS4.RStickY() <= 127 && PS4.RStickY() > -2) inverse = 127;
    if (ry_val > 10) ry_val -= 5;
    else if (ry_val < -10) ry_val += 5;
    atanVal = atan2(ry_val, inverse);
    atanVal = (atanVal * 180 / PI);
    Serial.printf("rx:%4d,ry:%4d", inverse, ry_val);
    Serial.println();
  }

  if (PS4.LStickX() != 0 || PS4.LStickY() != 0) {
    if (PS4.Right()) {
      Serial.println("kanan");
      xR = 128 * SpeedM / 255;
      yR = 0;
    } else if (PS4.Down()) {
      Serial.println("bawah");
      xR = 0;
      yR = -128 * SpeedM / 255;
    } else if (PS4.Up()) {
      Serial.println("atas");
      xR = 0;
      yR = 128 * SpeedM / 255;
    } else if (PS4.Left()) {
      Serial.println("kiri");
      xR = -128 * SpeedM / 255;
      yR = 0;
    } else {
      xR = PS4.LStickX() * SpeedM2 / 255;
      yR = PS4.LStickY() * SpeedM2 / 255;

      if (yR > xR) {
        if (yR > 50) xR = 0;
        else if (yR < -50) xR = 0;
      } else {
        if (xR > 50) yR = 0;
        else if (xR < -50) yR = 0;
      }
    }


    // Serial.printf("lx:%4d,ly:%4d,rx:%4d,ry:%4d", PS4.LStickX(), PS4.LStickY() , PS4.RStickX(), PS4.RStickY() );
    // Serial.println();
  }
}





int led_lcd = 0;
bool con_sh = false;
void lcd_control() {
  if (PS4.Share() && con_sh == false) {
    if (led_lcd > 1) led_lcd = 0;
    con_sh = true;
    led_lcd++;
    Serial.printf("led lcd :%2d", led_lcd - 1);
    Serial.println();
    delay(250);
  } else if (!PS4.Share()) {
    con_sh = false;
  }
  lcd.setBacklight(led_lcd - 1);
}


bool con_r1 = false, con_r2 = false;
int sk1 = 0, sk2 = 1;
int i_a1, i_a2;

// void ankle_control() {

//   if (PS4.R1() && con_r1 == false) {
//     con_r1 = true;
//     if(sk1 > 1) sk1 = 0;
//     sk1++;
//     if (sk1 == 1 ) {
//       for(i_a1 = min_ankle1 ; i_a1 < max_ankle1 ; i_a1++){
//       siku1.write(i_a1);

//       }
//       Serial.println("siku 1 naik");

//     } else if(sk1 == 2) {
//       for(i_a1 = max_ankle1 ; i_a1 > min_ankle1 ; i_a1--){
//       siku1.write(i_a1);

//       }
//       Serial.println("siku 1 turun");

//     }
//   } else if (!PS4.R1()) con_r1 = false;

//   if (PS4.R2() && con_r2 == false) {
//     con_r2 = true;
//     if(sk2 > 1) sk2 = 0 ;
//     sk2++;

//     if (sk2 == 1) {
//       for(i_a2 = min_ankle2 ; i_a2 < max_ankle2 ; i_a2++){
//       siku2.write(i_a2);

//       }
//       Serial.println("siku 2 naik");

//     } else if(sk2 == 2){
//       for(i_a2 = max_ankle2 ; i_a2 > min_ankle2 ; i_a2--){
//       siku2.write(i_a2);

//       }
//       Serial.println("siku 2 turun");

//     }
//   } else if (!PS4.R2()) con_r2 = false;
// }

int mode = 2;
bool con_op = false;
void mode_control() {
  if (PS4.Options() && con_op == false) {
    con_op = true;
    if (mode > 1) mode = 0;
    mode++;
    Serial.printf("mode :%2d ", mode);
    Serial.println();
    if (mode == 1) {
      Serial.println("direct servo");
    } else {
      Serial.println("delay servo");
    }
  } else if (!PS4.Options() && con_op == true) con_op = false;
}


void ankle_control() {
  if (PS4.R1()) {
    siku1.write(max_ankle1);
  }
  if (PS4.R2()) {
    siku1.write(min_ankle1);
  }
  if (PS4.L2()) {
    siku2.write(180 - min_ankle2);
  }
  if (PS4.L1()) {
    siku2.write(180 - max_ankle2);
  }
}

int con_sq = false,
    con_ci = false,
    con_tr = false,
    con_cr = false,
    dly = 50,
    i = 0,
    j = 0;


void actuator_control() {

  capit1.write(i);
  capit2.write(j);
  if (mode == 1) {
    if (PS4.Square() && con_sq == false) {
      con_sq = true;
      i = min_capit1;
      Serial.println("capit 1 terbuka");
    } else if (!PS4.Square()) con_sq = false;

    if (PS4.Circle() && con_ci == false) {
      con_ci = true;
      j = min_capit2;
      Serial.println("capit 2 terbuka");
    } else if (!PS4.Circle()) con_ci = false;

    if (PS4.Triangle() && con_tr == false) {
      con_tr = true;
      i = max_capit1;
      Serial.println("capit 1 tertutup");
    } else if (!PS4.Triangle()) con_tr = false;

    if (PS4.Cross() && con_cr == false) {
      con_cr = true;
      j = max_capit2;
      Serial.println("capit 2 tertutup");
    } else if (!PS4.Cross()) con_cr = false;


  } else if (mode == 2) {

    if (PS4.Square()) {
      for (i = i; i <= max_capit1; i++) {
        delay(del_capit);
        capit1.write(i);
        capit2.write(j);
        Serial.printf("i :%3d j :%3d", i, j);
        Serial.println();
        if (!PS4.Square()) break;
      }
    }

    if (PS4.Triangle()) {
      for (i = i; i >= min_capit1; i--) {
        delay(del_capit);
        capit1.write(i);
        capit2.write(j);
        Serial.printf("i :%3d j :%3d", i, j);
        Serial.println();
        if (!PS4.Triangle()) break;
      }
    }

    if (PS4.Cross()) {
      for (j = j; j <= max_capit2; j++) {
        delay(del_capit);
        capit1.write(i);
        capit2.write(j);
        Serial.printf("i :%3d j :%3d", i, j);
        Serial.println();
        if (!PS4.Cross()) break;
      }
    }

    if (PS4.Circle()) {
      for (j = j; j >= min_capit2; j--) {
        delay(del_capit);
        capit1.write(i);
        capit2.write(j);
        Serial.printf("i :%3d j :%3d", i, j);
        Serial.println();
        if (!PS4.Circle()) break;
      }
    }
  }
}

bool con_up = false,
     con_do = false,
     con_ri = false,
     con_le = false;

int menu_1 = 0;

bool clsc(bool cls = 0) {
  if (cls) {
    cls = false;
    lcd.clear();
  }
  return cls;
}
int crs_setting_x = 0,
    crs_setting_y = 0;
bool clsc_settings = 1;
void settings() {
  con_tr = false;
  clsc_settings = clsc(clsc_settings);
  lcd_control();

  //back
  if (PS4.Triangle()) {
    set = 0;
  }

  /////// Up
  if (PS4.Up() && con_up == false) {
    con_up = true;
    menu_1--;
    if (menu_1 < 0) menu_1 = 3;
  } else if (!PS4.Up()) con_up = false;

  /////// Down
  if (PS4.Down() && con_do == false) {
    con_do = true;
    menu_1++;
    if (menu_1 > 3) menu_1 = 0;
  } else if (!PS4.Down()) con_do = false;

  //////////right
  if (PS4.Right() && con_ri == false) {
    con_ri = true;
    menu_1 += 2;
    if (menu_1 % 2 == 0 && menu_1 > 3) menu_1 = 0;
    else if (menu_1 % 2 == 1 && menu_1 > 3) menu_1 = 1;

  } else if (!PS4.Right()) con_ri = false;

  ////////////left
  if (PS4.Left() && con_le == false) {
    con_le = true;
    menu_1 -= 2;
    if (menu_1 == -2) menu_1 = 2;
    else if (menu_1 == -1) menu_1 = 3;
  } else if (!PS4.Left()) con_le = false;

  ////////////////////////////////////////////////////////////////////

  if (menu_1 == 0) {
    crs_setting_x = 0;
    crs_setting_y = 0;
    lcd.setCursor(8, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(8, 1);
    lcd.print(" ");
  } else if (menu_1 == 1) {
    crs_setting_x = 0;
    crs_setting_y = 1;
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(8, 1);
    lcd.print(" ");
    lcd.setCursor(8, 0);
    lcd.print(" ");

  } else if (menu_1 == 2) {
    crs_setting_x = 8;
    crs_setting_y = 0;
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(8, 1);
    lcd.print(" ");

  } else if (menu_1 == 3) {
    crs_setting_x = 8;
    crs_setting_y = 1;
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(8, 0);
    lcd.print(" ");
  }
  lcd.setCursor(1, 0);
  lcd.print("Gripper");
  lcd.setCursor(9, 0);
  lcd.print("SpeedM ");
  lcd.setCursor(1, 1);
  lcd.print("Ankle  ");
  lcd.setCursor(9, 1);
  lcd.print("SpeedM2 ");

  lcd.setCursor(crs_setting_x, crs_setting_y);
  lcd.print(">");
  Serial.printf("menu : %2d", menu_1);
  Serial.println();


  if (menu_1 == 0 && PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 2;

  } else if (menu_1 == 1 && PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 3;

  } else if (menu_1 == 2 && PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 4;

  } else if (menu_1 == 3 && PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 5;
  }
}

bool clsc_settings2 = true;
int menu_grip = 0;

bool commit = true;
void reset_commit() {
  if (set == 0) commit = true;
}

void grip_setting() {
  clsc_settings2 = clsc(clsc_settings2);
  if (PS4.L3() || PS4.R3()) set = 1;
  if (!PS4.Cross() && con_cr == true) con_cr = false;
  if (PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 0;
  }
  lcd.setCursor(1, 0);
  lcd.print("1+");
  lcd.setCursor(4, 0);
  lcd.print("1-");
  lcd.setCursor(7, 0);
  lcd.print("2+");
  lcd.setCursor(10, 0);
  lcd.print("2-");
  lcd.setCursor(13, 0);
  lcd.print("dly");
  lcd.setCursor(0, 1);
  lcd.printf("%3d", max_capit1);
  lcd.setCursor(3, 1);
  lcd.printf("%3d", min_capit1);
  lcd.setCursor(6, 1);
  lcd.printf("%3d", max_capit2);
  lcd.setCursor(9, 1);
  lcd.printf("%3d", min_capit2);
  lcd.setCursor(13, 1);
  lcd.printf("%3d", del_capit);

  /////////EEPROM Write
  EEPROM.write(0, max_capit1);
  EEPROM.write(1, min_capit1);
  EEPROM.write(2, max_capit2);
  EEPROM.write(3, min_capit2);
  EEPROM.write(4, del_capit);
  EEPROM.commit();



  //// right
  if (PS4.Right() && con_ri == false) {
    con_ri = true;
    menu_grip++;
    if (menu_grip > 4) menu_grip = 0;
  } else if (!PS4.Right() && con_ri == true) con_ri = false;
  //// left
  if (PS4.Left() && con_le == false) {
    con_le = true;
    menu_grip--;
    if (menu_grip < 0) menu_grip = 4;
  } else if (!PS4.Left() && con_le == true) con_le = false;
  ////// Cursor
  if (menu_grip == 0) {

    capit1.write(max_capit1);

    if (PS4.Up()) {
      for (max_capit1 = max_capit1; max_capit1 < 180; max_capit1++) {
        capit1.write(max_capit1);
        lcd.setCursor(0, 1);
        lcd.printf("%3d", max_capit1);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (max_capit1 = max_capit1; max_capit1 > 0; max_capit1--) {
        capit1.write(max_capit1);
        lcd.setCursor(0, 1);
        lcd.printf("%3d", max_capit1);
        if (!PS4.Down()) break;
        delay(100);
      }
    }
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    lcd.setCursor(12, 0);
    lcd.print(" ");
  } else if (menu_grip == 1) {

    capit1.write(min_capit1);

    ////// Cursor
    if (PS4.Up()) {
      for (min_capit1 = min_capit1; min_capit1 < 180; min_capit1++) {
        capit1.write(min_capit1);
        lcd.setCursor(3, 1);
        lcd.printf("%3d", min_capit1);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (min_capit1 = min_capit1; min_capit1 > 0; min_capit1--) {
        capit1.write(min_capit1);
        lcd.setCursor(3, 1);
        lcd.printf("%3d", min_capit1);
        if (!PS4.Down()) break;
        delay(100);
      }
    }
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(">");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    lcd.setCursor(12, 0);
    lcd.print(" ");

  } else if (menu_grip == 2) {

    capit2.write(max_capit2);


    ////// Cursor
    if (PS4.Up()) {
      for (max_capit2 = max_capit2; max_capit2 < 180; max_capit2++) {
        capit2.write(max_capit2);
        lcd.setCursor(6, 1);
        lcd.printf("%3d", max_capit2);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (max_capit2 = max_capit2; max_capit2 > 0; max_capit2--) {
        capit2.write(max_capit2);
        lcd.setCursor(6, 1);
        lcd.printf("%3d", max_capit2);
        if (!PS4.Down()) break;
        delay(100);
      }
    }


    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(">");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    lcd.setCursor(12, 0);
    lcd.print(" ");
  } else if (menu_grip == 3) {


    capit2.write(min_capit2);
    ////// Cursor
    if (PS4.Up()) {
      for (min_capit2 = min_capit2; min_capit2 < 180; min_capit2++) {
        capit2.write(min_capit2);
        lcd.setCursor(9, 1);
        lcd.printf("%3d", min_capit2);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (min_capit2 = min_capit2; min_capit2 > 0; min_capit2--) {
        capit2.write(min_capit2);

        lcd.setCursor(9, 1);
        lcd.printf("%3d", min_capit2);
        if (!PS4.Down()) break;
        delay(100);
      }
    }

    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(">");
    lcd.setCursor(12, 0);
    lcd.print(" ");
  } else if (menu_grip == 4) {

    ////// Cursor
    if (PS4.Up()) {
      for (del_capit = del_capit; del_capit < 360; del_capit++) {
        lcd.setCursor(13, 1);
        lcd.printf("%3d", del_capit);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (del_capit = del_capit; del_capit > 0; del_capit--) {
        lcd.setCursor(13, 1);
        lcd.printf("%3d", del_capit);
        if (!PS4.Down()) break;
        delay(100);
      }
    }
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    lcd.setCursor(12, 0);
    lcd.print(">");
  }
}


int menu_ankle = 0;
void ankle_setting() {
  clsc_settings2 = clsc(clsc_settings2);
  if (PS4.L3() || PS4.R3()) set = 1;
  if (!PS4.Cross() && con_cr == true) con_cr = false;
  if (PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 0;
  }
  lcd.setCursor(1, 0);
  lcd.print("1+");
  lcd.setCursor(4, 0);
  lcd.print("1-");
  lcd.setCursor(7, 0);
  lcd.print("2+");
  lcd.setCursor(10, 0);
  lcd.print("2-");
  lcd.setCursor(0, 1);
  lcd.printf("%3d", max_ankle1);
  lcd.setCursor(3, 1);
  lcd.printf("%3d", min_ankle1);
  lcd.setCursor(6, 1);
  lcd.printf("%3d", max_ankle2);
  lcd.setCursor(9, 1);
  lcd.printf("%3d", min_ankle2);


  /////////EEPROM Write
  EEPROM.write(5, max_ankle1);
  EEPROM.write(6, min_ankle1);
  EEPROM.write(7, max_ankle2);
  EEPROM.write(8, min_ankle2);

  EEPROM.commit();



  //// right
  if (PS4.Right() && con_ri == false) {
    con_ri = true;
    menu_ankle++;
    if (menu_ankle > 3) menu_ankle = 0;
  } else if (!PS4.Right() && con_ri == true) con_ri = false;
  //// left
  if (PS4.Left() && con_le == false) {
    con_le = true;
    menu_ankle--;
    if (menu_ankle < 0) menu_ankle = 3;
  } else if (!PS4.Left() && con_le == true) con_le = false;
  ////// Cursor
  if (menu_ankle == 0) {

    siku1.write(max_ankle1);

    if (PS4.Up()) {
      for (max_ankle1 = max_ankle1; max_ankle1 < 180; max_ankle1++) {
        if (max_ankle1 == 180) max_ankle1 = 0;
        siku1.write(max_ankle1);
        lcd.setCursor(0, 1);
        lcd.printf("%3d", max_ankle1);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (max_ankle1 = max_ankle1; max_ankle1 >= 0; max_ankle1--) {
        if (max_ankle1 == 0) max_ankle1 = 180;
        siku1.write(max_ankle1);
        lcd.setCursor(0, 1);
        lcd.printf("%3d", max_ankle1);
        if (!PS4.Down()) break;
        delay(100);
      }
    }
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");

  } else if (menu_ankle == 1) {

    siku1.write(min_ankle1);

    ////// Cursor
    if (PS4.Up()) {
      for (min_ankle1 = min_ankle1; min_ankle1 <= 180; min_ankle1++) {
        if (min_ankle1 == 180) min_ankle1 = 0;
        siku1.write(min_ankle1);
        lcd.setCursor(3, 1);
        lcd.printf("%3d", min_ankle1);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (min_ankle1 = min_ankle1; min_ankle1 >= 0; min_ankle1--) {
        if (min_ankle1 == 0) min_ankle1 = 180;
        siku1.write(min_ankle1);
        lcd.setCursor(3, 1);
        lcd.printf("%3d", min_ankle1);
        if (!PS4.Down()) break;
        delay(100);
      }
    }
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(">");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");


  } else if (menu_ankle == 2) {

    siku2.write(180 - max_ankle2);


    ////// Cursor
    if (PS4.Up()) {
      for (max_ankle2 = max_ankle2; max_ankle2 <= 180; max_ankle2++) {
        if (max_ankle2 == 180) max_ankle2 = 0;
        siku2.write(180 - max_ankle2);
        lcd.setCursor(6, 1);
        lcd.printf("%3d", max_ankle2);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (max_ankle2 = max_ankle2; max_ankle2 >= 0; max_ankle2--) {
        if (max_ankle2 == 0) max_ankle2 = 180;
        siku2.write(180 - max_ankle2);
        lcd.setCursor(6, 1);
        lcd.printf("%3d", max_ankle2);
        if (!PS4.Down()) break;
        delay(100);
      }
    }


    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(">");
    lcd.setCursor(9, 0);
    lcd.print(" ");

  } else if (menu_ankle == 3) {

    siku2.write(180 - min_ankle2);

    ////// Cursor
    if (PS4.Up()) {
      for (min_ankle2 = min_ankle2; min_ankle2 <= 180; min_ankle2++) {
        if (min_ankle2 == 180) min_ankle2 = 0;
        siku2.write(180 - min_ankle2);
        lcd.setCursor(9, 1);
        lcd.printf("%3d", min_ankle2);
        if (!PS4.Up()) break;
        delay(100);
      }
    }
    if (PS4.Down()) {
      for (min_ankle2 = min_ankle2; min_ankle2 > 0; min_ankle2--) {
        if (min_ankle2 == 0) min_ankle2 = 180;
        siku2.write(180 - min_ankle2);
        lcd.setCursor(9, 1);
        lcd.printf("%3d", min_ankle2);
        if (!PS4.Down()) break;
        delay(100);
      }
    }

    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(">");
  }
}

void SpeedM_setting() {
  clsc_settings2 = clsc(clsc_settings2);
  if (PS4.L3() && PS4.R3()) set = 1;
  if (!PS4.Cross() && con_cr == true) con_cr = false;
  if (PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("Speed Motor:");
  lcd.setCursor(12, 0);
  lcd.printf("%3d", SpeedM);
  if (PS4.Up()) {
    for (SpeedM = SpeedM; SpeedM <= 255; SpeedM++) {
      if (!PS4.Up()) break;
      if (SpeedM == 255) SpeedM = 0;
      lcd.setCursor(12, 0);
      lcd.printf("%3d", SpeedM);
      delay(100);
    }
  }
  if (PS4.Down()) {
    for (SpeedM = SpeedM; SpeedM >= 0; SpeedM--) {
      if (!PS4.Down()) break;
      if (SpeedM == 0) SpeedM = 255;
      lcd.setCursor(12, 0);
      lcd.printf("%3d", SpeedM);
      delay(100);
    }
  }
  EEPROM.write(9, SpeedM);
  EEPROM.commit();
}

void SpeedL_setting() {
  clsc_settings2 = clsc(clsc_settings2);
  if (PS4.L3() && PS4.R3()) set = 1;
  if (!PS4.Cross() && con_cr == true) con_cr = false;
  if (PS4.Cross() && con_cr == false) {
    con_cr = true;
    set = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("Speed Lift :");
  lcd.setCursor(12, 0);
  lcd.printf("%3d", SpeedM2);
  if (PS4.Up()) {
    for (SpeedM2 = SpeedM2; SpeedM2 <= 255; SpeedM2++) {
      if (SpeedM2 == 255) SpeedM2 = 0;
      if (!PS4.Up()) break;
      lcd.setCursor(12, 0);
      lcd.printf("%3d", SpeedM2);
      delay(100);
    }
  }
  if (PS4.Down()) {
    for (SpeedM2 = SpeedM2; SpeedM2 >= 0; SpeedM2--) {
      if (SpeedM2 == 0) SpeedM2 = 255;
      if (!PS4.Down()) break;
      lcd.setCursor(12, 0);
      lcd.printf("%3d", SpeedM2);
      delay(100);
    }
  }
  EEPROM.write(10, SpeedM2);
  EEPROM.commit();
}



void reset_clsc() {
  if (set == 0) {
    clsc_settings = true;
    clsc_settings2 = true;
  } else if (set == 1) {
    clsc_settings2 = true;
  } else {
    clsc_settings = true;
  }
}

void connecting() {
  if (PS4.isConnected() && con_start == false) {
    con_start = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print(mac_address);
    for (int i = 10; i < 16; i++) {
      lcd.setCursor(i, 0);
      lcd.print(".");
      delay(200);
    }
    lcd.clear();
    lcd.print("   Connected!  ");
    delay(500);
    display_eeprom();
  } else if (!PS4.isConnected() && con_start == true) con_start = false;
}

void start() {
  if (PS4.isConnected()) {
    if (PS4.L3() && PS4.R3()) {
      set = 1;
    }
    if (set == 0) transporter();
    else if (set == 1) settings();
    else if (set == 2) grip_setting();
    else if (set == 3) ankle_setting();
    else if (set == 4) SpeedM_setting();
    else if (set == 5) SpeedL_setting();
  }
}

void setup_EEPROM() {
  EEPROM.begin(512);
}

void read_EEPROM() {
  max_capit1 = EEPROM.read(0);
  min_capit1 = EEPROM.read(1);
  max_capit2 = EEPROM.read(2);
  min_capit2 = EEPROM.read(3);
  del_capit = EEPROM.read(4);
  max_ankle1 = EEPROM.read(5);
  min_ankle1 = EEPROM.read(6);
  max_ankle2 = EEPROM.read(7);
  min_ankle2 = EEPROM.read(8);
  SpeedM = EEPROM.read(9);
  SpeedM2 = EEPROM.read(10);
}

void transporter() {
  if (!PS4.Cross() && con_cr == true) con_cr = false;
  lift_control();
  move_control();
  actuator_control();
  lcd_control();
  ankle_control();
  mode_control();
  motor_control();
  display_eeprom();
}


void setup() {
  Serial.begin(115200);
  setup_motor();
  setup_actuator();
  setup_lcd();
  setup_ps4();
  setup_EEPROM();
  read_EEPROM();
}

void loop() {
  connecting();
  start();
  reset_clsc();
  reset_commit();
}