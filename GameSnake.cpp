#include <EEPROM.h>
#include "GameSnake.h"

GameSnake::GameSnake(MyTFT* tft, char* buf1, char* buf2, Joystick* joy, int8_t soundpin, uint16_t mem) {
  //запоминаем переданные параметры в наши указатели
  this->tft  = tft;
  this->buf1 = buf1;
  this->buf2 = buf2;
  this->mem  = mem;
  this->soundpin = soundpin;
  this->joy  = joy;

}

/**
   Инициализируем нашу змейку: перерисовываем экран, производим начальную инициализацию переменных
   @param tft  - ссылка на экран
   @param buf1 - ссылка на временный текстовый буффер
   @param buf2 - ссылка на временный текстовый буффер
   @param mem  - ссылка на младший байт рекорда, всего будет 2 байта
   @param soundpin  - пин пьезоизлучателя
*/
void GameSnake::Init() {
  //EEPROM.write(mem,0);
  //EEPROM.write(mem+1,0);
  record = EEPROM.read(mem) + (EEPROM.read(mem + 1) << 8);

  //закрашиваем экран в черный цвет, "->" означает, что мы ссылаемся на метод в классе, переданный по ссылке
  tft->background(0, 0, 0);
  //закрашиваем рамку желтым цветом
  tft->fillRect(2, 12, 4 + 96 + 4, 4 + 96 + 4, ST7735_YELLOW);
  //закрашиваем игровое поле зеленым цветом
  tft->fillRect(2 + 4, 12 + 4, 96, 96, ST7735_GREEN);
  //выводим надпись на экран
  tft->drawUTF8Text(112, 12, "СЧЁТ", "", ST7735_WHITE, ST7735_BLACK, 2);
  tft->drawText(112, 12 + 22, "0000", "", ST7735_WHITE, ST7735_BLACK, 2);

  tft->drawUTF8Text(112 + 4, 12 + 58, "РЕКОРД", "", ST7735_WHITE, ST7735_BLACK, 1);
  sprintf(buf2, "%04d", record);
  tft->drawText(112, 12 + 76, buf2, "", ST7735_WHITE, ST7735_BLACK, 2);

  //первоначальная длина змейки
  snake_l = 5;
  //заполняем первоначальное местоположение
  snake_xy[0] = 0x87;
  snake_xy[1] = 0x97;
  snake_xy[2] = 0xA7;
  snake_xy[3] = 0xA6;
  snake_xy[4] = 0xA5;
  //первоначальное направление
  snake_dx = -1;
  snake_dy = 0;
  //позиция головы змейки в массиве
  snake_head = 0;

  //отрисвовываем первоначальную змейку
  for (uint8_t i = 0; i < snake_l; i++) snake_paint(i, 0xF300);

  //сразу заполняем массив бонусов *просроченными* бонусами, чтобы сработал механизм первоначального заполнения
  for (uint8_t i = 0; i < BONUS_MAX; i++) {
    bonus[i].tm = 0;
    bonus[i].type = 0;
  }
  lastUpdate = millis();

  //отрисовываем их на экране
  repaintBonus();
  isGameover = false;
}


/**
   Возвращаем текущий счет
*/
uint16_t GameSnake::getCnt() {
  return cnt;
}

/**
   Проверяем на свободность поля по заданным координатам.
   @param x
   @param y
   @return - 0 - свободная ячейка, -1 - тело змейки, > 0 - порядковый номер ячейки в массиве бонусов (от 1 до BONUS_MAX)
*/
int8_t GameSnake::collision(uint8_t x, uint8_t y) {
  int8_t collision = 0;
  for (uint8_t i = 0; i < snake_l; i++) {
    if (((snake_xy[i] & 0xF0) >> 4 == x) && ((snake_xy[i] & 0x0F) == y)) {
      collision = -1;
      break;
    }
  }
  if (collision == 0)
    for (uint8_t i = 0; i < BONUS_MAX; i++) {
      if ((bonus[i].x == x) && (bonus[i].y == y)) {
        if  (bonus[i].type > 0) collision = i + 1;
        break;
      }
    }
  return collision;
}

/**
   Закрашиваем блок на игровом поле с координатами [0..15][0..15] заданным цветом
*/
void GameSnake::paintBlock(uint8_t x, uint8_t y, uint16_t color) {
  tft->fillRect(2 + 4 + x * 6, 12 + 4 + y * 6, 6, 6, color);
}

/**
   Выставляем бонус в ячейку i
*/
void GameSnake::insertBonus(uint8_t i) {
  uint16_t color;
  //do {
  bonus[i].x = random(0, 16);
  bonus[i].y = random(0, 16);
  //} while (collision(bonus[i].x, bonus[i].y) == 0);
  bonus[i].tm = millis();
  bonus[i].type = random(0, 100);
  if (bonus[i].type < 60) {
    color = ST7735_MAGENTA;
    bonus[i].type = 2;
  } else if (bonus[i].type < 90) {
    color = ST7735_RED;
    bonus[i].type = 1;
  } else {
    color = ST7735_CYAN;
    bonus[i].type = 3;
  }
  paintBlock(bonus[i].x, bonus[i].y, color);
}

/**
   Удаляем бонус из ячейки i
*/
void GameSnake::removeBonus(uint8_t i) {
  if (collision(bonus[i].x, bonus[i].y) > 0 && bonus[i].type > 0) paintBlock(bonus[i].x, bonus[i].y, ST7735_GREEN);
  bonus[i].type = 0;
}

/**
   Заполняем поле бонусами, если с момента выставления прошло более 5 секунд - убираем, если больше random(7,10) то выставляем
*/
void GameSnake::repaintBonus() {
  uint16_t color;
  for (uint8_t i = 0; i < BONUS_MAX; i++) {
    if (millis() > bonus[i].tm + random(3500, 4000)) {
      removeBonus(i);
      insertBonus(i);
    }
    else if ((millis() > bonus[i].tm + random(3000, 3500))) {
      removeBonus(i);
    }
  }
}

// отрисовка конкретной ячейки змейки
void GameSnake::snake_paint(uint8_t p, uint16_t color) {
  tft->fillRect(2 + 4 + ((snake_xy[p] & 0xF0) >> 4) * 6, 12 + 4 + (snake_xy[p] & 0x0F) * 6, 6, 6, color);
}

// сохранение результатов в памяти
void GameSnake::Save() {
  // не допускаем записи значений более 9999
  if ((cnt <= 9999) && (cnt > record)) {
    EEPROM.write(mem, (uint8_t)(cnt & 0xFF));
    EEPROM.write(mem + 1, (uint8_t)((cnt & 0xFF00) >> 8));
  }
}

//игра окончена
void GameSnake::gameover() {
  tft->background(0, 0, 0);
  tft->drawText(40, 30, "GAME", "", ST7735_WHITE, ST7735_BLACK, 3);
  tft->drawText(40, 70, "OVER", "", ST7735_WHITE, ST7735_BLACK, 3);
  Save();
  isGameover = true;
}

// вычисление задержки в движении змейки (скорость игры)
uint8_t GameSnake::delayGame(){
  uint8_t d = 30;
  if (cnt<10) {
    d+=180;
  } else if (cnt<20) {
    d+=140;
  } else if (cnt<50) {
    d+=120;
  } else if (cnt<100) {
    d+=100;
  } else if (cnt<250) {
    d+=50;
  } else if (cnt<500) {
    d+=30;
  } else if (cnt<750) {
    d+=10;
  }
  
  return d;
}

/**
   выбираем направление движения
   0-вверх(dy=-1), 1-вправо(dx=+1),2-вниз(dy=1),3-влево(dx=-1)
*/
void GameSnake::Update() {
  if (!isGameover) {
    if (millis() - lastUpdate > delayGame()) {
      lastUpdate = millis();
      repaintBonus();

      int8_t d = -1;
      //Джойстик должен быть сильно направлен в какую-либо сторону
      if (joy->getUD() < -30) {
        d = 0;
      }
      else if (joy->getUD() > 30) {
        d = 2;
      }
      else if (joy->getLR() > 30) {
        d = 1;
      }
      else if (joy->getLR() < -30) {
        d = 3;
      }

      if (d == 1 && snake_dx != -1) {
        snake_dy = 0;
        snake_dx = 1;
      }

      if (d == 3 && snake_dx != 1) {
        snake_dy = 0;
        snake_dx = -1;
      }

      if (d == 0 && snake_dy != 1) {
        snake_dx = 0;
        snake_dy = -1;
      }

      if (d == 2 && snake_dy != -1) {
        snake_dx = 0;
        snake_dy = 1;
      };

      // храним координаты кончика хвоста. если ячейка с координатами головы = 0, то это длина змейки-1 (отсчет идет от 0)
      // в ином случае это предыдущая ячейка
      uint8_t tail = snake_head == 0 ? snake_l - 1 : snake_head - 1;

      //формируем новые координаты головы
      int8_t newX = ((snake_xy[snake_head] & 0xF0) >> 4) + snake_dx;
      //проверяем, чтобы не вышли за пределы экрана
      if (newX < 0) {
        newX = 15;
      } else if (newX > 15) {
        newX = 0;
      };

      int8_t newY = (snake_xy[snake_head] & 0x0F) + snake_dy;
      //проверяем, чтобы не вышли за пределы экрана
      if (newY < 0) {
        newY = 15;
      } else if (newY > 15) {
        newY = 0;
      };

      //запоминаем результат проверки на ячейки, куда должна переместиться голова змейки
      int8_t colX = collision(newX, newY);

      //прибавляем очки, если съели бонус
      if (colX > 0) {
        sprintf(buf1, "%04d\0", cnt);
        cnt++;
        if (cnt <= 9999) {
          sprintf(buf2, "%04d\0", cnt);
          tft->drawText(112, 12 + 22, buf2, buf1, ST7735_WHITE, ST7735_BLACK, 2);
        }
        if (soundpin > 0) tone(soundpin, 6000, 40);
      }

      /*
         colX =-1 - сами себя съели, GAME OVER
         colX = 0 - пустое поле
         bonus[colX-1] = 1 - добавляем очки, не удлиняемся
         bonus[colX-1] = 2 - добавляем очки, удлиняемся
         bonus[colX-1] = 3 - добавляем очки, укорачиваемся
      */
      if (colX < 0) {
        gameover();
      }
      else if (colX == 0 || bonus[colX - 1].type == 1 || (bonus[colX - 1].type == 2 && snake_l >= SNAKE_MAX) || (bonus[colX - 1].type == 3 && snake_l <= 3)) {
        //перемещаемся
        snake_paint(tail, ST7735_GREEN);
        snake_xy[tail] = newX * 16 + newY;
        snake_paint(tail, 0xF300);
        snake_head = tail;
      } else if (bonus[colX - 1].type == 2 && snake_l < SNAKE_MAX) {
        //удлиняемся
        for (uint8_t i = snake_l; i > snake_head; i--) snake_xy[i] = snake_xy[i - 1];
        snake_l++;
        snake_xy[snake_head] = newX * 16 + newY;
        snake_paint(snake_head, 0xF300);
      } else if (bonus[colX - 1].type == 3 && snake_l > 3) {
        //укорачиваемся
        snake_paint(tail, ST7735_GREEN);
        tail = snake_head == 0 ? snake_l - 1 : snake_head - 1;
        snake_paint(tail, ST7735_GREEN);
        if (tail > 0) {
          for (uint8_t i = tail; i < snake_l; i++) snake_xy[i - 1] = snake_xy[i];
        }
        snake_xy[tail] = newX * 16 + newY;
        snake_paint(tail, 0xF300);
        snake_head = tail;
        snake_l--;
      }
      if (colX > 0) {
        bonus[colX - 1].type = 0;
      }

      delay(5);
    }
  }
}
