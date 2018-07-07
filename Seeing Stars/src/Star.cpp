#include "Star.h"
#include <sstream>
#include <limits>
using namespace std;

/* Constant indicating something that isn't a star. */
const Star kNotAStar = {
    numeric_limits<size_t>::max(),
    numeric_limits<size_t>::max()
};

bool operator== (const Star& lhs, const Star& rhs) {
    return lhs.numPoints == rhs.numPoints &&
           lhs.stepSize  == rhs.stepSize;
}
bool operator!= (const Star& lhs, const Star& rhs) {
    return !(lhs == rhs);
}

ostream& operator<< (ostream& out, const Star& star) {
    if (star == kNotAStar) {
        return out << "(not a star)";
    }

    ostringstream result;
    result << "{ " << star.numPoints << " / " << star.stepSize << " }";
    return out << result.str();
}

string to_string(const Star& star) {
    ostringstream result;
    result << star;
    return result.str();
}
