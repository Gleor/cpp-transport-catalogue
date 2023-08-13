#pragma once

#include <cmath>

constexpr double RadToDegCoef = 3.1415926535 / 180.;
constexpr int EarthRadius = 6371000;

namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    return acos(sin(from.lat * RadToDegCoef) * sin(to.lat * RadToDegCoef)
        + cos(from.lat * RadToDegCoef) * cos(to.lat * RadToDegCoef)
        * cos(abs(from.lng - to.lng) * RadToDegCoef)) * EarthRadius;
}
}