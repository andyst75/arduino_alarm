#include "ModeSetting.h";

ModeSetting::ModeSetting(MyTFT* tft, char* buf1, char* buf2, Alarm* alarm, Joystick* joy, uint16_t mem){
//запоминаем переданные параметры в наши указатели
  this->tft   = tft;
  this->buf1  = buf1;
  this->buf2  = buf2;
  this->alarm = alarm;
  this->joy   = joy;
  this->mem   = mem;
}

void ModeSetting::Init() {
  //очищаем экран
  tft->fillScreen(ST7735_BLACK);
  tft->drawUTF8Text(30,0,"НАСТРОЙКИ","",ST7735_WHITE,ST7735_BLACK,2);
  tft->drawUTF8Text(10,19,"Текущее дата/время","",ST7735_WHITE,ST7735_BLACK,1);

  //формируем строку с текущей датой
  sprintf(buf1,"%02d.%02d.%02d",alarm->getDay(),alarm->getMonth(),alarm->getYear()-2000);
  tft->drawText(0,28,buf1,"",ST7735_WHITE,ST7735_BLACK,2);

  //формируем строку с текущим временем
  sprintf(buf1,"%02d:%02d",alarm->getHour(),alarm->getMinute());
  tft->drawText(101,28,buf1,"",ST7735_WHITE,ST7735_BLACK,2);

  //выводим будильники
  for (uint8_t j=0;j<3;j++) {

    //выводим надпись "Будильник"
    tft->drawUTF8Text(10,48+j*27,"Будильник","",ST7735_WHITE,ST7735_BLACK,1);

    //выводим номер будильника
    sprintf(buf1,"%01d",j+1);
    tft->drawText(10+10*6,48+j*27,buf1,"",ST7735_WHITE,ST7735_BLACK,1);

    //если будильник активный, выводим A
    if ((alarm->getAlarmDay(j) & B10000000)==B10000000) tft->drawText(1,57+j*27,"\xB3","",ST7735_WHITE,ST7735_BLACK,2);

    //формируем время срабатывания будильника
    sprintf(buf1,"%02d:%02d",alarm->getAlarmHour(j),alarm->getAlarmMinute(j));
    tft->drawText(15,57+j*27,buf1,"",ST7735_WHITE,ST7735_BLACK,2);

    //выводим дни недели, 5 и 6 закрашиваем в красный цвет
    for(int i=0;i<7;i++){
      tft->fillRect(80+2+i*10,62+j*27,8,5, i < 5 ? ST7735_WHITE : ST7735_RED);
      //есть день недели неактивен, то закрашиваем середину
      if((alarm->getAlarmDay(j) & (1L << i)) == 0) tft->fillRect(80+2+1+i*10,63+j*27,6,3,ST7735_BLACK);
    }
  }

  menuCLine = 19;
  menuTimer = millis();
  lastKeyPressed = millis();
  menuCPos=0;

  setDateTime = alarm->getDateTime();
  
}

void ModeSetting::Update(){
  tft->drawText(0,menuCLine,"*","",((millis() - menuTimer) >> 8 ) % 2 == 0 ? ST7735_BLACK : ST7735_RED,ST7735_BLACK,1);

  // menuCPos==0 - находимся в режиме выбора строки для редактирования
  if (menuCPos==0) {
    if (abs(joy->getUD())>20) {
      if ((millis()-lastKeyPressed)>500) {
        lastKeyPressed = millis();
        tft->drawText(0,menuCLine,"*","",ST7735_BLACK,ST7735_BLACK,1);
        if (joy->getUD()<0) {
          switch (menuCLine) {
              case 19:menuCLine=102; break;
              case 48:menuCLine=19; break;
              case 75:menuCLine=48; break;
              case 102:menuCLine=75; break;
          }
        } else {
          switch (menuCLine) {
              case 19: menuCLine=48; break;
              case 48: menuCLine=75; break;
              case 75: menuCLine=102; break;
              case 102: menuCLine=19; break;
          }
        }
      }
    } else {
      //входим в режим редактирования строки
      if (joy->isJoyClick()) {
        //признак, что мы находимся не в режиме выбора строки, а в режиме редактирования
        menuCPos=1;
        if (menuCLine!=19) {
          aDay    = alarm->getAlarmDay((menuCLine-48)/27);
          aHour   = alarm->getAlarmHour((menuCLine-48)/27);
          aMinute = alarm->getAlarmMinute((menuCLine-48)/27);
        }
      }
    }
    
  } else {   // находимся в режиме редактирования конкретного элемента на строке
    aniMenuPos(menuCLine, menuCPos,((millis() - menuTimer) >> 8 ) % 2 == 0 ? ST7735_RED : ST7735_BLACK);

    //формируем горизонтальную позицию для редактирования
    if (abs(joy->getLR()) > 20) {
      if ((millis()-lastKeyPressed)>500) {
        lastKeyPressed = millis();
        //стираем предыдущий элемент для редактирования
        aniMenuPos(menuCLine, menuCPos,ST7735_BLACK);
        //перемещаем позицию
        menuCPos += joy->getLR() > 0 ? 1 : -1;
        if (menuCLine==19) { if (menuCPos<=0) menuCPos=5; if (menuCPos>5) menuCPos=1;}
        if (menuCLine!=19) { if (menuCPos<=0) menuCPos=10; if (menuCPos>10) menuCPos=1;}
      }
    }

    if (abs(joy->getUD()) > 20) {
      int joyUD = joy->getUD() > 0 ? -1 : 1 ;
      if ((millis()-lastKeyPressed)>500) {
        lastKeyPressed = millis();
        // настройка даты/времени
        if (menuCLine==19) {
            uint8_t x;
            switch (menuCPos) {
              //день
              case 1: //формируем предыдущее символьное представление дня месяца
                sprintf(buf1,"%02d",setDateTime.day);
                setDateTime.day+=joyUD;
                // если год високосный, то в феврале добавляем один день
                uint8_t t;
                t = alarm->getDayOfMonth(setDateTime.month) +  (((setDateTime.year % 4 == 0) && setDateTime.month == 2) ? 1 : 0); 
                if (setDateTime.day>t) { setDateTime.day = 1; } else if (setDateTime.day<1) { setDateTime.day = t; }
                //формируем новое символьное представление дня месяца
                sprintf(buf2,"%02d",setDateTime.day);
                //заменяем день
                x = 0;
                break;

              case 2: //формируем предыдущее символьное представление номера месяца
                sprintf(buf1,"%02d",setDateTime.month);
                setDateTime.month+=joyUD;
                if (setDateTime.month>12) { setDateTime.month = 1; } else if (setDateTime.month<1) { setDateTime.month = 12; }
                //формируем новое символьное представление номера месяца
                sprintf(buf2,"%02d",setDateTime.month);
                //заменяем месяц
                x = 0+3*6*2;
                break;

              case 3: //формируем предыдущее символьное представление последних двух цифр года
                      //соответсвенно диапазон для года будет 2000 - 2099
                sprintf(buf1,"%02d",setDateTime.year-2000);
                setDateTime.year+= joyUD;
                if (setDateTime.year>2099) { setDateTime.year = 2000; } else if (setDateTime.year<2000) { setDateTime.day = 2099; }
                //формируем новое символьное представление года
                sprintf(buf2,"%02d",setDateTime.year-2000);
                //заменяем год
                x = 0+6*6*2;
                break;

              case 4: //формируем предыдущее символьное представление часов
                sprintf(buf1,"%02d",setDateTime.hour);
                setDateTime.hour+= joyUD;
                if (setDateTime.hour>23) { setDateTime.hour = 0; } else if (setDateTime.hour<0) { setDateTime.hour = 23; }
                //формируем новое символьное представление часов
                sprintf(buf2,"%02d",setDateTime.hour);
                //заменяем часы
                x = 101;
                break;

              case 5: //формируем предыдущее символьное представление минут
                 sprintf(buf1,"%02d",setDateTime.minute);
                 setDateTime.minute+= joyUD;
                 if (setDateTime.minute>59) { setDateTime.minute = 0; } else if (setDateTime.minute<0) { setDateTime.minute = 59; }
                 //формируем новое символьное представление часов
                 sprintf(buf2,"%02d",setDateTime.minute);
                 //заменяем минуты
                x = 101+3*6*2;
                break;
            }
            tft->drawText(x,28,buf2,buf1,ST7735_WHITE,ST7735_BLACK,2);
          } else {
            switch (menuCPos) {
                case 1: //изменяем активность будильника
                        aDay = aDay ^ B10000000;
                        //если новое состояние будильника - активный, значит предыдущее - не активное
                        if (aDay & B10000000) {tft->drawText(1,menuCLine+9,"\xB3"," ",ST7735_WHITE,ST7735_BLACK,2);}
                          else {tft->drawText(1,menuCLine+9," ","\xB3",ST7735_WHITE,ST7735_BLACK,2);}
                        break;          
                case 2: //формируем предыдущее символьное представление часов
                        sprintf(buf1,"%02d",aHour);
                        aHour+=joyUD;
                        if (aHour>23) { aHour = 0; } else if (aHour<0) { aHour = 0; }
                        //формируем новое символьное представление часов
                        sprintf(buf2,"%02d",aHour);
                        //заменяем часы
                        tft->drawText(15,menuCLine+9,buf2,buf1,ST7735_WHITE,ST7735_BLACK,2);
                        break;          
                case 3: //формируем предыдущее символьное представление минут
                        sprintf(buf1,"%02d",aMinute);
                        aMinute+=joyUD;
                        if (aMinute>59) { aMinute = 0; } else if (aMinute<0) { aMinute = 0; }
                        //формируем новое символьное представление часов
                        sprintf(buf2,"%02d",aMinute);
                        //заменяем минуты
                        tft->drawText(15+3*6*2,menuCLine+9,buf2,buf1,ST7735_WHITE,ST7735_BLACK,2);
                        break;          
                case 4: 
                case 5: 
                case 6: 
                case 7: 
                case 8: 
                case 9: 
                case 10://активность по дням неделям
                        //выставляем бит дня недели в 1, остальные обнуляем
                        uint8_t t = 1 << (menuCPos-4);
                        aDay = aDay ^ t;
                        if (aDay & t) {tft->fillRect(40+2+1+menuCPos*10,menuCLine+15,6,3,menuCPos < 9 ? ST7735_WHITE : ST7735_RED);}
                          else {tft->fillRect(40+2+1+menuCPos*10,menuCLine+15,6,3,ST7735_BLACK);}
                        break;      

            }
          }
      }
    }

    // по нажатию клавиши сохраняем результаты и выходим из настроек строки
    if (joy->isJoyClick()) {
        aniMenuPos(menuCLine, menuCPos, ST7735_BLACK);
        lastKeyPressed = millis();
        if (menuCLine==19) {
          //редактировали дату/время
          alarm->setDateTime(setDateTime.year,setDateTime.month,setDateTime.day,setDateTime.hour,setDateTime.minute,0);
        } else {
          alarm->alarmUpdate((uint8_t) ((menuCLine-48)/27),aDay,aHour,aMinute);
        }
        menuCPos=0;

    }

  }
}

/*
 * Выводим подсветку активной позиции для редактирования
 * 
 * @param uint8_t l  - номер строки
 * @param uint8_t p  - деньнедели
 * @param uint16_t c - цвет
 */
void ModeSetting::aniMenuPos(uint8_t l, uint8_t p, uint16_t color) {
  // l== 19 - настройка даты 00.00.00 и времени 00:00
  uint8_t x,y,w=24; // в зависимости от выбранного элемента будет разная позиция и ширина элемента
  if (l==19) {
    y=44;
    switch (p) {
      case 1: x=0; break;          //день
      case 2: x=3*2*6; break;      //месяц 
      case 3: x=6*2*6; break;      //последние 2 цифры года
      case 4: x=101; break;        //часы
      case 5: x=101+3*2*6; break;  //минуты
    }
  } else {
    y=l+25;
    switch (p) {
      case 1: x=1; w=12;  break;  //активность будильника
      case 2: x=15;       break;  //часы
      case 3: x=15+3*2*6; break;  //минуты
      case 4: 
      case 5: 
      case 6: 
      case 7: 
      case 8: 
      case 9: 
      case 10: x=42+p*10; w=8; break; //активность по дням неделям
    }
  }
  //выводим подчеркивание в конкретную позицию
  tft->drawFastHLine(x,y,w,color);
}

boolean ModeSetting::isTimeout() {
  return (millis() - lastKeyPressed) > 120000;
}


