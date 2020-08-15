#include "Alarm.h"
#include <Wire.h>

const uint8_t daysArray [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
const uint8_t dowArray[] PROGMEM = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

// Инициализация класса
Alarm::Alarm(uint8_t baseMem) {
  this->baseMem=baseMem;
}

// Инициализация часов и начальное считывание настроек будтльника из памяти
void Alarm::begin() {
  Wire.begin();

  // считываем показания часов
  readClock();
  
  //считываем из памяти значения будильников
  for (int i=0; i<3; i++) {
    alarm[i].day     = EEPROM.read(baseMem+0+i*3);
    alarm[i].hour    = EEPROM.read(baseMem+1+i*3);
    alarm[i].minute  = EEPROM.read(baseMem+2+i*3);
    
    alarm[i].atime   = UINT32_MAX;  // максимальное значение срабатывания будильника
    
    if (alarm[i].hour > 23 || alarm[i].minute > 59) {  
      // в памяти ошибочные данные, обнуляем будильник
      alarm[i].day     = 0;
      alarm[i].hour    = 0;
      alarm[i].minute  = 0;
      // и фиксируем это в памяти
      EEPROM.write(baseMem+0+i*3, 0);
      EEPROM.write(baseMem+1+i*3, 0);
      EEPROM.write(baseMem+2+i*3, 0);
    } else {
      atimeUpdate(i);
    }
  }
  lastUpdate = millis();
}

/**
 * Cчитываем показания часов не чаще одного раза в 250 миллиссекунд
 */
void Alarm::Update(){
  unsigned long m = millis();
  if (m - lastUpdate > 250) {
    readClock();
    lastUpdate = m;
  }
}

/**
 * Проверяем, должен ли в данный момент звенеть будильник
 */
boolean Alarm::isAlarm() {
  boolean f = false;
  uint8_t i=0;
  while( i<3 && !f ) {
    if( (alarm[i].day & B10000000) && ((alarm[i].day >> t.dayOfWeek) & B00000001 ) && ((alarm[i].hour * 60 + alarm[i].minute) == (t.hour * 60 + t.minute)) && (t.unixtime>nextAlarmTime)) f = true;
    i++;
  }
  return f;
}

/**
 * Принудительно переключаемся на следующий будильник
 */
void Alarm::nextAlarm(){
  nextAlarmTime = unixtime() - t.second + 60;
}
 
/**
 * Обновляем параметр atime для конкретного будильника
 */
void Alarm::atimeUpdate(uint8_t i) {
  alarm[i].atime = UINT32_MAX;
  if( alarm[i].day & B10000000 ) {  // будильник активный
    // будильник установлен на сегодня, позже текущего времени
    int j=t.dayOfWeek;
    if( ((alarm[i].day >> j) & B00000001) && ((alarm[i].hour * 60 + alarm[i].minute) > (t.hour * 60 + t.minute)) ) {

        alarm[i].atime = unixtime(alarm[i].hour, alarm[i].minute);

    } else {
        // сначала проверяем активные будильники до конца недели
        j++;
        while(alarm[i].atime == UINT32_MAX && j <= 7) { // проверяем до конца недели
          if( (alarm[i].day >> j) & B00000001 ) {      // на данный день недели установлен будильник
              // вычисляем unixtime и корректируем на смещение в днях (один день - 86400 секунд)
              alarm[i].atime = unixtime(alarm[i].hour, alarm[i].minute) + (j - t.dayOfWeek) * 86400;
          }
          j++;
        }

        j=1;
        //проверяем активность будильника с начала следующей
        while(alarm[i].atime == UINT32_MAX && j < t.dayOfWeek) { // проверяем с начала недели до текушей даты
            if( (alarm[i].day>>j) & B00000001 ) { // на данный день недели установлен будильник
              // вычисляем unixtime и корректируем на смещение в днях (один день - 86400 секунд)
              alarm[i].atime = unixtime(alarm[i].hour, alarm[i].minute) + (j + 7 - t.dayOfWeek)*86400;
            }
            j++;
        }
    }
  } else alarm[i].atime = UINT32_MAX;
}

/**
 * Считываем текущее время
 */
void Alarm::readClock(){
  int values[7];

  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_REG_TIME);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 7);

  while(!Wire.available()) {}; // ожидание ответа шины

  for (int i = 6; i >= 0; i--) values[i] = bcd2dec(Wire.read());

  Wire.endTransmission();

  t.year = values[0] + 2000;
  t.month = values[1];
  t.day = values[2];
  t.dayOfWeek = values[3];
  // меняем формат хранения дня недели: 0 - понедельник, 6 - воскресенье
  if (t.dayOfWeek==0) t.dayOfWeek=6; else t.dayOfWeek--;
  t.hour = values[4];
  t.minute = values[5];
  t.second = values[6];
  t.unixtime = unixtime();
}

void Alarm::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_REG_TIME);
  Wire.write(dec2bcd(second));
  Wire.write(dec2bcd(minute));
  Wire.write(dec2bcd(hour));
  Wire.write(dec2bcd(dow(year, month, day)));
  Wire.write(dec2bcd(day));
  Wire.write(dec2bcd(month));
  Wire.write(dec2bcd(year-2000));
  Wire.write(DS3231_REG_TIME);
  Wire.endTransmission();

  //фиксируем значения во внутренней структуре
  t.year = year;
  t.month = month;
  t.day = day;
  t.dayOfWeek = dow(year, month, day);
  // меняем формат хранения дня недели: 0 - понедельник, 6 - воскресенье
  if (t.dayOfWeek==0) t.dayOfWeek=6; else t.dayOfWeek--;
  t.hour = hour;
  t.minute = minute;
  t.second = second;
  t.unixtime = unixtime();
}

uint8_t Alarm::dow(uint16_t y, uint8_t m, uint8_t d)
{
    uint8_t dow;

    y -= m < 3;
    dow = ((y + y/4 - y/100 + y/400 + pgm_read_byte(dowArray+(m-1)) + d) % 7);

    if (dow == 0)
    {
        return 7;
    }

    return dow;
}

uint8_t Alarm::getDayOfMonth(uint8_t m) {
  return pgm_read_byte(daysArray + m - 1);
}

/**
 * Считываем текущую температуру
 */
float Alarm::readTemperature(void)
{
    uint8_t msb, lsb;

    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_TEMPERATURE);
    Wire.endTransmission();
    Wire.requestFrom(DS3231_ADDRESS, 2);
    while(!Wire.available()) {};

    msb = Wire.read();
    lsb = Wire.read();

    return ((((short)msb << 8) | (short)lsb) >> 6) / 4.0f;
}

RTCDateTime Alarm::getDateTime(){
  return t;
}

ALARM Alarm::getAlarm(uint8_t i){
  return alarm[i];
}

void Alarm::alarmUpdate(uint8_t a, uint8_t day, uint8_t hour, uint8_t minute){
  //обновляем будильники в памяти
  alarm[a].day = day;
  alarm[a].hour = hour;
  alarm[a].minute = minute;
  //сохраняем настройку в энергонезависимую память
  EEPROM.write(baseMem+0+a*3,day);
  EEPROM.write(baseMem+1+a*3,hour);
  EEPROM.write(baseMem+2+a*3,minute);
  atimeUpdate(a);
}

/**
 * Возвращаем номер ближайшего активного будильника
 */
int8_t Alarm::getNearAlarm(){
  int8_t a=-1; // нет активных будильников
 
  // alarm.atime содержит unixtime ближайшего срабатывания. всегда больше текущего значения, если будильник неактивный, то значение равно uint32_max
  if (alarm[0].atime<=alarm[1].atime&&alarm[0].atime<=alarm[2].atime) {        // активен первый будильник
    a=0;
  } else if (alarm[1].atime<=alarm[0].atime&&alarm[1].atime<=alarm[2].atime) { // активен второй будильник
    a=1;
  } else if (alarm[2].atime<=alarm[0].atime&&alarm[2].atime<=alarm[1].atime) { // активен третий будильник
    a=2;
  }
  if (alarm[a].atime==UINT32_MAX) a=-1;

  return a;
}

/**
 * Возвращаем строковое представление ближайшего будильника в формате ДД.ММ ЧЧ.ММ
 */
void Alarm::getNearAlarmStr(char* buf){
  tm* t = new tm;
  t->tm_mday=0;
  t->tm_mon=0;
  t->tm_hour=0;
  t->tm_min=0;
  if (getNearAlarm()>=0) xttotm(t,alarm[getNearAlarm()].atime);
  sprintf(buf,"%02d.%02d %02d:%02d",t->tm_mday,t->tm_mon,t->tm_hour,t->tm_min); // форматируем данные для вывода
}


/**
 * Получаем текущие значения дней, месяцев, лет, часов и т.д.
 */
uint8_t Alarm::getHour(){ return t.hour; }
uint8_t Alarm::getMinute(){ return t.minute; }
uint8_t Alarm::getSecond(){ return t.second; }
uint8_t Alarm::getDay(){ return t.day; }
uint8_t Alarm::getMonth(){ return t.month; }
uint8_t Alarm::getDayOfWeek(){ return t.dayOfWeek; }
uint16_t Alarm::getYear(){ return t.year; }

uint8_t  Alarm::getAlarmDay(uint8_t a) { return (a>=0 && a<=2) ? alarm[a].day : 0; }
uint8_t  Alarm::getAlarmHour(uint8_t a) { return (a>=0 && a<=2) ? alarm[a].hour : 0; }
uint8_t  Alarm::getAlarmMinute(uint8_t a) { return (a>=0 && a<=2) ? alarm[a].minute : 0; }

uint32_t Alarm::getAlarmAtime(uint8_t a) { return (a>=0 && a<=2) ? alarm[a].atime : 0; }

uint8_t Alarm::bcd2dec(uint8_t bcd) { return ((bcd / 16) * 10) + (bcd % 16); }
uint8_t Alarm::dec2bcd(uint8_t dec) { return ((dec / 10) * 16) + (dec % 10); }

uint32_t Alarm::unixtime()
{
    uint32_t u;
    u = time2long(date2days(t.year, t.month, t.day), t.hour, t.minute, t.second);
    u += 946681200;
    return u;
}

uint32_t Alarm::unixtime(uint8_t hour, uint8_t minute)
{
    uint32_t u;
    u = time2long(date2days(t.year, t.month, t.day), hour, minute, 0);
    u += 946681200;
    return u;
}

uint16_t Alarm::date2days(uint16_t year, uint8_t month, uint8_t day)
{
    year = year - 2000;
    uint16_t days16 = day;

    for (uint8_t i = 1; i < month; ++i)
        days16 += pgm_read_byte(daysArray + i - 1);

    if ((month == 2) && isLeapYear(year))
        ++days16;

    return days16 + 365 * year + (year + 3) / 4 - 1;
}

long Alarm::time2long(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    return ((days * 24L + hours) * 60 + minutes) * 60 + seconds;
}

bool Alarm::isLeapYear(uint16_t year)
{
    return (year % 4 == 0);
}

/////////////////////////////////////////////////////////////////////
//перевод из unixtime в дата-время (заполнение полей структуры tm)
void Alarm::xttotm(struct tm *tm_s, uint32_t t)
{
    t -= 946681200;

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    second = t % 60;
    t /= 60;

    minute = t % 60;
    t /= 60;

    hour = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;

    for (year = 0; ; ++year)
    {
        leap = year % 4 == 0;
        if (days < 365 + leap)
        {
            break;
        }
        days -= 365 + leap;
    }

    for (month = 1; ; ++month)
    {
        uint8_t daysPerMonth = pgm_read_byte(daysArray + month - 1);

        if (leap && month == 2)
        {
            ++daysPerMonth;
        }

        if (days < daysPerMonth)
        {
            break;
        }
        days -= daysPerMonth;
    }

    day = days + 1;

    tm_s->tm_year = year+2000;
    tm_s->tm_mon = month;
    tm_s->tm_mday = day;
    tm_s->tm_hour = hour;
    tm_s->tm_min = minute;
    tm_s->tm_sec = second;

}
