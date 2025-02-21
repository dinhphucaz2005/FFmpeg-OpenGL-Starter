//
// Created by nd-phuc on 2/21/25.
//

#include "Bar.h"

#include "extension.h"

Bar::Bar() : percent(10), color(randomColor()) {
}

Bar::Bar(const float percent) : percent(get(percent)), color(getColor()) {
}

Bar::Bar(const float percent, const Color color) : percent(get(percent)), color(color) {
}

float Bar::getPercent() const {
    return this->percent;
}

Color Bar::getColor() const {
    return this->color;
}

float Bar::get(const float percent) {
    return percent > 100 ? 100 : percent < 0 ? 0 : percent;
}
