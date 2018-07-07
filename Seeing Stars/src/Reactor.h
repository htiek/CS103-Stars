#ifndef Reactor_Included
#define Reactor_Included

#include "gevents.h"
#include <string>

class Reactor {
public:
    /* Interfaces need virtual destructors. */
    virtual ~Reactor() = default;

    /* Responds to an event. */
    virtual void handleEvent(GEvent e) = 0;

    /* Returns a string representing the transition in the global state machine that
     * should be followed. This should be empty if there is no transition requested.
     *
     * TODO: Once C++17 support comes around, we should use std::optional here.
     */
    virtual std::string transition() = 0;
};

#endif
