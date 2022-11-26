#include <TM1637.h>
#include <CSV_Parser.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

//Serial Communication with ESP32
SoftwareSerial espSerial(32, 33);
String espCode, espMsg;

// SDCard
const int chipSelect = 53;
CSV_Parser cp(/*format*/ "sddsddd", /*has_header*/ true, /*delimiter*/ ';');

// 4 digit seven segment TM1637
TM1637 tm1(40, 41);
TM1637 tm2(42, 43);
TM1637 tm3(44, 45);

//Laser
#define laserA_1 22
#define laserA_2 23
#define laserA_3 24
#define laserB_1 25
#define laserB_2 26
#define laserB_3 27
#define laserC_1 28
#define laserC_2 29
#define laserC_3 30

//LDR
#define ldrA_1 31
#define ldrA_2 32
#define ldrA_3 33
#define ldrB_1 34
#define ldrB_2 35
#define ldrB_3 36
#define ldrC_1 37
#define ldrC_2 38
#define ldrC_3 39

int iTrafficLevelA = 0;
int iTrafficLevelB = 0;
int iTrafficLevelC = 0;

//TrafficLight
int R_led_A = 2;
int Y_led_A = 3;
int G_led_A = 4;

int R_led_B = 5;
int Y_led_B = 6;
int G_led_B = 7;

int R_led_C = 8;
int Y_led_C = 9;
int G_led_C = 10;

bool bGreenOn_A, bYellowOn_A, bRedOn_A;
bool bGreenOn_B, bYellowOn_B, bRedOn_B;
bool bGreenOn_C, bYellowOn_C, bRedOn_C;

//Data Traffic
int maxG, minG;
int hhStart, hhEnd, hhESP, mmESP;
int *arrHijau = 0;
String sJdwlOp[4];

int iDurasi = 0;
int iDurasiTemp = 0;
int iCount = 1;

//delay manager
const long delay_LDR = 1000;
unsigned long prev_LDR = 0;

const long delay_SerialComm = 1;    //in ms
unsigned long prev_SerialComm = 0;  //in ms

const long delay_Durasi = 1000;  //in ms
unsigned long prev_Durasi = 0;   //in ms

const long delay_SerialDebug = 1000;  //in ms
unsigned long prev_SerialDebug = 0;   //in ms

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  //4 digit 7 segment TM1637
  tm1.begin();
  tm2.begin();
  tm3.begin();

  //Traffic Light
  pinMode(R_led_A, OUTPUT);
  pinMode(Y_led_A, OUTPUT);
  pinMode(G_led_A, OUTPUT);

  pinMode(R_led_B, OUTPUT);
  pinMode(Y_led_B, OUTPUT);
  pinMode(G_led_B, OUTPUT);

  pinMode(R_led_C, OUTPUT);
  pinMode(Y_led_C, OUTPUT);
  pinMode(G_led_C, OUTPUT);

  digitalWrite(R_led_A, LOW);
  digitalWrite(Y_led_A, LOW);
  digitalWrite(G_led_A, LOW);

  digitalWrite(R_led_B, LOW);
  digitalWrite(Y_led_B, LOW);
  digitalWrite(G_led_B, LOW);

  digitalWrite(R_led_C, LOW);
  digitalWrite(Y_led_C, LOW);
  digitalWrite(G_led_C, LOW);

  //Laser
  pinMode(laserA_1, OUTPUT);
  pinMode(laserA_2, OUTPUT);
  pinMode(laserA_3, OUTPUT);

  pinMode(laserB_1, OUTPUT);
  pinMode(laserB_2, OUTPUT);
  pinMode(laserB_3, OUTPUT);

  pinMode(laserC_1, OUTPUT);
  pinMode(laserC_2, OUTPUT);
  pinMode(laserC_3, OUTPUT);

  digitalWrite(laserA_1, HIGH);
  digitalWrite(laserA_2, HIGH);
  digitalWrite(laserA_3, HIGH);

  digitalWrite(laserB_1, HIGH);
  digitalWrite(laserB_2, HIGH);
  digitalWrite(laserB_3, HIGH);

  digitalWrite(laserC_1, HIGH);
  digitalWrite(laserC_2, HIGH);
  digitalWrite(laserC_3, HIGH);

  //LDR
  pinMode(ldrA_1, INPUT);
  pinMode(ldrA_2, INPUT);
  pinMode(ldrA_3, INPUT);

  pinMode(ldrB_1, INPUT);
  pinMode(ldrB_2, INPUT);
  pinMode(ldrB_3, INPUT);

  pinMode(ldrC_1, INPUT);
  pinMode(ldrC_2, INPUT);
  pinMode(ldrC_3, INPUT);

  //SDCard
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");

    // don't do anything more:
    while (1)
      ;
  }
  Serial.println("card initialized.");

  getCSV();
  getMinMax();
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - prev_Durasi >= delay_Durasi) {
    lightLoopA();
    prev_Durasi = currentTime;
  }

  if (currentTime - prev_LDR >= delay_LDR) {
    getLDR();
    prev_LDR = currentTime;
  }

  if (currentTime - prev_SerialComm >= delay_SerialComm) {
    // getSerialMsg();
    prev_SerialComm = currentTime;
  }

  if (currentTime - prev_SerialDebug >= delay_SerialDebug) {
    serialDebug();
    prev_SerialDebug = currentTime;
  }
}

void lightLoopA() {
  if (iDurasi > 0) {
    if (iDurasiTemp <= 3 && iDurasiTemp >= 1) {
      bYellowOn_A = true;
      bYellowOn_B = true;
      bYellowOn_C = true;

      if (iCount == 3) {
        iCount = 1;
      }
    } else if (iDurasiTemp == 0) {
      iDurasiTemp = iDurasi;
      iCount++;
    } else {
      if (iCount == 1) {
        bRedOn_A = false;
        bYellowOn_A = false;
        bGreenOn_A = true;

        bRedOn_B = true;
        bYellowOn_B = false;
        bGreenOn_B = false;

        bRedOn_C = true;
        bYellowOn_C = false;
        bGreenOn_C = false;
      }

      if (iCount == 2) {
        bRedOn_A = true;
        bYellowOn_A = false;
        bGreenOn_A = false;

        bRedOn_B = false;
        bYellowOn_B = false;
        bGreenOn_B = true;

        bRedOn_C = true;
        bYellowOn_C = false;
        bGreenOn_C = false;
      }

      if (iCount == 3) {
        bRedOn_A = true;
        bYellowOn_A = false;
        bGreenOn_A = false;

        bRedOn_B = true;
        bYellowOn_B = false;
        bGreenOn_B = false;

        bRedOn_C = false;
        bYellowOn_C = false;
        bGreenOn_C = true;
      }
    }

    digitalWrite(R_led_A, bRedOn_A);
    digitalWrite(Y_led_A, bYellowOn_A);
    digitalWrite(G_led_A, bGreenOn_A);

    digitalWrite(R_led_B, bRedOn_B);
    digitalWrite(Y_led_B, bYellowOn_B);
    digitalWrite(G_led_B, bGreenOn_B);

    digitalWrite(R_led_C, bRedOn_C);
    digitalWrite(Y_led_C, bYellowOn_C);
    digitalWrite(G_led_C, bGreenOn_C);

    tm1.display(String(iDurasiTemp));
    tm2.display(String(iDurasiTemp));
    tm3.display(String(iDurasiTemp));

    iDurasiTemp--;
  }
}

void getSerialMsg() {
  while (espSerial.available() > 0) {
    String sBuff = espSerial.readStringUntil('\n');
    espCode = getValue(sBuff, '#', 0);
    espMsg = getValue(sBuff, '#', 1);

    //Jika ESP mengirim waktu (jam & menit)
    if (espCode == "T") {
      hhESP = getValueInt(espMsg, ':', 0);
      mmESP = getValueInt(espMsg, ':', 1);
    }
  }
}

void serialDebug() {
  Serial.println("LDR - Traffic Level");
  Serial.println("===================");
  Serial.println("Jalur A : " + String(iTrafficLevelA));
  Serial.println("Jalur B : " + String(iTrafficLevelB));
  Serial.println("Jalur C : " + String(iTrafficLevelC));
  Serial.println();
  Serial.println("Min - Max");
  Serial.println("===================");
  Serial.println("MinG : " + String(minG));
  Serial.println("MaxG : " + String(maxG));
}

void getCSV() {
  if (cp.readSDfile("/traffic.csv")) {
    int16_t *column_1 = (int16_t *)cp["LOKASI"];
    int16_t *column_2 = (int16_t *)cp["WKT START"];
    int16_t *column_3 = (int16_t *)cp["WKT END"];
    int16_t *column_4 = (int16_t *)cp["JALUR"];
    int16_t *column_5 = (int16_t *)cp["MERAH"];
    int16_t *column_6 = (int16_t *)cp["KUNING"];
    int16_t *column_7 = (int16_t *)cp["HIJAU"];

    if (column_1 && column_2 && column_3 && column_4 && column_5 && column_6 && column_7) {

      if (arrHijau != 0) {
        delete[] arrHijau;
      }
      arrHijau = new int[cp.getRowsCount()];

      for (int row = 0; row < cp.getRowsCount(); row++) {
        arrHijau[row] = column_7[row], DEC;
      }
    } else {
      Serial.println("ERROR: At least 1 of the columns was not found, something went wrong.");
    }

    // output parsed values (allows to check that the file was parsed correctly)
    cp.print();  // assumes that "Serial.begin()" was called before (otherwise it won't work)

  } else {
    Serial.println("ERROR: File called '/traffic.csv' does not exist...");
  }
}

void getMinMax() {
  maxG = arrHijau[0];
  minG = arrHijau[0];

  Serial.print("Size of myArray is: ");
  Serial.println(sizeof(arrHijau));

  for (int i = 0; i < (sizeof(arrHijau) / sizeof(arrHijau[0])); i++) {
    maxG = max(arrHijau[i], maxG);
    minG = min(arrHijau[i], minG);
  }

  iDurasiTemp = maxG;
  iDurasi = iDurasiTemp;
}

void setDuration() {
  if (hhESP >= hhStart && hhESP <= hhEnd) {
  }
}

void getLDR() {
  if (digitalRead(ldrA_1) > 500) {
    iTrafficLevelA = 1;
  } else if (digitalRead(ldrA_2) > 500) {
    iTrafficLevelA = 2;
  } else if (digitalRead(ldrA_3) > 500) {
    iTrafficLevelA = 3;
  } else {
    iTrafficLevelA = 0;
  }

  if (digitalRead(ldrB_1) > 500) {
    iTrafficLevelB = 1;
  } else if (digitalRead(ldrB_2) > 500) {
    iTrafficLevelB = 2;
  } else if (digitalRead(ldrB_3) > 500) {
    iTrafficLevelB = 3;
  } else {
    iTrafficLevelB = 0;
  }

  if (digitalRead(ldrC_1) > 500) {
    iTrafficLevelC = 1;
  } else if (digitalRead(ldrC_2) > 500) {
    iTrafficLevelC = 2;
  } else if (digitalRead(ldrC_3) > 500) {
    iTrafficLevelC = 3;
  } else {
    iTrafficLevelC = 0;
  }
}

//get Jadwal Operasi 1
void getJadwal() {

  int iHour = hhESP;  //START HOUR VARIABLE
  int iMin = mmESP;   //START MINUTE VARIABLE

  sJdwlOp[0] = iHour + ":" + iMin;
}

bool cek_Jadwal_Start() {
  String jamstart_jdwl[2];
  int jam_start[2];
  int mnt_start[2];

  for (int i = 0; i < 3; i++) {
    jam_start[i] = getValueInt(jamstart_jdwl[i], ':', 0);
    mnt_start[i] = getValueInt(jamstart_jdwl[i], ':', 1);
  }

  for (int i = 0; i < 3; i++) {

    if (jam_start[i] == hhESP && mnt_start[i] == mmESP) {
      return true;
    }
  }

  return false;
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int maxIndex = data.length() - 1;
  int strIndex[] = { 0, -1 };

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int getValueInt(String data, char separator, int index) {
  int found = 0;
  int maxIndex = data.length() - 1;
  int strIndex[] = { 0, -1 };

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]).toInt() : 0;
}
