#pragma once
#include <Arduino.h>

// Tic Tac Toe room data structure.
struct Room {
  bool used;
  char token[5];
  char pidX[9];
  char pidO[9];
  String nameX;
  String nameO;
  char board[10];
  char turn;      // 'X' or 'O'
  char winner;    // 0 = none, 'X', 'O', 'D' = draw
  char starter;   // who starts the current round
  int8_t line[3];
  uint16_t sx, so, d;
  uint32_t v;     // version, goes up on every change
  uint32_t seenX, seenO;
  bool hasO;
};
