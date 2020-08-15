#include <Arduino.h>     //подключение системной библиотеки

#include "Alarm.h"       //класс для работы с часами и будильниками
#include "MyTFT.h"       //класс для работы с экраном
#include "Joystick.h"    //класс для работы с кнопками и джойстиком

#include "ModeNormal.h"  //класс для работы в основном режиме
#include "ModeAlarm.h"   //класс для работы в режиме сработавшего будильника
#include "ModeSetting.h" //класс для работы в режиме настроек
#include "GameSnake.h"   //класс для работы в режиме игры змейка

/**
 * Описываем разъемы
 */
// Пин подключения пьезоизлучателя
#define soundPin 3

// Для быстрой работы TFT экрана через шину SPI требуется предопределенные разъемы D11 (MOSI) и D13 (SCK),
// А также произвольные 3 разъема CS, DC, и RST. Мы их подключим к разъемам D10, D9 и D8 соответственно
#define cs 10 
#define dc 9 
#define rst 8 

// Разъем кнопки "Меню"
#define menuPin 6 

// Разъемы для подключения кнопки джойстика и контактов переменных резисторов
#define joyPin 4 
#define joyUDPin A0
#define joyLRPin A1

/*
 * Определяем режимы работы
 */
#define MODE_NORMAL  0  // выводим часы, обычный режим
#define MODE_ALARM   1  // сигнал будильника
#define MODE_SETTING 2  // режим настроек
#define MODE_GAME    3  // игра

/*
 * При каждой смене режима сначала делаем инициализацию режима через метод Init, затем в цикле запускаем метод Update.
 * В процедуре setup() проводим инициализацию режима MODE_NORMAL устанавливаем его текущим режимом.
 */
uint8_t mode;

// Инициализируем экран с заданными параметрами, в дальнейшем при работе будем обращаться к объекту tft
MyTFT tft = MyTFT(cs, dc, rst); 

//задаем буферы для формирования строк вывода на экран/в отладку
char strbuf[30];  
char strbuf2[30];

//создаем и инициализируем объект для работы с джойстиком
Joystick joy = Joystick(menuPin,joyPin,joyUDPin,joyLRPin);
//создаем и инициализируем объект для работы с часами и будильником, параметр - адрес в EEPROM, начиная с которого хранится 9 байт данных
Alarm         alarm(0);

//определяем объекты, с которыми будем работать в каждом режиме
ModeNormal    modeNormal  = ModeNormal(&tft,&strbuf[0],&strbuf2[0],&alarm);
ModeAlarm     modeAlarm   = ModeAlarm(&tft,&strbuf[0],&strbuf2[0],&alarm,soundPin);
ModeSetting   modeSetting = ModeSetting(&tft,&strbuf[0],&strbuf2[0],&alarm,&joy,0);  // последний параметр - адрес для хранения 9 байт настроек будильников
GameSnake     modeGame    = GameSnake(&tft,&strbuf[0],&strbuf2[0],&joy,soundPin,10);  // последний параметр - адрес для хранения двух байтов рекорда

/**
 * setup - устанавливающая функция, в ней происходит первоначальная настройка параметров устройства
 */
void setup() {
  Serial.begin(9600);           // инициализируем консольный порт для отладки
  Serial.println("TFT start");  // выводим отладочную информацию
  tft.begin();                  // инициализируем экран
  tft.fillScreen(ST7735_WHITE); // закрашиваем экран в белый цвет

  alarm.begin();                // инициализируем часы
  joy.begin();                  // инициализируем джойстик
  tone(soundPin,4000,20);       // короткий сигнал
  delay(200);                   // пауза 0.2 секунды

  tft.fillScreen(ST7735_BLACK); //закрашиваем экран в черный цвет

  //выводим приветственную надпись в указанные координаты, заданным размером шрифта и цветом
  tft.drawText(10,30,"Mr.Masya","",ST7735_WHITE,ST7735_BLACK,3);
  tft.drawUTF8Text(10,70,"ПРЕДСТАВЛЯЕТ","",ST7735_WHITE,ST7735_BLACK,2);

  delay(500);   // пауза 0.5 секунды

  // обновляем данные часов
  alarm.Update();

  //инициализируем генератор случайных чисел 
  randomSeed(alarm.getMinute() * 60 + alarm.getSecond());

  // инициализируем основной режим работы
  mode = MODE_NORMAL;
  modeNormal.Init();
}

/**
 * loop - основной цикл работы программы
 */
void loop() {
    alarm.Update();
    joy.Update();

  // Сработавшему будильнику - основной приоритет!
  // Если функция isAlarm ИСТИНА и текущий режим не MODE_ALARM
  if (alarm.isAlarm()&&mode!= MODE_ALARM){
    //переключаемся в режим будильника
    mode = MODE_ALARM;
    modeAlarm.Init();
  }else { 
    //четыре режима работы - четыре блока логики програмы
    //вначале обновляем текущий режим, затем проверяем условия смены режима
    switch(mode){
      
      case MODE_NORMAL:
        modeNormal.Update();
        if (joy.isMenuClick()){  // переключаемся из основного режима в режим настроек при нажатии кнопки "Меню"
          mode = MODE_SETTING;
          modeSetting.Init();
        }else if(joy.isJoyClick()) { // переключаемся из основного режима в режим игры при нажатии кнопки Джойстика
          mode = MODE_GAME;
          modeGame.Init();
        }
        break;

      case MODE_ALARM:
        modeAlarm.Update();
        // переключаемся из режима будильника в основной режим при нажатии на любую кнопку или изменения положения джойстика
        if(!alarm.isAlarm() || joy.isMenuClick() || joy.isJoyClick() || abs(joy.getLR())>20 || abs(joy.getUD())>20 ){

          if ( alarm.isAlarm() ) alarm.nextAlarm();  // если смена режима по нажатию клавишы, то обновляем ближайший будильник
          mode = MODE_NORMAL;
          modeNormal.Init();

        }
        break;

      case MODE_SETTING:
        modeSetting.Update();
        // выходим из режима настроек при нажатии клавиши "Меню"
        if (joy.isMenuClick() || modeSetting.isTimeout()){
          mode = MODE_NORMAL;
          modeNormal.Init();
        }
        break;

      case MODE_GAME:
        modeGame.Update();
        // выходим из игры при нажатии клавиши "Меню"
        if (joy.isMenuClick()){
          modeGame.Save();
          mode=MODE_NORMAL;
          modeNormal.Init();
        }
        break;

    }//switch

  }//if

}//loop

