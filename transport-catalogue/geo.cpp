#include "geo.h"

namespace geo {

    Coordinates::Coordinates(double lat, double lng)
        : lat(lat), lng(lng) {
    }

    Coordinates::Coordinates(const Coordinates& other)
        : lat(other.GetLatitude()), lng(other.GetLongitude()) {
    }

    Coordinates::Coordinates(Coordinates&& other) noexcept
        : lat(std::move(other.lat)), lng(std::move(other.lng)) {
    }

    Coordinates& Coordinates::operator=(const Coordinates& other) {
        lat = other.GetLatitude();
        lng = other.GetLongitude();
        return *this;
    }

    Coordinates& Coordinates::operator=(Coordinates&& other) noexcept {
        lat = std::move(other.GetLatitude());
        lng = std::move(other.GetLongitude());
        return *this;
    }

    Coordinates& Coordinates::SetLatitude(double latitude) {
        lat = latitude;
        return *this;
    }

    Coordinates& Coordinates::SetLongitude(double longitude) {
        lng = longitude;
        return *this;
    }

    double Coordinates::GetLatitude() const {
        return lat;
    }

    double Coordinates::GetLongitude() const {
        return lng;
    }

    bool Coordinates::operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }

    bool Coordinates::operator!=(const Coordinates& other) const {
        return !(*this == other);
    }

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        return acos(sin(from.lat * RadToDegCoef) * sin(to.lat * RadToDegCoef)
            + cos(from.lat * RadToDegCoef) * cos(to.lat * RadToDegCoef) * cos(abs(from.lng - to.lng) * RadToDegCoef))
            * EarthRadius;
    }
}