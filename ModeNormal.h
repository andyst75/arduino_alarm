#include "MyTFT.h"
#include "Alarm.h"

#ifndef ModeNormal_H
#define ModeNormal_H

class ModeNormal{
  public:
    //конструктор класса
    ModeNormal(MyTFT* tft, char* buf1, char* buf2, Alarm* alarm);
    //инициализация режима
    void Init();

    //обновление информации на экране
    void Update();

  private:
    MyTFT* tft;
    char* buf1;
    char* buf2;
    Alarm* alarm;
    
    char   dtOlds[11];    // dd.mm.yyyy\0
    char   tmOlds[6];     // hh.mm\0
    char   tpOlds[5];     // 00.0\0
    char   alrmOlds[12];  // dd.mm hh.mm\0

    float tempOld,tempNew;

    /**
     Определяем переменные типа структура RTCDateTime, в которую запрашивается информация о дате/времени
    */
    RTCDateTime tmOld;
    RTCDateTime tmNew;

    void printBell(uint8_t x, uint8_t y, char c, uint16_t color, uint16_t bg, uint8_t act);
    void printAlarm();
};

#endif //ModeNormal
