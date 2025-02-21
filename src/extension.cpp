//
// Created by nd-phuc on 2/21/25.
//
#include "extension.h"

#include <iterator>

#include "Bar.h"

Bar extLerp(const Bar a, const Bar b, const float t) {
    const int percent = static_cast<int>(a.getPercent() + t * (b.getPercent() - a.getPercent()));
    return Bar(percent, b.getColor());
}

Color randomColor() {
    return Color(GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255));
}

Color tempColors[] = {
    GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE,
    VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN
};

Bar getRandBar(const Color color) {
    return Bar(GetRandomValue(1, 100), tempColors[GetRandomValue(0, std::size(tempColors))]);
}
