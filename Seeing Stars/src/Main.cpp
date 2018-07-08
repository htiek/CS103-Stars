#include "Reactor.h"
#include "FreeformEditorReactor.h"
#include "AligningReactor.h"
#include "HTMLWaiterReactor.h"
#include "AnimatedStarReactor.h"
#include "RadialEditorReactor.h"
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
    ANIMATE_5,
    DONE_5,

    FIND_7,
    ANIMATE_7_case_2,
    ANIMATE_7_case_3,
    DONE_7_case_2,
    DONE_7_case_3,

    FIND_7_2,
    ANIMATE_7_2,

    FIND_7_3,
    ANIMATE_7_3,
    DONE_SECOND_7,

    DEMO_ALGORITHM,

    DRAW_11_4,
    DONE_11_4,

    DEMO_SIMPLE,

    FIND_8,
    DONE_8,

    FIND_10,
    DONE_10,

    FIND_12,
    DONE_12
};

/* Total window dimensions. */
const double kWindowWidth  = 1000;
const double kWindowHeight = 400;

/* Width allocated to the canvas. */
const double kCanvasWidth = kWindowWidth / 2.0;

struct Graphics {
    GWindow window;
    GFormattedPane* pane;
    shared_ptr<Reactor> reactor;

    State state;
};

unique_ptr<Graphics> makeGraphics() {
    unique_ptr<Graphics> result(new Graphics);

    result->window.setSize(kWindowWidth, kWindowHeight);
    result->window.setExitOnClose();

    result->pane = new GFormattedPane();
    result->pane->setSize(kWindowWidth - kCanvasWidth, kWindowHeight);
    result->window.addToRegion(result->pane, "WEST");

    result->reactor = make_shared<FreeformEditorReactor>(result->window);
    result->pane->readTextFromFile("Welcome.html");
    result->state = State::FIND_5;
    return result;
}

/* State machine logic to advance us from screen to screen. */
void handleTransition(Graphics& g) {
    if (g.state == State::FIND_5) {
        auto fer = static_pointer_cast<FreeformEditorReactor>(g.reactor);
        if (fer->type() == StarType{ 5, 2 }) {
            g.reactor = make_shared<AligningReactor>(fer->star(), fer->pointsInOrder(), fer->center());
            g.state = State::ANIMATE_5;
        }
    } else if (g.state == State::ANIMATE_5) {
        if (static_pointer_cast<AligningReactor>(g.reactor)->done()) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Finish_52.html");
            g.state = State::DONE_5;
        }
    } else if (g.state == State::DONE_5) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<FreeformEditorReactor>(g.window);
            g.pane->readTextFromFile("Find_7.html");
            g.state = State::FIND_7;
        }
    } else if (g.state == State::FIND_7) {
        auto fer = static_pointer_cast<FreeformEditorReactor>(g.reactor);
        if (fer->type() == StarType{ 7, 1 }) {
            g.pane->readTextFromFile("Find_7_1.html");
        } else if (fer->type() == StarType{ 7, 2 }) {
            g.reactor = make_shared<AligningReactor>(fer->star(), fer->pointsInOrder(), fer->center());
            g.state = State::ANIMATE_7_case_2;
        } else if (fer->type() == StarType{ 7, 3 }) {
            g.reactor = make_shared<AligningReactor>(fer->star(), fer->pointsInOrder(), fer->center());
            g.state = State::ANIMATE_7_case_3;
        }
    } else if (g.state == State::ANIMATE_7_case_2 ||
               g.state == State::ANIMATE_7_case_3) {
        if (static_pointer_cast<AligningReactor>(g.reactor)->done()) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Finish_7.html");
            g.state = (g.state == State::ANIMATE_7_case_2? State::DONE_7_case_2 :
                                                           State::DONE_7_case_3);
        }
    } else if (g.state == State::DONE_7_case_2) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<FreeformEditorReactor>(g.window);
            g.pane->readTextFromFile("Find_7_3.html");
            g.state = State::FIND_7_3;
        }
    } else if (g.state == State::DONE_7_case_3) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<FreeformEditorReactor>(g.window);
            g.pane->readTextFromFile("Find_7_2.html");
            g.state = State::FIND_7_2;
        }
    } else if (g.state == State::FIND_7_2) {
        auto fer = static_pointer_cast<FreeformEditorReactor>(g.reactor);
        if (fer->type() == StarType{ 7, 2 }) {
            g.reactor = make_shared<AligningReactor>(fer->star(), fer->pointsInOrder(), fer->center());
            g.state = State::ANIMATE_7_2;
        }
    }  else if (g.state == State::FIND_7_3) {
        auto fer = static_pointer_cast<FreeformEditorReactor>(g.reactor);
        if (fer->type() == StarType{ 7, 3 }) {
            g.reactor = make_shared<AligningReactor>(fer->star(), fer->pointsInOrder(), fer->center());
            g.state = State::ANIMATE_7_3;
        }
    } else if (g.state == State::ANIMATE_7_2 ||
               g.state == State::ANIMATE_7_3) {
         if (static_pointer_cast<AligningReactor>(g.reactor)->done()) {
             g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
             g.pane->readTextFromFile("Finish_Second_7.html");
             g.state = (g.state == State::ANIMATE_7_2? State::DONE_SECOND_7 :
                                                       State::DONE_SECOND_7);
         }
    } else if (g.state == State::DONE_SECOND_7) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<HTMLWaiterReactor>(make_shared<AnimatedStarReactor>(g.window, StarType{ 7, 2 }));
            g.pane->readTextFromFile("Algorithm.html");
            g.state = State::DEMO_ALGORITHM;
        }
    } else if (g.state == State::DEMO_ALGORITHM) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<RadialEditorReactor>(g.window, 11);
            g.pane->readTextFromFile("Draw_11_4.html");
            g.state = State::DRAW_11_4;
        }
    } else if (g.state == State::DRAW_11_4) {
        if (static_pointer_cast<RadialEditorReactor>(g.reactor)->type() == StarType{ 11, 4 }) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Finish_11_4.html");
            g.state = State::DONE_11_4;
        }
    } else if (g.state == State::DONE_11_4) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<HTMLWaiterReactor>(make_shared<AnimatedStarReactor>(g.window, StarType{ 8, 2 }));
            g.pane->readTextFromFile("Simple.html");
            g.state = State::DEMO_SIMPLE;
        }
    } else if (g.state == State::DEMO_SIMPLE) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<RadialEditorReactor>(g.window, 8);
            g.pane->readTextFromFile("Find_8.html");
            g.state = State::FIND_8;
        }
    } else if (g.state == State::FIND_8) {
        auto rer = static_pointer_cast<RadialEditorReactor>(g.reactor);
        if (rer->type() == StarType{ 8, 1 }) {
            g.pane->readTextFromFile("Find_8_1.html");
        } else if (rer->type() == StarType{ 8, 3 }) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Find_8_3.html");
            g.state = State::DONE_8;
        }
    } else if (g.state == State::DONE_8) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<RadialEditorReactor>(g.window, 10);
            g.pane->readTextFromFile("Find_10.html");
            g.state = State::FIND_10;
        }
    } else if (g.state == State::FIND_10) {
        auto rer = static_pointer_cast<RadialEditorReactor>(g.reactor);
        if (rer->type() == StarType{ 10, 3 }) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Finish_10.html");
            g.state = State::DONE_10;
        }
    } else if (g.state == State::DONE_10) {
        if (static_pointer_cast<HTMLWaiterReactor>(g.reactor)->done()) {
            g.reactor = make_shared<RadialEditorReactor>(g.window, 12);
            g.pane->readTextFromFile("Find_12.html");
            g.state = State::FIND_12;
        }
    } else if (g.state == State::FIND_12) {
        auto rer = static_pointer_cast<RadialEditorReactor>(g.reactor);
        if (rer->type() == StarType{ 12, 5 }) {
            g.reactor = make_shared<HTMLWaiterReactor>(g.reactor);
            g.pane->readTextFromFile("Finish_12.html");
            g.state = State::DONE_12;
        }
    }
}

int main() {
    auto graphics = makeGraphics();

    while (true) {
        graphics->reactor->handleEvent(waitForEvent());
        handleTransition(*graphics);
    }
}
