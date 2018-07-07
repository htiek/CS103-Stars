#ifndef Star_Included
#define Star_Included

#include <cstddef> // For std::size_t
#include <ostream>
#include <string>

struct Star {
    std::size_t numPoints;
    std::size_t stepSize;
};

extern const Star kNotAStar;

bool operator== (const Star& lhs, const Star& rhs);
bool operator!= (const Star& lhs, const Star& rhs);
std::ostream& operator<< (std::ostream& out, const Star& star);
std::string to_string(const Star& star);

#endif
