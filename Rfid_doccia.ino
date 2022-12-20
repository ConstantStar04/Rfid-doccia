#include <LiquidCrystal_I2C.h> #include <SPI.h>
#include <MFRC522.h> #include <Wire.h>

const byte SS_PIN = 10; const byte RST_PIN = 9;
const byte lg = 2, lr = 3, lv = 4, lb = 5;
const byte cs[4] = {148, 46, 203, 115}; byte cl[4] = {NULL};
byte vv = false, st = 0, pp = 0, x = 0; const long int tp = 160000;//160000
unsigned long int dt, t1, dt2, t2, dt3, t3, dtt, tt, dtc, tc = 0;
String rx = "";
byte Edrop[8] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01010,
  0b00100,
};

byte Fdrop[8] = {
  0b00100,
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
};
MFRC522 mfrc522(SS_PIN, RST_PIN); LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  Serial.println("system start");
  SPI.begin(); mfrc522.PCD_Init();
  lcd.init(); lcd.init(); lcd.backlight();
  pinMode(lg, 1); pinMode(lr, 1); pinMode(lv, 1); pinMode(lb, 1);
  lcd.createChar(0, Edrop);lcd.createChar(1, Fdrop);
  lcd.setCursor(0, 0); lcd.print("Doccia pronta");
  lcd.setCursor(0, 1); lcd.print("avvicina tessera");
  digitalWrite(lb, 1);

}
void loop() {
  tt = millis();
  Serial.print(dtt); Serial.print(" - "); Serial.print((tp - dt) / 1000); Serial.print(" - ");
  switch (st) {
    case 0:
      Serial.print(pp); Serial.print(" - "); Serial.println("LETTURA");
      lettura();
      break;
    case 10:
      Serial.print(pp); Serial.print(" - "); Serial.println("VERIFICA");
      verifica();
      break;
    case 20:
      pp = 0;
      Serial.print(pp); Serial.print(" - "); Serial.println("LAVATI");
      doccia();
      break;
    case 150:
      Serial.print(pp); Serial.print(" - "); Serial.println("DOVE SEI ANDATO?");
      psdoccia();
      break;
    case 200:
      Serial.print(pp); Serial.print(" - "); Serial.println("RESET");
      rt();
      break;
  }
  dtt = millis() - tt;
}
//####################################################################################################################################
void lettura() {//0
  //  digitalWrite(lb, 1); digitalWrite(lv, 0); digitalWrite(lr, 0);
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  Serial.print("UID tag :");

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "); Serial.print(mfrc522.uid.uidByte[i], DEC);
    cl[i] = (mfrc522.uid.uidByte[i]);
  } Serial.println();
  st = 10;
}
//####################################################################################################################################
void verifica() {//10
  delay(1000);
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    //---------------------------------------------------------------------------------------------------------------------------------
    if (pp == 0 && cs[i] == cl[i]) {
      st = 20;
      lcd.clear();
      lcd.setCursor(0, 0);lcd.print("tempo:");
      tc = t1 = millis();
    } else if (pp == 0 && cs[i] != cl[i]) {
      lcd.clear();lcd.setCursor(0, 0);lcd.print("Tessera errata");
      delay(800);
      lcd.clear();lcd.print("Doccia pronta");lcd.setCursor(0, 1);lcd.print("avvicina tessera");
      st = 0;
      return;
    }
    //---------------------------------------------------------------------------------------------------------------------------------
    if (pp == 10 && cs[i] == cl[i]) {
      st = 150;
    } else if (pp == 10 && cs[i] != cl[i]) {
      st = 20;
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("tempo:");
      return;
    }
    //---------------------------------------------------------------------------------------------------------------------------------
    if (pp == 20 && cs[i] == cl[i]) {
      st = 20;
      digitalWrite(lr, 0);
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("tempo:");
      for (byte i = 0; i <= x; i++) {
        lcd.setCursor(i, 1); lcd.write(1);
        lcd.setCursor((i + 1), 1); lcd.write(0);
        Serial.println("goccia");
      }
      //t1 = millis();
    } else if (pp == 20 && cs[i] != cl[i]) {
      st = 150;
      lcd.clear();
      lcd.setCursor(0, 0);lcd.print("Tessera errata");
      delay(800);
      lcd.clear();
      lcd.setCursor(0, 0);lcd.print("tempo:");
      return;
    }
  }
}
//####################################################################################################################################
void doccia() {//20
  dt = millis() - t1; dt2 = millis() - t2; dtc = millis() - tc; dt3 = millis() - t3;
  if (dt > 1000) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { //carta non presente true [!false]
      Serial.println("verifica pausa");
      pp = 10; st = 0;
    }
  }

  if ((tp - dt) / 1000 == 99 || (tp - dt) / 1000 == 10) {
    lcd.setCursor(7, 0); lcd.print("   ");
    tc = millis();
  }

  if (dt < tp) {
    digitalWrite(lb, 0); digitalWrite(lv, 1);
    lcd.setCursor(7, 0); lcd.print((tp - dt) / 1000);
    if (dt3 > 10000) {
      lcd.setCursor(x, 1); lcd.write(1);
      lcd.setCursor((x + 1), 1); lcd.write(0);
      x++;
      t3 = millis();
    }
    if (dt > (tp - 5000)) {
      if (dt2 > 500) {
        digitalWrite(lg, 1);
      }
      if (dt2 > 1000) {
        digitalWrite(lg, 0);
        t2 = millis();
      }
    }
  } else {
    st = 200;
  }
}
//####################################################################################################################################
void psdoccia() {//150
  digitalWrite(lb, 0); digitalWrite(lv, 0); digitalWrite(lg, 0); digitalWrite(lr, 1);
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { //carta non presente true [!false]
    Serial.println("verifica ripressa");
    pp = 20; st = 0;
    t1 = millis() - dt;
  }
  //lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Doccia in pausa");
  lcd.setCursor(0, 1); lcd.print("avvicina tessera");

}
//####################################################################################################################################
void rt() {//200
  lcd.clear();
  t1 = millis();
  st = 0; pp = 0; x = 0;
  lcd.setCursor(0, 0); lcd.print("Doccia pronta");
  lcd.setCursor(0, 1); lcd.print("avvicina tessera");
  digitalWrite(lb, 1); digitalWrite(lg, 0); digitalWrite(lr, 0); digitalWrite(lv, 0);
}

