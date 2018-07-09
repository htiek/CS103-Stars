#ifndef StateMachine_Included
#define StateMachine_Included

#include "Reactor.h"
#include "ginteractors.h"
#include "gwindow.h"
#include "gevents.h"
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <istream>

/* Type: GraphicsSystem
 *
 * Type representing the graphics system used by the program.
 */
struct GraphicsSystem {
    GWindow window;
    GFormattedPane* pane;
};

/* Type: ReactorConstructor
 *
 * Function that constructs a reactor given a string description of the reactor to construct
 * and the previous reactor that was installed.
 */
using ReactorConstructor =
  std::function<std::shared_ptr<Reactor> (GraphicsSystem&,
                                          const std::string&,
                                          std::shared_ptr<Reactor>)>;

/* Type: Transition
 *
 * Function that takes as input the current reactor, then returns the name of the next state
 * to transition to, or the empty string if no transition is requested.
 *
 * TODO: Once we get C++17 support, we should change this to use std::optional<std::string>.
 */
using Transition =
  std::function<std::string (std::shared_ptr<Reactor>)>;

/* Type: TransitionConstructor
 *
 * Function that constructs a transition given the arguments to the transition.
 * TransitionConstructors are stored hierarchically and are associated with a specific type of
 * Reactor.
 */
using TransitionConstructor =
  std::function<Transition (const std::string &)>;

/* Type: StateReader
 *
 * Function that takes in a string and returns an istream that can pull data from that source.
 */
using StateReader =
  std::function<std::unique_ptr<std::istream> (const std::string&)>;

/* Type: StateMachine
 *
 * Type that represents a state machine. Drives the execution of a particular state machine.
 */
class StateMachine {
public:
    void handleEvent(GEvent e);

private /* helpers */:
    friend class StateMachineBuilder;
    StateMachine() = default;

    /* Changes state. */
    void setState(const std::string& state);

    /* Parses individual lines of the config script. */
    void setHTML(std::istream& command);
    void setReactor(std::istream& command, std::string& reactorName);
    void addTransition(std::istream& command, const std::string& reactorName);

private /* state */:
    std::shared_ptr<Reactor> reactor;
    std::vector<Transition>  transitions;

    std::unordered_map<std::string, ReactorConstructor> reactorConstructors;
    std::unordered_map<std::string,
      std::unordered_map<std::string, TransitionConstructor>
    > transitionConstructors;

    std::shared_ptr<GraphicsSystem> graphics;

    StateReader reader;
};

class StateMachineBuilder {
public:
    StateMachineBuilder(std::shared_ptr<GraphicsSystem> graphics,
                        const std::string& initialState,
                        StateReader reader);

    void addReactor   (const std::string& name, ReactorConstructor constructor);
    void addTransition(const std::string& reactor,
                       const std::string& name,
                       TransitionConstructor constructor);

    std::shared_ptr<StateMachine> build() const;

private:
    StateMachine result;
    std::string initialState;
};

#endif
