#include "Reactor.h"
#include "FreeformEditorReactor.h"
#include "AligningReactor.h"
#include "HTMLWaiterReactor.h"
#include "AnimatedStarReactor.h"
#include "RadialEditorReactor.h"
#include "StateMachine.h"
#include "Grabbag.h"
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

namespace {
    /* Total window dimensions. */
    const double kWindowWidth  = 1000;
    const double kWindowHeight = 800;

    /* Width allocated to the canvas. */
    const double kCanvasWidth = kWindowWidth / 2.0;

    /* Constant representing the comment to look for when injecting a new piece of content. */
    const string kInjectionSite   = "<!-- Inject ";
    const string kInjectionCloser = "-->";

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

    /* Given a string, replaces all injection sites with the appropriate contents. */
    string replaceInjectionSitesIn(const string& str, const Grabbag& grabbag) {
        string result;

        size_t index = 0;
        while (true) {
            /* See if we can find an injection site. */
            size_t site = str.find(kInjectionSite, index);

            /* If there isn't one, then whatever we've assembled so far, plus the rest
             * of the string, is the final result.
             */
            if (site == string::npos) return result + str.substr(index);

            /* Otherwise, we found an injection site. Everything up to this point is
             * perfectly safe to add in.
             */
            result += string(str.begin() + index, str.begin() + site);

            /* Otherwise, we found a site. See what to replace it with. */
            size_t endpoint = str.find(kInjectionCloser, site);
            if (endpoint == string::npos) error("Unterminated injection site?");

            /* Get the filename from this range. */
            string filename(str.begin() + site + kInjectionSite.size(),
                            str.begin() + endpoint);

            /* Recursively replace injections within that file and append that to the result. */
            result += replaceInjectionSitesIn(grabbag.contentsOf(trim(filename)), grabbag);

            /* Scoot past this point in the file. */
            index = endpoint + kInjectionCloser.size();
        }
    }

    /* Data sourcing function for a Grabbag. */
    StateReader grabbagReader(const string& grabbagFile) {
        ifstream input(grabbagFile);
        if (!input) error("Cannot open grabbag file " + grabbagFile);

        Grabbag grabbag(input);

        return [grabbag](const string& filename) {
            string text = grabbag.contentsOf("states/" + filename + ".state");

            /* TODO: With C++14 support, use make_unique. */
            return unique_ptr<istringstream>(new istringstream(replaceInjectionSitesIn(text, grabbag)));
        };
    }

    shared_ptr<StateMachine> createStateMachine() {
        StateMachineBuilder builder(makeGraphics(), "Welcome", grabbagReader("assignment.grabbag"));

        AligningReactor::installHandlers(builder);
        AnimatedStarReactor::installHandlers(builder);
        FreeformEditorReactor::installHandlers(builder);
        HTMLWaiterReactor::installHandlers(builder);
        RadialEditorReactor::installHandlers(builder);

        return builder.build();
    }
}

int main() {
    auto stateMachine = createStateMachine();

    while (true) {
        stateMachine->handleEvent(waitForEvent());
    }
}
