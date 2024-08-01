#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);// Inisialisasi LCD 20x4
RTC_DS3231 rtc;
SoftwareSerial Serial2(2, 3);

int buttonstate = 4;
int relay = 5;
int sampling;

char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", 
"Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;

File myFile;

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(9600); // Mulai komunikasi serial dengan baud rate 9600

  Wire.begin();
  lcd.init(); // Inisialisasi LCD
  lcd.backlight(); // Nyalakan backlight LCD
  lcd.begin(20, 4); // Inisialisasi LCD 20x4
  lcd.clear(); // Hapus layar LCD

  // Tampilkan pesan selamat datang
  lcd.setCursor(3, 0);
  lcd.print("WELCOME");
  lcd.setCursor(3, 1);
  lcd.print("MONITORING");
  lcd.setCursor(3,3);
  lcd.print("Klik Sampling");
  delay(1000); // Tampilkan selama 1 detik

  // Atur waktu RTC dengan waktu kompilasi
  rtc.begin(); 
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // rtc.adjust(DateTime(2024, 5, 2, 01, 05, 00));

  // Inisialisasi SD Card
  Serial.print("Inisialisasi kartu SD...");
  if (!SD.begin(10)) {
    Serial.println("Inisialisasi gagal!");
    while (1);
  }
  Serial.println("Inisialisasi selesai");

  // Buka file "data.txt" untuk menulis
  myFile = SD.open("GAS.txt", FILE_WRITE);
  if (myFile) {
    myFile.print("Sampling\t");
    myFile.print("Timestamp\t\t\t");
    myFile.print("MQ9\t");
    myFile.close();
  } 
  pinMode(relay, OUTPUT);
  pinMode(buttonstate, INPUT_PULLUP);
}

void loop()
{
  if (digitalRead(buttonstate) == LOW)
  {
    // unsigned long startTime = millis(); // Waktu mulai program
    // unsigned long duration = 6 * 60 * 60 * 1000; // Durasi 6 jam dalam milidetik
    digitalWrite(relay, LOW);
   
    while (true)
    {
      int A_MQ9 = analogRead(A3);  // Baca sensor MQ8
      float vrl_MQ9 = A_MQ9 * (5.0 / 1024.0);
      float rs_MQ9 = ((5.0 / vrl_MQ9) - 1.0) * 2.642;
      float rasio_MQ9 = rs_MQ9 / 28.247;
      float MQ9 = exp((rasio_MQ9 –2.9951 / -0.218); 

      DateTime now = rtc.now();
      hari = dataHari[now.dayOfTheWeek()];
      tanggal = now.day(), DEC;
      bulan = now.month(), DEC;
      tahun = now.year(), DEC;
      jam = now.hour(), DEC;
      menit = now.minute(), DEC;
      detik = now.second(), DEC;

      // Tampilkan data pada LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQ9: ");
      lcd.print(MQ9);
      sampling++;

      lcd.setCursor(15, 3);
      lcd.print(sampling);
      
      lcd.setCursor(13, 1);
      lcd.print(now.hour(), DEC);
      lcd.print(':');
      lcd.print(now.minute(), DEC);
      lcd.print(':');
      lcd.print(now.second(), DEC);

      // Simpan data dengan timestamp ke kartu SD ketika relay aktif
      if (digitalRead(relay) == LOW)
      {
        myFile = SD.open("CB.txt", FILE_WRITE);
      if (myFile) {
        // Header hanya ditambahkan sekali
        myFile.print(sampling);
        myFile.print("\t\t");
        myFile.print(now.year(), DEC);
        myFile.print('/');
        myFile.print(now.month(), DEC);
        myFile.print('/');
        myFile.print(now.day(), DEC);
        myFile.print(' ');
        myFile.print(now.hour(), DEC);
        myFile.print(':');
        myFile.print(now.minute(), DEC);
        myFile.print(':');
        myFile.print(now.second(), DEC);
        myFile.print("\t\t");
        myFile.print(A_MQ9);
        myFile.print("\t");
        myFile.print(MQ9);
        myFile.print("\t");
      }

        //Pengiriman data ke ESP
        String msg = String(MQ9) + ";" + String(sampling);
        Serial.println(msg);
      }
      // Jeda selama 30 detik
      delay(30000);
    }
  }
  else
    { 
   digitalWrite(relay, HIGH); // Matikan relay jika tombol start tidak ditekan
    }
}
