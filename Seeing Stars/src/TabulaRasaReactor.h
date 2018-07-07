#ifndef TabulaRasaReactor_Included
#define TabulaRasaReactor_Included

/* A reactor that gives the student a blank canvas to explore and draw stars in.
 * The reported transition at each point is either
 *
 * 1. "{ p / s }" if the { p / s } star has been found.
 * 2. "Star Graph" if the star drawn is actually a star graph.
 * 3. [[ todo: others? ]]
 */

#include "Reactor.h"
#include "Star.h"
#include "gwindow.h"
#include "gobjects.h"
#include "gtimer.h"
#include <vector>
#include <set>
#include <utility>

class TabulaRasaReactor: public Reactor {
public:
    TabulaRasaReactor(GWindow& window, const std::vector<Star>& permitted);
    ~TabulaRasaReactor();

    void handleEvent(GEvent e) override;
    std::string transition() override;

    using Line = std::pair<size_t, size_t>;

private /* Types */:
    enum class State {
        DRAWING,    // Still playing around and trying to find the star.
        ANIMATING,  // Animating everything into place.
        DONE        // Finished and ready.
    };

private /* Helpers */:
    std::size_t addPoint(double x, double y);
    void addActiveLine(double x, double y);
    void handleMouseEvent(GMouseEvent e);
    void handleTimerEvent(GTimerEvent e);
    void startAnimation();

private /* State */:
    /* Reference to the containing window. */
    GWindow& window;

    /* List of stars we're permitted to terminate on. */
    const std::vector<Star> permitted;

    /* List of logical circle centers, ordered by insertion time. */
    std::vector<GPoint> logicalPoints;

    /* Collection of completed lines, ordered by insertion time. */
    std::vector<Line> logicalLines;

    /* List of actual graphics objects. */
    std::vector<GOval*> graphicsPoints;
    std::vector<GLine*> graphicsLines;

    /* Target coordinates for each point. */
    std::vector<GPoint> pointTargets;

    /* Anchor point for the current line, if any. */
    std::size_t origin;

    /* Currently-drawn line, if any. */
    GLine* active = nullptr;

    /* Current state. */
    State state = State::DRAWING;

    /* Underlying star type. Only valid during the ANIMATING or DONE states. */
    Star type;

    /* Timer, used to tick the animation. */
    GTimer timer;

    /* Number of animation frames advanced so far. */
    std::size_t frame = 0;
};

#endif
