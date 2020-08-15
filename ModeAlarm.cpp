#include "ModeAlarm.h";

ModeAlarm::ModeAlarm(MyTFT* tft, char* buf1, char* buf2, Alarm* alarm, uint8_t soundPin) {
  //запоминаем переданные параметры в наши указатели
  this->tft   = tft;
  this->buf1  = buf1;
  this->buf2  = buf2;
  this->alarm = alarm;
  this->soundPin = soundPin;

}

void ModeAlarm::Init() {
  // закрашиваем экран в черный цвет
  tft->fillScreen(ST7735_BLACK);

  alrmTm = millis();
  strcpy(alrmOlds, "");
  alrmIv = (tm.second & B1) == 0 ? 1 : 0;

}

void ModeAlarm::Update() {
  if (millis() - alrmTm > 100 ) {
    alrmTm = millis();
    //получение текущей даты-времени
    tm = alarm->getDateTime();

    // четные секунды красный фон, нечетный - черный
    if (tm.second % 3 == 0) {
      tone(soundPin, 1985, 800);
    }

    if ((tm.second & B1) != alrmIv) {
      tft->fillScreen( alrmIv==1 ? ST7735_RED : ST7735_BLACK);
      sprintf(buf1, "%02d:%02d", tm.hour, tm.minute); // форматируем данные для вывода
      tft->drawText(6, 32, buf1, "", ST7735_WHITE, alrmIv==1 ? ST7735_RED : ST7735_BLACK, 5); // выводим текст
      alrmIv = tm.second & B1;
    }
  }
}
