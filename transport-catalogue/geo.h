#pragma once

#include <cmath>
#include <utility>

constexpr double RadToDegCoef = 3.1415926535 / 180.;
constexpr int EarthRadius = 6371000;

namespace geo {

    struct Coordinates {

        Coordinates() = default;
        Coordinates(double, double);
        Coordinates(const Coordinates&);
        Coordinates(Coordinates&&) noexcept;

        ~Coordinates() = default;

        Coordinates& operator=(const Coordinates&);
        Coordinates& operator=(Coordinates&&) noexcept;

        Coordinates& SetLatitude(double);
        Coordinates& SetLongitude(double);

        double GetLatitude() const;
        double GetLongitude() const;

        bool operator==(const Coordinates&) const;
        bool operator!=(const Coordinates&) const;

        double lat = 0.0;
        double lng = 0.0;

    };

    double ComputeDistance(Coordinates from, Coordinates to);
}