#include "MyTFT.h"
#include "Alarm.h"
#include "Joystick.h"

#ifndef ModeSetting_H
#define ModeSetting_H

class ModeSetting {
  public:
    //конструктор класса
    ModeSetting(MyTFT* tft, char* buf1, char* buf2, Alarm* alarm, Joystick* joy, uint16_t mem);
    //инициализация режима
    void Init();

    //обновление информации на экране
    void Update();

    // выходим, если с момента последнего нажатия на кнопки прошло более 120 секунд
    boolean isTimeout();

  private:
    MyTFT* tft;
    char* buf1;
    char* buf2;
    Alarm* alarm;
    Joystick* joy;
    uint16_t mem;
    uint8_t menuCLine,menuCPos;
    unsigned long menuTimer, lastKeyPressed;
    RTCDateTime setDateTime;
    uint8_t  aDay,aHour,aMinute;
    
    void aniMenuPos(uint8_t l, uint8_t p, uint16_t color); //подсветка активной позиции для редактирования

};

#endif //ModeSetting
