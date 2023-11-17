#include <iostream>

#include "canvas.h"

struct shape {
    int x_, y_;
    char c_;

    shape(int x, int y, char c) : x_(x), y_(y), c_(c) {}

    virtual ~shape() {}

    virtual void draw(canvas& c) const {
        c.set(x_, y_, c_);
    }
};

struct line : public shape {
    int x1_, y1_;
    std::string name_;

    line(int x0, int y0, int x1, int y1, char c) : 
        shape(x0, y0, c), x1_(x1), y1_(y1) {}

    ~line() {}

    void draw(canvas& c) const override {
        c.line(x_, y_, x1_, y1_, c_);
    }

    void set_name(const std::string& name) {
        name_ = name;
    }
};

int main() {
    std::cout << " a b c d     x\n";
    {
        canvas c(80, 25);

        auto s1 = new shape(5, 10, '%');
        auto s2 = new shape(25, 17, '*');
        auto l1 = new line(40, 9, 0, 20, 'x');
        auto l2 = new line(0, 0, 80, 21, '#');

        l1->set_name("first line");
        l2->set_name("second line");

        shape* shapes[] = { s1, s2, l1, l2 };

        for (auto& s : shapes) {
            s->draw(c);
        }

        c.out(std::cout);

        for (auto& x : shapes) {
            delete x;
        }
    }
    _CrtDumpMemoryLeaks();
    return 0;
}
