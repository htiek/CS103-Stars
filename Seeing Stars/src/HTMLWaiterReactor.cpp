#include "HTMLWaiterReactor.h"
using namespace std;

HTMLWaiterReactor::HTMLWaiterReactor(shared_ptr<Reactor> previous) : previous(previous) {
    // Handled by initializer
}

void HTMLWaiterReactor::handleEvent(GEvent e) {
    if (e.getEventClass() == HYPERLINK_EVENT &&
        GHyperlinkEvent(e).getUrl() == "next") {
        isDone = true;
    } else {
        previous->handleEvent(e);
    }
}

bool HTMLWaiterReactor::done() const {
    return isDone;
}
