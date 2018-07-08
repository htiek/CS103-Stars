#ifndef RadialEditorReactor_Included
#define RadialEditorReactor_Included

#include "Reactor.h"
#include "Star.h"
#include <vector>

/* Editor that presents a fixed set of points in a radial layout for the user to connect. */
class RadialEditorReactor: public Reactor {
public:
    RadialEditorReactor(GWindow& window, std::size_t numPoints);
    ~RadialEditorReactor();

    void handleEvent(GEvent e) override;
    StarType type() const;

private /* helpers */:
    void handleMouseEvent(GMouseEvent e);
    void handleHyperlinkEvent(GHyperlinkEvent e);

private /* state */:
    std::shared_ptr<Star> star;
    std::vector<StarPoint *> points;

    StarType theType = kNotAStar;

    StarPoint* origin = nullptr;
    GLine* currentLine;
};

#endif
