#include <TimeAlarms.h>

#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h>


void setup() {
  setSyncProvider(RTC.get);
  Serial.begin(9600);
  time_t T0 = fechaUnix(2017, 2, 8, 17, 24);
  Alarm.triggerOnce(T0, mensaje);
 
}

void loop() {
  digitalClockDisplay();
  Alarm.delay(1000); // wait one second between clock display
}

time_t fechaUnix(int ano, int mes, int dia, int hora, int minuto){
  tmElements_t tm;
    tm.Year =  ano - 1970;
    tm.Month = mes;
    tm.Day = dia;
    tm.Hour = hora;
    tm.Minute = minuto;
    tm.Second = 0;
  time_t T0 = makeTime(tm);
  return T0;
} 

void mensaje(){
  for (int i = 0; i < 10 ; i++){
   Serial.print("Hola mundo");
   delay(1000);
  }
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
