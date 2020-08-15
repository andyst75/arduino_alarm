/*
Класс для работы со временем и будильниками. Основан на библиотеке DS3231 Real-Time Clock
*/

/*
DS3231.h - Header file for the DS3231 Real-Time Clock
Version: 1.0.1
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl
This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <EEPROM.h>

#ifndef Alarm_H
#define Alarm_H

#define DS3231_ADDRESS              (0x68)
#define DS3231_REG_TIME             (0x00)
#define DS3231_REG_TEMPERATURE      (0x11)

// структура для хранения даты/времени
struct RTCDateTime {
      uint16_t year;
      uint8_t month;
      uint8_t day;
      uint8_t hour;
      uint8_t minute;
      uint8_t second;
      uint8_t dayOfWeek;
      uint32_t unixtime;
};

// структура для хранения данных будильника
struct ALARM {
      uint8_t  day,hour,minute;
      uint32_t atime; // время срабатывания ближайшего будильника, в секундах unixtime
};

class Alarm {
  public:

    Alarm(uint8_t baseMem);
    void begin();
    void Update();
    float readTemperature();
    boolean isAlarm();

    uint8_t getHour(),
            getMinute(),
            getSecond(),
            getDay(),
            getMonth(),
            getDayOfWeek();
    uint16_t getYear();

    uint8_t getDayOfMonth(uint8_t m);

    //возвращает соответствующие данные будильника
    uint8_t  getAlarmDay(uint8_t d), getAlarmHour(uint8_t h), getAlarmMinute(uint8_t m);
    uint32_t getAlarmAtime(uint8_t a); // время срабатывания ближайшего будильника, в секундах unixtime

    //возвращаем текущее значение времени в виде структуры
    RTCDateTime getDateTime();
    //устанавливаем текущее значение даты / времени
    void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    //возвращаем данные конкретного будильника
    ALARM getAlarm(uint8_t);
    //номер ближайшего активного будильника
    int8_t getNearAlarm();
    //строковое представление ближайшего будильника в формате ДД.ММ ЧЧ.ММ
    void getNearAlarmStr(char* buf);
    //принудительно переключаемся на следующий будильник
    void nextAlarm();
    void alarmUpdate(uint8_t a, uint8_t day, uint8_t hour, uint8_t minute);

  private:
    struct tm  {
      uint8_t  tm_sec;
      uint8_t  tm_min;
      uint8_t  tm_hour;
      uint8_t  tm_mday;
      uint8_t  tm_mon;
      uint16_t tm_year;
    };

    void readClock();
    unsigned long lastUpdate;
    RTCDateTime t;
    uint8_t baseMem;
    uint8_t bcd2dec(uint8_t bcd),
            dec2bcd(uint8_t dec);
    uint32_t unixtime(),
             unixtime(uint8_t hour, uint8_t minute);
    long time2long(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);
    uint16_t date2days(uint16_t year, uint8_t month, uint8_t day);
    bool isLeapYear(uint16_t year);
    void atimeUpdate(uint8_t i);
    void xttotm(struct tm *tm_s, uint32_t t);
    uint8_t dow(uint16_t y, uint8_t m, uint8_t d);

    ALARM alarm[3];

    uint32_t nextAlarmTime=0;
    
};

#endif // Alarm_H
