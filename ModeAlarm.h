#include "MyTFT.h"
#include "Alarm.h"

#ifndef ModeAlarm_H
#define ModeAlarm_H

class ModeAlarm {
  public:
    //конструктор класса
    ModeAlarm(MyTFT* tft, char* buf1, char* buf2, Alarm* alarm, uint8_t soundPin);
    //инициализация режима
    void Init();

    //обновление информации на экране
    void Update();
  private:
    MyTFT* tft;
    char* buf1;
    char* buf2;
    Alarm* alarm;
    uint8_t soundPin;

    char   alrmOlds[12];  // dd.mm hh.mm\0
    unsigned long alrmTm;
    uint8_t alrmIv;
    RTCDateTime tm;

 
};

#endif // ModeAlarm
