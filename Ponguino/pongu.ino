/*
   __    _                        _ _        __  
  / /   (_)___  ___ _ __ ___   __| (_) ___   \ \ 
 / /    | / __|/ _ \ '__/ _ \ / _` | |/ _ \   \ \
 \ \    | \__ \  __/ | | (_) | (_| | | (_) |  / /
  \_\  _/ |___/\___|_|  \___/ \__,_|_|\___/  /_/ 
      |__/                                       

  @author jserodio <serodio.jose@protonmail.ch>

  MIT License

  Copyright (c) 2017 Jose Serodio Ignacio

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

 
*/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

// Screen
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

// Screen limit
const uint8_t LCD_WIDTH = 128;
const uint8_t LCD_HEIGHT = 64;

// FPS measuring variables
uint8_t fps = 0;
long lastTime = 0;
long frameCount = 0;
long deltaTime = 0;
  
// JoyStick Pins
const int X_PIN = A0; // x - Analogic pin A0
const int Y_PIN = A1; // y - Analogic pin A1
const int S_PIN = 2; // s - Digital pin 2
bool s;


// Pad variables
const uint8_t PAD_W = 4; // pad width
const uint8_t PAD_H = 12; // pad height

// Player 1
// int x = 0; uncomment only if using x-axis in Joystick
const uint8_t x = 0; // comment only if moving x-axis in Joystick
int y = 24;

// Player 2
const uint8_t x2 = 122;
int y2 = 24;

// Ball variables
float b_speed = 0.3;
float b_x = 64;
float b_y = 32;
float b_angle = 0.7854;

// Global
const uint8_t LIVES = 3;
uint8_t p1_lives = LIVES;
uint8_t p2_lives = LIVES;
bool pause = false;
bool gameWin = false;
bool gameLost = false;

void setup(void)
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_timB08_tf);
  u8g2.setFontMode(0);
  pinMode (S_PIN, INPUT);
  digitalWrite (S_PIN, HIGH);
}

void drawPad()
{
  //  u8g2.drawBox(x, y, w, h); // pad variables
  u8g2.drawBox(0, y, PAD_W, PAD_H); // pad variables
  u8g2.drawBox(x2, y2, PAD_W, PAD_H); // pad variables
}

void drawBall()
{
  moveBall(); // x, y, radio, options
  u8g2.drawCircle(b_x, b_y, 1, U8G2_DRAW_ALL);
}

void drawHUD()
{
  u8g2.setCursor(10, 15);
  u8g2.print(p1_lives);
  
  u8g2.setCursor(115, 15);
  u8g2.print(p2_lives);
}

void moveBall()
{
  float incX = 0.0;
  float incY = 0.0;

  incX = b_speed * cos(b_angle);
  incY = b_speed * sin(b_angle);

  // b_x = b_x + calcdisttomove(incX);
  b_x = b_x + incX;
  if (b_y > 0 && b_y < LCD_WIDTH) {
    // b_y = b_y - calcdisttomove(incY);
    b_y = b_y - incY;
  }

  //  Printing variables for developing purposes
  //  u8g2.drawStr(100, 15, "incX");
  //  u8g2.setCursor(90, 15);
  //  u8g2.print(incX);
  //  u8g2.drawStr(100, 25, "incY");
  //  u8g2.setCursor(90, 25);
  //  u8g2.print(incY);
}

// float calcdisttomove(float inc)
// {
//   return (inc * deltaTime) / 1000;
// }

void colision()
{
  if (b_y < 0)  // top border limit
  { 
    b_y = 1;
    b_angle = -b_angle;
  }
  if (b_y >= LCD_HEIGHT)  // bottom border limit
  { 
    b_y = b_y - 2;
    b_angle = -b_angle;
  }
  if ((b_x + 1) >= x2 ) // right rectangle limit
  {
    if (((b_y+2) >= y2) && (b_y <= (y2+PAD_H)))
    { 
      b_x = b_x - 2;
      b_angle = -b_angle + 3.1416;
      b_speed += 0.08;
    }
  }

  if ((b_x) <= (PAD_W) ) // left rectangle limit
  {
    if ( ((b_y+2) >= y) && (b_y <= (y+PAD_H)) )
    { 
      b_x = b_x + 2;
      b_angle = -b_angle + 3.1416;
      b_speed += 0.08;
    }
  }

  if (b_x > LCD_WIDTH)
  {
    p2_lives--;
    resetBall();
  }
  
  if (b_x <= 0)
  {
    p1_lives--;
    resetBall();
  }

  if (p1_lives == 0) {
    gameLost = true;
    pause = true;
  }

  if (p2_lives == 0) {
    gameWin = true;
    pause = true;
  }
}

void resetBall()
{
  b_x = (LCD_WIDTH/2)-1;
  b_y = (LCD_HEIGHT/2)-1;
  b_speed = 0.5;
}

void playerMove()
{
  if ( (b_y + 2) <= (y2+random(0,3)) ) {
    y2-=random(0,3);  // making enemy stupid
  }
  if ( b_y >= (y2+PAD_H+random(0,3)) ) {
    y2+=random(0,3);  // making enemy stupid
  }
  //y2 = b_y - PAD_H/2; // uncomment only if unbeatable enemy
}

void measureFPS()
{
  deltaTime = millis() - lastTime;

  if (deltaTime > 1000)
  { // 1 second elapsed
    fps = frameCount;
    frameCount = 0;
    lastTime = millis();
  }
  frameCount++;
  
  //printFPS(&fps); // comment this line to hide frames per second
}

void printFPS(uint8_t *fps)
{
  // Print FPS
  u8g2.drawStr(10, 15, "FPS");
  u8g2.setCursor(0, 15);
  u8g2.print(*fps);
}

void readJoy()
{
  //x = analogRead(X_PIN);
  y = analogRead(Y_PIN);
  //x = evalX(x);
  y = evalY(y);

  s = digitalRead(S_PIN);
  
  if (!s)
  { // if the Joystick button is pressed
    // this delay is needed so it doesn't toogle pause too quickly
    delay(350);
    pause = !pause;
    if (gameLost || gameWin) {
      p1_lives = LIVES;
      p2_lives = LIVES;
      gameLost = false;
      gameWin = false;
    }
  }

  // Print Joy value
  //  u8g2.drawStr(60, 15, "X");
  //  u8g2.setCursor(40, 15);
  //  u8g2.print(x);
  //
  //  // Print Joy value
  //  u8g2.drawStr(60, 25, "Y");
  //  u8g2.setCursor(40, 25);
  //  u8g2.print(y);
}

// transform joystick input into a coordinate
int evalX(int data)
{ // formula: (x * LCD_WIDTH / 768px, wich is Joystick's maximum value) - PAD_W / 2
  return (data * LCD_WIDTH / 768) - PAD_W / 2;
}

int evalY(int data)
{
  return 64 - (data * LCD_HEIGHT / 768) - PAD_H / 2;
}

void loop(void)
{
  u8g2.clearBuffer(); // Clears all pixel in the memory frame buffer
  readJoy(); // read Joystick
  measureFPS(); // measure delta time and frames per second
  if (!pause) {
    colision(); // detect colision and make the ball bounce
    playerMove(); // make the enemy pad move with the ball
    drawPad(); // draw pads
    drawBall(); // draw ball
    drawHUD(); // draw lives
  } else {
    if (gameWin) {
      u8g2.drawStr(LCD_WIDTH/2-26, LCD_HEIGHT/2+4, "YOU WON");
    } else {
      if (gameLost) {
        u8g2.drawStr(LCD_WIDTH/2-31, LCD_HEIGHT/2+4, "GAME OVER");
      }
      else {
        y = b_y - PAD_H/2;
        y2 = b_y - PAD_H/2;
        colision(); // detect colision and make the ball bounce
        drawPad(); // draw pads
        drawBall(); // draw ball
        u8g2.drawStr(LCD_WIDTH/2-12, LCD_HEIGHT/6+4, "Pause");
        drawHUD();
      }
    }
  }
  u8g2.sendBuffer();  // Send the content of the memory frame buffer to the display
}