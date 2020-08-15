#include "myTFT.h"
#include "Joystick.h"

#ifndef GameSnake_H
#define GameSnake_H

class GameSnake{

public:
  GameSnake(MyTFT* tft, char* buf1, char* buf2, Joystick* joy, int8_t soundpin, uint16_t mem);
  void Init();
  void Update();
  uint16_t getCnt();
  void Save();

private:  
  MyTFT* tft;
  char* buf1;
  char* buf2;
  uint16_t mem;
  int8_t soundpin;
  Joystick* joy;
  boolean isGameover;

  struct bonus_struct {
   uint8_t  type; // 1 - яблоко, 2 - гнилое, 3 - кристалл
   uint8_t x;
   uint8_t y;
   unsigned long tm;
 };

 unsigned long lastUpdate;

//максимальное кол-во яблок и др. бонусов на экране
#define BONUS_MAX 8
//массив бонусов
bonus_struct bonus[BONUS_MAX];
//максимальная длина змейки
#define SNAKE_MAX 128
//координата х*16+y для каждой части хвоста
uint8_t snake_xy[SNAKE_MAX];
//длина змеи
uint8_t snake_l=0;
//позиция квадратика, именуемого головой
uint8_t snake_head=0;
//направление движения по х
int8_t snake_dx;
//направление движения по у
int8_t snake_dy;
//счёт
uint16_t cnt=0;
uint16_t record=0;

/**
 * Проверяем на свободность поля по заданным координатам.
 * @param x
 * @param y
 * @return - 0 - свободная ячейка, -1 - тело змейки, > 0 - порядковвый номер ячейки в массиве бонусов (от 1 до 
 */
int8_t collision(uint8_t x,uint8_t y);

void repaintBonus();
void insertBonus(uint8_t i);
void removeBonus(uint8_t i);
void paintBlock(uint8_t x, uint8_t y, uint16_t color);
void snake_paint(uint8_t p, uint16_t color);
void gameover();
uint8_t delayGame();

};

#endif //GameSnake
