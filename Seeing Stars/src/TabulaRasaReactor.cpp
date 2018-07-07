#include "TabulaRasaReactor.h"
#include "Star.h"
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
using namespace std;

namespace {
    struct GraphicsParameters {
        double lineWidth;
        string fillColor;
    };

    struct FilledGraphicsParameters {
        double lineWidth;
        string fillColor;
        string borderColor;
    };

    const double kPointRadius = 10;
    const FilledGraphicsParameters kPointParams = {
        2,
        "#000080",
        "#000040"
    };

    const GraphicsParameters kActiveLineParams = {
        4,
        "Red"
    };

    const GraphicsParameters kOldLineParams = {
        4,
        "#0000FF"
    };

    /* Padding from the sides of the window to the star points. */
    const double kWindowPadding = 25;

    const double kTimerDelay = 25; // ms

    /* Number of frames in the animation. */
    const size_t kAnimationFrames = 2000 / kTimerDelay;

    /* Sentinel value indicating "not actually an index. */
    const size_t kNotAPoint = numeric_limits<size_t>::max();

    /* Given a set of graphics attributes, applies those attributes to the specified object. */
    void setAttributesOf(GObject* gObj, const GraphicsParameters& params) {
        gObj->setLineWidth(params.lineWidth);
        gObj->setColor(params.fillColor);
    }

    /* The different graphics objects that are fillable don't inherit from a common type,
     * so we'll use the template system to find the right methods to call.
     */
    template <typename GType>
    void setAttributesOf(GType* gObj, const FilledGraphicsParameters& params) {
        gObj->setLineWidth(params.lineWidth);
        gObj->setColor(params.borderColor);
        gObj->setFilled(true);
        gObj->setFillColor(params.fillColor);
    }


    /* Given a collection of points, returns our best guess for where the center should be. */
    GPoint centerOf(const vector<GPoint>& pts) {
        /* Return the barycenter. TODO: Investigate better ways to do this? */
        double x = 0;
        double y = 0;

        for (const auto& pt: pts) {
            x += pt.getX();
            y += pt.getY();
        }

        return { x / pts.size(), y / pts.size() };
    }

    /* Returns the angle formed by moving from the specified center point to the given
     * peripheral point.
     */
    double angleBetween(const GPoint& center, const GPoint& dest) {
        double dx = dest.getX() - center.getX();
        double dy = dest.getY() - center.getY();

        return atan2(dy, dx);
    }

    /* Given a collection of points, returns the order in which they appear around the perimeter
     * of a circle - or, at least, our best guess.
     */
    vector<size_t> orderOf(const vector<GPoint>& pts) {
        GPoint center = centerOf(pts);

        /* Sort the points by the angle they make with the circle center. */
        vector<size_t> result(pts.size());
        iota(result.begin(), result.end(), 0);

        sort(result.begin(), result.end(), [&](size_t lhs, size_t rhs) {
            return angleBetween(center, pts[lhs]) < angleBetween(center, pts[rhs]);
        });

        return result;
    }

    using Graph = unordered_map<size_t, unordered_set<size_t>>;

    /* Given a list of points in circle order and a list of edges in insertion order,
     * constructs an graph representation of those points.
     */
    Graph asGraph(const vector<size_t>& order, const vector<TabulaRasaReactor::Line>& lines) {
        /* Construct a map from insertion order to circle order. We'll need to this to transform
         * our lines. For example, if the circle order is
         *
         *     4 1 0 3 2
         *
         * we'll want this to have the order
         *
         *     2 1 4 3 0
         *
         * since that maps item 0 to circle position 2, item 1 to circle position 1, etc.
         */
        unordered_map<size_t, size_t> toOrder(order.size());
        for (size_t i = 0; i < order.size(); i++) {
            toOrder[order[i]] = i;
        }

        /* Convert the lines to a graph, applying the transformation above, to get everything into
         * the proper circular ordering system.
         */
        unordered_map<size_t, unordered_set<size_t>> graph;
        for (auto line: lines) {
            graph[toOrder[line.first]].insert(toOrder[line.second]);
            graph[toOrder[line.second]].insert(toOrder[line.first]);
        }

        return graph;
    }

    /* Given a graph, guesses an origin point and step size for that graph. */
    tuple<size_t, size_t> stepSizeGuessFor(const Graph& graph) {
        /* Find some point that has edges leaving it, and some point it's adjacent to. */
        size_t source = graph.size();
        size_t target;
        for (const auto& entry: graph) {
            if (!entry.second.empty()) {
                source = entry.first;
                target = *entry.second.begin();
                break;
            }
        }

        if (source >= graph.size()) error("Internal logic error: no edges found?");

        size_t stepSize;
        if (target > source) stepSize = target - source;
        else stepSize = source - target;

        /* Pick either this value, or the mirror value, whichever is smaller. */
        return make_tuple(source, min(stepSize, graph.size() - stepSize));
    }

    /* Determines the type of the current star and the circle ordering of each of the points. The
     * returned vector maps each point to its circle index and is only valid if the points form
     * a star.
     */
    Star starTypeOf(const vector<GPoint>& points, const vector<TabulaRasaReactor::Line>& lines) {
        /* If there are no points, there isn't a star. */
        if (points.size() == 0) return kNotAStar;

        /* If there is exactly one point, it's the 1/1 star. */
        if (points.size() == 1) return { 1, 1 };

        /* If there are no lines at all, this is the { p, 0 } star. */
        if (lines.empty()) return { points.size(), 0 };

        /* If there are two points, this is the { 2 / 1 } star because we know there's at least
         * one edge.
         */
        if (points.size() == 2) return { 2, 1 };

        /* Order the points in our best guess of a circle. */
        auto order = orderOf(points);

        /* Convert the edge list in insertion order into an adjacency list in circle order. */
        auto graph = asGraph(order, lines);

        /* Get a guess of the step size. */
        size_t source, stepSize;
        tie(source, stepSize) = stepSizeGuessFor(graph);

        /* Confirm that this is a valid step size by seeing if we can trace around the circle.
         * As we do, we'll keep track of how many lines we visited.
         */
        size_t linesVisited = 0;
        size_t curr = source;
        do {
            size_t next = (curr + stepSize) % points.size();

            if (!graph[curr].count(next)) return kNotAStar;

            curr = next;
            linesVisited++;
        } while (curr != source);

        /* If we visited every line, we have a star! */
        if (linesVisited == lines.size()) return { points.size(), stepSize };

        return kNotAStar;
    }

    /* Given an x/y coordinate, returns the index of the star point it corresponds to. */
    size_t pointAt(const vector<GPoint>& points, double x, double y) {
        for (size_t i = 0; i < points.size(); i++) {
            double dx = points[i].getX() - x;
            double dy = points[i].getY() - y;

            if (dx * dx + dy * dy <= kPointRadius * kPointRadius) return i;
        }
        return kNotAPoint;
    }

    /* Maps a list of destination point angles to destination points by placing them
     * onto an appropriately-sized circle.
     */
    vector<GPoint> destinationsFor(const vector<double>& angles, double width, double height) {
        /* Compute the radius to use. This will be chosen so that the resulting figure
         * takes up the full canvas, minus the padding.
         */
        double radius = min(width, height) / 2.0 - kPointRadius - kWindowPadding;
        double centerX = width  / 2.0;
        double centerY = height / 2.0;

        vector<GPoint> result;
        for (auto angle: angles) {
            result.push_back({
                centerX + radius * cos(angle),
                centerY + radius * sin(angle)
            });
        }
        return result;
    }

    /* Given a number of points, returns the angles at which those points should end up getting
     * placed in the final layout.
     */
    vector<double> destinationAnglesFor(size_t numPoints) {
        /* Angle between points.
         *
         * We want to move clockwise, so we'd normally take a negative step. However,
         * we are also inverting the y axis, so by moving counterclockwise backwards,
         * we end up taking clockwise steps.
         */
        double thetaStep = 2 * M_PI / numPoints;

        /* Base angle. We want the first point to be at the bottom and slightly offset
         * from the vertical.
         */
        double thetaBase = 3 * M_PI / 2;

        /* Compute a layout of where everything goes. */
        vector<double> result;
        for (size_t i = 0; i < numPoints; i++) {
            result.push_back(thetaBase + thetaStep * i);
        }
        return result;
    }

    /* Interpolation function: given a real number in [0, 1], returns a new
     * real number in the range [0, 1] that maps to a smoother transition.
     *
     * This is the "smoothstep" function.
     */
    double interpolate(double alpha) {
        return -2 * alpha * alpha * alpha + 3 * alpha * alpha;
    }
}

/* Creates a new point in the graphics space, returning its index. */
size_t TabulaRasaReactor::addPoint(double x, double y) {
    /* Add the point to our internal list of points. */
    logicalPoints.push_back({ x, y });

    /* Create a new graphics object to represent that point. */
    GOval* point = new GOval(x - kPointRadius, y - kPointRadius, 2 * kPointRadius, 2 * kPointRadius);
    setAttributesOf(point, kPointParams);
    window.add(point);

    graphicsPoints.push_back(point);
    return logicalPoints.size() - 1;
}

/* Creates a new active line in the graphics window. */
void TabulaRasaReactor::addActiveLine(double x, double y) {
    active = new GLine(x, y, x, y);
    setAttributesOf(active, kActiveLineParams);

    /* Add this to the display, then shove it down below the points. */
    window.add(active);
    for (size_t i = 0; i < logicalPoints.size(); i++) {
        active->sendBackward();
    }
}

string TabulaRasaReactor::transition() {
    if (state == State::DONE) {
        return to_string(type);
    }
    return "";
}

void TabulaRasaReactor::handleEvent(GEvent e) {
    if (e.getEventClass() == MOUSE_EVENT) {
        handleMouseEvent(GMouseEvent(e));
    } else if (e.getEventClass() == TIMER_EVENT) {
        handleTimerEvent(GTimerEvent(e));
    }
}

void TabulaRasaReactor::handleMouseEvent(GMouseEvent e) {
    /* We completely ignore mouse events unless we're drawing. */
    if (state != State::DRAWING) return;

    /* Press: Initiate a new line. */
    if (e.getEventType() == MOUSE_PRESSED) {
        /* Ensure there's a source point. If there isn't, go make one. */
        auto source = pointAt(logicalPoints, e.getX(), e.getY());
        if (source == kNotAPoint) source = addPoint(e.getX(), e.getY());

        /* Mark this as the start / end point. */
        origin = source;
        addActiveLine(logicalPoints[source].getX(), logicalPoints[source].getY());
    }
    /* Drag: Move the endpoint. */
    else if (e.getEventType() == MOUSE_DRAGGED) {
        active->setEndPoint(e.getX(), e.getY());
    }
    /* Release: Finalize the line, if appropriate. */
    else if (e.getEventType() == MOUSE_RELEASED) {
        /* See whether we know the endpoint. If not, create a new endpoint. */
        auto dest = pointAt(logicalPoints, e.getX(), e.getY());
        if (dest == kNotAPoint) {
            addPoint(e.getX(), e.getY());
            dest = logicalPoints.size() - 1;
        }

        /* Enforce the invariant that the source is always less than the destination. */
        if (origin > dest) swap(origin, dest);

        /* If this is a new line and not a degenerate case, go add it. */
        auto line = make_pair(origin, dest);
        if (origin != dest && find(logicalLines.begin(), logicalLines.end(), line) == logicalLines.end()) {
            logicalLines.push_back(line);
            graphicsLines.push_back(active);
            setAttributesOf(active, kOldLineParams);

            /* See if we just completed a star. If so, if it's one of the permitted types,
             * configure ourselves for the animation.
             */
            type = starTypeOf(logicalPoints, logicalLines);
            if (find(permitted.begin(), permitted.end(), type) != permitted.end()) {
                startAnimation();
            }
        }
        /* If not, remove this line from the display. */
        else {
            window.remove(active);
            delete active;
        }

        /* Either way, deactivate the current line. */
        origin = kNotAPoint;
    }
}

void TabulaRasaReactor::handleTimerEvent(GTimerEvent e) {
    /* We shouldn't do anything if we aren't in an animation. */
    if (state != State::ANIMATING) return;
    if (e.getGTimer() != timer) return;

    /* Advance the animation one step forward. */
    frame++;

    /* Compute our interpolation factor. */
    double alpha = interpolate(frame / double(kAnimationFrames));

    /* Compute each point's new positions. */
    vector<GPoint> positions;
    for (size_t i = 0; i < logicalPoints.size(); i++) {
        double x = logicalPoints[i].getX() + alpha * (pointTargets[i].getX() - logicalPoints[i].getX());
        double y = logicalPoints[i].getY() + alpha * (pointTargets[i].getY() - logicalPoints[i].getY());
        positions.push_back({ x, y });
    }

    /* Move all points. */
    window.setRepaintImmediately(false);
    for (size_t i = 0; i < graphicsPoints.size(); i++) {
        graphicsPoints[i]->setLocation(positions[i].getX() - kPointRadius,
                                       positions[i].getY() - kPointRadius);
    }

    /* Move all lines. */
    for (size_t i = 0; i < graphicsLines.size(); i++) {
        graphicsLines[i]->setStartPoint(positions[logicalLines[i].first].getX(),
                                        positions[logicalLines[i].first].getY());
        graphicsLines[i]->setEndPoint(positions[logicalLines[i].second].getX(),
                                      positions[logicalLines[i].second].getY());
    }
    window.setRepaintImmediately(true);
    window.repaint();

    /* If we hit the end of the animation, we're done. */
    if (frame == kAnimationFrames) {
        state = State::DONE;
        timer.stop();
    }
}

/* Initiates the animation. This entails computing the destination point for each of the
 * star points.
 */
void TabulaRasaReactor::startAnimation() {
    state = State::ANIMATING;

    /* Get the final positions for each point in circular coordinate space. */
    auto angles = destinationAnglesFor(logicalPoints.size());

    /* Assign each point to its target. There are two challenges here:
     *
     * 1. The points are stored in insertion order, not circular order.
     * 2. The circular ordering we use - which just snaps them to *some* circle, not the
     *    *reference* circle - might have everything rotated relative to our destinations.
     *
     * To address this, we'll pick some arbitrary graphics point, compute its angle with the
     * center of the circle, then try to find the destination angle that's as close to that
     * angle as possible. This will then minimize the total rotation.
     */
    auto order  = orderOf(logicalPoints);
    auto center = centerOf(logicalPoints);
    auto theta  = angleBetween(center, logicalPoints[order[0]]);

    /* Now, find the best point to match this point with. */
    size_t bestOffset = 0;
    double bestDistance = 2 * M_PI; // Something impossible

    for (size_t i = 0; i < angles.size(); i++) {
        double distance = fabs(angles[i] - theta);
        distance = min(distance, 2 * M_PI - distance);

        if (distance < bestDistance) {
            bestDistance = distance;
            bestOffset = i;
        }
    }

    /* With that in mind, map each point to its position. We'll iterate over the points in circle
     * order and assign each one its position, shifted over the the offset we determined above.
     */
    auto destinations = destinationsFor(angles, window.getCanvasWidth(), window.getCanvasHeight());
    pointTargets.resize(order.size());
    for (size_t i = 0; i < order.size(); i++) {
        pointTargets[order[i]] = destinations[(i + bestOffset) % destinations.size()];
    }

    timer.start();
}

/* Constructor stashes the window away for later use. */
TabulaRasaReactor::TabulaRasaReactor(GWindow& window, const vector<Star>& permitted) :
    window(window), permitted(permitted), origin(kNotAPoint), timer(kTimerDelay) {

}

/* Clean up our messes. */
TabulaRasaReactor::~TabulaRasaReactor() {
    window.setRepaintImmediately(false);
    for (auto obj: graphicsPoints) {
        window.remove(obj);
    }
    for (auto obj: graphicsLines) {
        window.remove(obj);
    }
    window.setRepaintImmediately(true);
    window.repaint();
}
