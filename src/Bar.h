//
// Created by nd-phuc on 2/21/25.
//

#ifndef BAR_H
#define BAR_H
#include <raylib.h>


class Bar {
public:
    explicit Bar();
    explicit Bar(float percent);
    explicit Bar(float percent, Color color);


    float getPercent() const;

    Color getColor() const;
    static constexpr int maxPercent = 100;

private:
    float percent; // 0-100
    Color color;
    static float get(float percent);
};


#endif //BAR_H
