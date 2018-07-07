#include "Reactor.h"
#include "TabulaRasaReactor.h"
#include "gwindow.h"
#include "gobjects.h"
#include "gevents.h"
#include "ginteractors.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <set>
using namespace std;

enum class State {
    FIND_5,
    FIND_7,
    FIND_7_2,
    FIND_7_3,
    FIND_8
};

const double kWindowWidth  = 600;
const double kWindowHeight = 400;

struct Graphics {
    GWindow window;
    shared_ptr<Reactor> reactor;

    State state = State::FIND_5;
};

unique_ptr<Graphics> makeGraphics() {
    unique_ptr<Graphics> result(new Graphics);

    result->window.setCanvasSize(kWindowWidth, kWindowHeight);
    result->window.setExitOnClose();

    result->reactor = make_shared<TabulaRasaReactor>(result->window, vector<Star>{ { 5, 2 } });

    return result;
}

/* Responds to a transition. */
void handleTransition(Graphics& g) {
    switch (g.state) {
    case State::FIND_5:
        if (g.reactor->transition() == "{ 5 / 2 }") {
            g.reactor = make_shared<TabulaRasaReactor>(g.window, vector<Star>{ { 7, 2 }, { 7, 3 }});
            g.state = State::FIND_7;
        }
        break;
    case State::FIND_7:
        if (g.reactor->transition() == "{ 7 / 2 }") {
            g.reactor = make_shared<TabulaRasaReactor>(g.window, vector<Star>{ { 7, 3 }});
            g.state = State::FIND_7_3;
        } else if (g.reactor->transition() == "{ 7 / 3 }") {
            g.reactor = make_shared<TabulaRasaReactor>(g.window, vector<Star>{ { 7, 2 }});
            g.state = State::FIND_7_2;
        }
        break;
    case State::FIND_7_2:
        if (g.reactor->transition() == "{ 7 / 2 }") {
            g.reactor = make_shared<TabulaRasaReactor>(g.window, vector<Star>{{ 8, 3 }});
            g.state = State::FIND_8;
        }
        break;
    case State::FIND_7_3:
        if (g.reactor->transition() == "{ 7 / 3 }") {
            g.reactor = make_shared<TabulaRasaReactor>(g.window, vector<Star>{{ 8, 3 }});
            g.state = State::FIND_8;
        }
        break;
    default:
        /* Do nothing */
        break;
    }
}


int main() {
    auto graphics = makeGraphics();

    while (true) {
        graphics->reactor->handleEvent(waitForEvent());
        handleTransition(*graphics);
    }
}
