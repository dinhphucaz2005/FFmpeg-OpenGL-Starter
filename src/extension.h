//
// Created by nd-phuc on 2/21/25.
//
#ifndef EXTENSION_H
#define EXTENSION_H

#include "Bar.h"
#include "raylib.h"

Bar extLerp(Bar a, Bar b, float t);
Color randomColor();
Bar getRandBar(Color color= randomColor());

#endif //EXTENSION_H
