#include "Reactor.h"
#include "FreeformEditorReactor.h"
#include "AligningReactor.h"
#include "HTMLWaiterReactor.h"
#include "AnimatedStarReactor.h"
#include "RadialEditorReactor.h"
#include "StateMachine.h"
#include "gwindow.h"
#include "gobjects.h"
#include "gevents.h"
#include "ginteractors.h"
#include <iostream>
#include <fstream>
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

/* Total window dimensions. */
const double kWindowWidth  = 1000;
const double kWindowHeight = 400;

/* Width allocated to the canvas. */
const double kCanvasWidth = kWindowWidth / 2.0;

/* Constructs the graphics system. */
shared_ptr<GraphicsSystem> makeGraphics() {
    shared_ptr<GraphicsSystem> result = make_shared<GraphicsSystem>();

    result->window.setSize(kWindowWidth, kWindowHeight);
    result->window.setExitOnClose();

    result->pane = new GFormattedPane();
    result->pane->setSize(kWindowWidth - kCanvasWidth, kWindowHeight);
    result->window.addToRegion(result->pane, "WEST");
    return result;
}

/* Data is sourced from disk. */
unique_ptr<istream> readState(const string& state) {
    /* TODO: With C++14 support, use make_unique. */
    unique_ptr<istream> result(new ifstream(state + ".state"));
    if (!*result) error("Could not load file " + state + ".state");

    return result;
}

shared_ptr<StateMachine> createStateMachine() {
    StateMachineBuilder builder(makeGraphics(), "Welcome", readState);

    AligningReactor::installHandlers(builder);
    AnimatedStarReactor::installHandlers(builder);
    FreeformEditorReactor::installHandlers(builder);
    HTMLWaiterReactor::installHandlers(builder);
    RadialEditorReactor::installHandlers(builder);

    return builder.build();
}

int main() {
    auto stateMachine = createStateMachine();

    while (true) {
        stateMachine->handleEvent(waitForEvent());
    }
}
