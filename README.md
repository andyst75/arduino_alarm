# arduino_alarm
Arduino Alarm clock project

Будильник на базе Arduino UNO R3 (три будильника с возможностью выбора дней недели срабатывания) + классическая игра змейка.

Используются TFT на базе ST7735 и DS3231 в качестве RTC + термометр


Вид основного экрана (видим количество активных будильников, текущая температура, дата/время, день недели и дата/время срабатывания ближайшего будильника.

<p align="center">
  <img src="https://github.com/andyst75/arduino_alarm/blob/master/screen1.jpg?raw=true" alt="Arduino Alarm clock screen1"/>
</p>

Собранный будильник

<p align="center">
  <img src="https://github.com/andyst75/arduino_alarm/blob/master/screen2.jpg?raw=true" alt="Arduino Alarm clock screen2"/>
</p>

Для борьбы с мерцанием экрана при обновлении информации внесены изменения в функцию вывода символов: MyTFT::reDrawChar. Теперь обновляются только непосредственно измененные пиксели.
