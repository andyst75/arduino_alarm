#include "Joystick.h"

Joystick::Joystick(uint8_t menuPin, uint8_t joyPin,uint8_t joyUDPin,uint8_t joyLRPin) {
  this->menuPin=menuPin;
  this->joyPin=joyPin;
  this->joyUDPin=joyUDPin;
  this->joyLRPin=joyLRPin;
}

// инициализируем порты и проводим начальную настройку
void Joystick::begin() {
  pinMode(joyPin, INPUT);     // 4й цифровой разъем используется для кнопки джойстика
  digitalWrite(joyPin, HIGH); // в неподключенном состоянии на входе будет логическая единица (через внутренний резистор)

  pinMode(menuPin, INPUT);     // 6й цифровой разъем используется для кнопки джойстика
  digitalWrite(menuPin, HIGH); // в неподключенном состоянии на входе будет логическая единица (через внутренний резистор)

  prevJoyButton   = HIGH;      // выставляем начальное состояние кнопок
  valueMenuButton = HIGH;  
  prevJoyButton   = HIGH;
  valueJoyButton  = HIGH;

  isMenuReadStatus = btState3;  // клавиша не нажата
  isJoyReadStatus  = btState3;

  valueLR = 0;                  // выставляем начальное состояние джойстика
  valueUD = 0;   

  lastUpdate = millis();        // момент последней проверки состояния
  btUpdate   = millis();
}

boolean Joystick::isMenuButton() {
  // низкий уровень (LOW) означает что кнопка нажата
  return valueMenuButton == LOW ? true : false;
}

boolean Joystick::isJoyButton() {
  // низкий уровень (LOW) означает что кнопка нажата
  return valueJoyButton == LOW ? true : false;
}

boolean Joystick::isMenuClick() {
  // TODO: реализовать "автоклик", при зажатой клавише

  // низкий уровень (LOW) означает что кнопка нажата
  // если клавиша нажата и еще не считывали состояние, то переводим состояние на сладующий уровень и возвращаем нажатие
  boolean f = (valueMenuButton == LOW && btState1 == isMenuReadStatus ) ? true : false;
  if ( f ) isMenuReadStatus = btState2;
  return f;
}

boolean Joystick::isJoyClick() {
  // низкий уровень (LOW) означает что кнопка нажата
  // если клавиша нажата и еще не считывали состояние, то скидываем состояние и возвращаем нажатие
  boolean f = (valueJoyButton == LOW && btState1 == isJoyReadStatus) ? true : false;
  if ( f ) isJoyReadStatus = btState2;
  return f;
}

int8_t Joystick::getLR(){
  return (int8_t)valueLR;
}

int8_t Joystick::getUD(){
  return (int8_t)valueUD;
}

void Joystick::Update() {
  unsigned long m = millis();
  // считываем показания не чаще одного раза в 20 миллисекунд
  if ( m - lastUpdate >= 20 ) {

    // устраняем "дребезг контактов"
    // если значение нажатия кнопки не меняется в течение 40 миллисекунд, тогда считаем что это правильным значением
    if ( m - btUpdate >= 40 ) {
      // считываем зачение порта
      uint8_t n = digitalRead(menuPin);

      // низкий уровень - клавиша нажата, высокий - клавиша отжата
      // проверяем что предыдущее состояние такое же, тогда присваиваем значение во внутреннюю переменную
      if ( (LOW==n && LOW==prevMenuButton) || (HIGH==n && HIGH==prevMenuButton) ) {
        valueMenuButton = prevMenuButton;

        // если клавиша отжата, то state3
        // если клавишу нажали и state3, то state1
        // если клавиша нажата и считали состояние то state2

        if ( HIGH==prevMenuButton ) { isMenuReadStatus = btState3; }
        else if ( LOW==prevMenuButton && isMenuReadStatus == btState3 ) { isMenuReadStatus = btState1; };
      }
      // запоминаем текущее значение
      prevMenuButton = n;

      n = digitalRead(joyPin);
      if ( (LOW==n && LOW==prevJoyButton) || (HIGH==n && HIGH==prevJoyButton) ) {
        valueJoyButton = prevJoyButton;
        if ( HIGH==prevJoyButton ) { isJoyReadStatus = btState3; }
        else if ( LOW==prevJoyButton && isJoyReadStatus == btState3 ) { isJoyReadStatus = btState1; };
      }
      prevJoyButton = n;
      
      btUpdate = m;
    }

    //усредняем значения джойстика для уменьшения эффекта дребезга контактов
    valueLR = (valueLR + map(analogRead(joyLRPin),0,1023, -127, 127)) / 2;
    valueUD = (valueUD + map(analogRead(joyUDPin),0,1023, -127, 127)) / 2;
    lastUpdate = m;
  }
}
