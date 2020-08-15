#include "ModeNormal.h"

const uint8_t BELL_HEIGHT = 16;
const uint8_t BELL_WIDTH  = 16;

//неактивный будильник, значек для вывода на экран
const uint16_t bell [BELL_HEIGHT] PROGMEM  = {
  //0123456789ABCDEF
  0B0000000000000001,//0
  0B0000000000110010,//1
  0B0000000001001100,//2
  0B0011111110000100,//3
  0B0100000000000010,//4
  0B1000000000000010,//5
  0B1000000000000010,//6
  0B1000000000000100,//7
  0B1000000000001000,//8
  0B0100000000001000,//9
  0B0010000000001000,//A
  0B0011000000001000,//B
  0B0010100000001000,//C
  0B0001111000001000,//D
  0B0000000100010000,//E
  0B0000000011100000 //F
  };

const uint16_t bell2 [BELL_HEIGHT] PROGMEM  = {
  //0123456789ABCDEF
  0B0000000000000001,//0
  0B0000000000110010,//1
  0B0000000001101100,//2
  0B0011111110110100,//3
  0B0100000000111010,//4
  0B1011111111111010,//5
  0B1011111111111010,//6
  0B1011111111100100,//7
  0B1011111111101000,//8
  0B0101111111101000,//9
  0B0010111111101000,//A
  0B0011011111101000,//B
  0B0010100111101000,//C
  0B0001111011101000,//D
  0B0000000100010000,//E
  0B0000000011100000 //F
  };

/**
 * Конструктор класса
 */
ModeNormal::ModeNormal(MyTFT* tft, char* buf1, char* buf2,Alarm* alarm){
  //запоминаем переданные параметры в наши указатели
  this->tft   = tft;
  this->buf1  = buf1;
  this->buf2  = buf2;
  this->alarm = alarm;
}

/**
 * Инициализация экрана и переменных
 */
void ModeNormal::Init() {
  //очищаем экран
  tft->fillScreen(ST7735_BLACK);

  tempNew = alarm->readTemperature();
  tempOld = -tempNew;

  //получение текущей даты-времени
  tmNew = alarm->getDateTime();

// обнуляем предыдущие значения в строках времени, даты и температуры  
  dtOlds[0] = 0; 
  tmOlds[0] = 0;
  tpOlds[0] = 0;
  tempOld   = 0;
  strcpy(alrmOlds,"");
  strcpy(tmOlds,"");
  strcpy(dtOlds,"");
  tmOld.second   = UINT8_MAX;
  tmOld.minute   = UINT8_MAX;
  tmOld.hour     = UINT8_MAX;
  tmOld.day      = UINT8_MAX;
  tmOld.month    = UINT8_MAX;
  tmOld.year     = UINT8_MAX;
  tmOld.dayOfWeek= UINT8_MAX;

// выводим дни недели, суббота и воскресенье - красные
 for(int i=0;i<7;i++){
      tft->fillRect(10+2+i*20,100,16,5, i < 5 ? ST7735_WHITE : ST7735_RED);
      tft->fillRect(10+2+1+i*20,101,14,3,ST7735_BLACK);
  }
// выводим значек температуры
  tft->drawText(145,5,"C","",ST7735_WHITE,ST7735_BLACK,2);

// выводим значки будильников в верхнем левом углу, с указанием активности
// старший бит поля day показывает активность будильника
  for (int i=0; i<3; i++) printBell(i*25,4,' ',ST7735_YELLOW,ST7735_BLACK,alarm->getAlarm(i).day&B10000000);

// выводим данные ближайшего будильника
  printAlarm();

}


/*
 * Функция для отображения значка будильника на экране.
 * Если act=1, то выводим активный будильник, если act=0, то неактивный.
 * Задаем точку вывода значка, цвет изображения, цвет фона, номер будильника (выводится только на неактивном).
 * Если цвет фона совпадает с цветом изображения, то фон прозрачный.
  */
void ModeNormal::printBell(uint8_t x, uint8_t y, char c, uint16_t color, uint16_t bg, uint8_t act) {
  for (uint8_t i=0;i<BELL_HEIGHT;i++) {
    uint16_t a = pgm_read_word(&(act ? bell2[i] : bell[i]));
      for (uint8_t j=0;j<BELL_WIDTH;j++) {
        if (a &  0B1000000000000000) { tft->drawPixel(x+j,y+i,color); }
        else if(color!=bg) tft->drawPixel(x+j,y+i,bg);
        a = a<<1;
      }
  }
  // если будильник не активный и есть символ для вывода, то выводим в центр колокольчика
  if (!act && c!=' ') tft->drawChar(x+4,y+5, c, color, bg, 1);
}

/*
 * Выводим на экран ближайший активный будильник. Если будильников нет, то выводим нулевой будильник с нулевыми значениями.
 */
void ModeNormal::printAlarm() {
  int8_t a = alarm->getNearAlarm();
  //есть активный будильник
  if (a>=0) {
    alarm->getNearAlarmStr(buf1);
    tft->drawText(30,110,buf1,alrmOlds,ST7735_WHITE,ST7735_BLACK,2);
    strcpy(alrmOlds,buf1); // запоминаем выведенную строку
    //выводим номер активного будильника. ASCII-код нуля - 48, для формирования номера для вывода прибавляем номер активного будильника и добавляем 1
    printBell(2, 109,48+a+1,ST7735_WHITE,ST7735_BLACK,0) ;
  } else { // нет активных будильников
    strcpy(buf2,"00.00 00:00");
    tft->drawText(30,110,buf2,alrmOlds,ST7735_WHITE,ST7735_BLACK,2);
    strcpy(alrmOlds,buf2); // запоминаем выведенную строку
    printBell(2, 109,'0',ST7735_WHITE,ST7735_BLACK,0) ;
  }
  
}


void ModeNormal::Update() {
    tmNew = alarm->getDateTime();
    //корректируем день недели, если воскресенье, то 6, понедельник - 0
    //if (tmNew.dayOfWeek==0) tmNew.dayOfWeek=6; else tmNew.dayOfWeek--;

    //если время изменилось, то выводим очень крупно текущее время
  if (tmNew.hour!=tmOld.hour || tmNew.minute!=tmOld.minute) {
    sprintf(buf1,"%02d:%02d",tmNew.hour,tmNew.minute); // форматируем данные для вывода
    tft->drawText(6, 32, buf1, tmOlds,ST7735_WHITE,ST7735_BLACK,5); // выводим текст
    strcpy(tmOlds,buf1); // запоминаем выведенную строку
    tmOld.hour       = tmNew.hour;
    tmOld.minute     = tmNew.minute;
  }
  //если дата изменилась, то выводим очень крупно текущую дату
  if (tmNew.day!=tmOld.day || tmNew.month!=tmOld.month || tmNew.year!=tmOld.year) {
    sprintf(buf1,"%02d.%02d.%04d",tmNew.day,tmNew.month,tmNew.year);
    tft->drawText(22, 80, buf1, dtOlds,ST7735_WHITE,ST7735_BLACK,2);
    strcpy(dtOlds,buf1);

    // и корректируем день недели 
    if (tmOld.dayOfWeek<7) tft->fillRect(10+2+tmOld.dayOfWeek*20,101,14,3,ST7735_BLACK); // делаем день недели не активным
    tft->fillRect(10+2+tmNew.dayOfWeek*20,100,16,5, tmNew.dayOfWeek < 5 ? ST7735_WHITE : ST7735_RED);

    tmOld.day       = tmNew.day;
    tmOld.month     = tmNew.month;
    tmOld.year      = tmNew.year;
    tmOld.dayOfWeek = tmNew.dayOfWeek;
  }

  //если изменилась температура, то выводим новое значение
  if (tempNew!=tempOld) {
    dtostrf(tempNew, 4, 1, buf1);  // преобразуем из float в строку длиной 4 символа и одним знаком после запятой
    tft->drawText(92, 5,buf1,tpOlds,ST7735_WHITE,ST7735_BLACK,2);
    strcpy(tpOlds,buf1);
    tempOld=tempNew;
  }

  //если изменилась изменилась секунда, то отрисовываем динамику движения
  if (tmOld.second!=tmNew.second) {
     if(tmNew.second){
     if(tmNew.second<11){
        tft->drawFastHLine(80,26,80*tmNew.second/10,ST7735_WHITE);
     } else{
        if (tmOld.second == UINT8_MAX) tft->drawFastHLine(80,26,80,ST7735_WHITE);  //первый запуск, отображаем уже произошедшие секунды
        if(tmNew.second<21) {
          tft->drawFastVLine(159,26,46*(tmNew.second-10.0)/10,ST7735_WHITE);      
        }else{
          if (tmOld.second == UINT8_MAX) tft->drawFastVLine(159,26,46,ST7735_WHITE);
          if(tmNew.second<31) {
             tft->drawFastHLine(80*(2.0-(tmNew.second-20.0)/10),72,80*(tmNew.second-20.0)/10,ST7735_WHITE);
          } else {
             if (tmOld.second == UINT8_MAX) tft->drawFastHLine(80,72,80,ST7735_WHITE);
             if(tmNew.second<41) {
                tft->drawFastHLine(80*(1.0-(tmNew.second-30.0)/10),72,80*(tmNew.second-30.0)/10,ST7735_WHITE);
             } else{ 
                if (tmOld.second == UINT8_MAX) tft->drawFastHLine(0,72,80,ST7735_WHITE);
                if(tmNew.second<51) {
                  tft->drawFastVLine(0,26+46-46*(tmNew.second-40.0)/10,46*(tmNew.second-40.0)/10,ST7735_WHITE);
                } else {
                  if (tmOld.second == UINT8_MAX) tft->drawFastVLine(0,26,46,ST7735_WHITE);
                  tft->drawFastHLine(0,26,80*(tmNew.second-50)/10,ST7735_WHITE);
                }
            }
          }
        }
     }
     } else{  // текущее значение секунд 0, очищаем области
      tft->drawFastHLine(80,26,80,ST7735_BLACK);
      tft->drawFastVLine(159,26,46,ST7735_BLACK);
      tft->drawFastHLine(80,72,80,ST7735_BLACK);
      tft->drawFastHLine(0,72,80,ST7735_BLACK);
      tft->drawFastVLine(0,26,46,ST7735_BLACK);
      tft->drawFastHLine(0,26,80,ST7735_BLACK);
     }
    
    tmOld.second=tmNew.second;
  }

}

