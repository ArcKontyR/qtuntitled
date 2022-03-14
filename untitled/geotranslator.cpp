#include "geotranslator.h"
#include <QtMath>
GeoTranslator::GeoTranslator() {}

void GeoTranslator::setBaseLatitude(double lat) {
    baseLatitude = lat;
    bo = qDegreesToRadians(baseLatitude);
    w = qSqrt(1 - qPow(fe * qSin(bo), 2));
}
double GeoTranslator::getBaseLatitude() { return baseLatitude; }

void GeoTranslator::setBaseLongitude(double lon) {
    baseLongitude = lon;
    lo = qDegreesToRadians(baseLongitude);

}
double GeoTranslator::getBaseLongitude() { return baseLongitude; }

double GeoTranslator::LatToZ(double lat) {
    double dZ = (qDegreesToRadians(lat) - bo) *
        (earthRadius * (1 - qPow(fe, 2)) / qPow(w, 3));
    return Z0 + dZ;
}
double GeoTranslator::LonToX(double lon)
{
    double dX = (qDegreesToRadians(lon) - lo) * ((earthRadius * qCos(bo)) / w);
    return X0 + dX;
}

double GeoTranslator::XToLon(double x) {
    double dL = (x - X0) / (earthRadius * qCos(bo) / w);
    return baseLongitude + qRadiansToDegrees(dL);
}

double GeoTranslator::ZToLat(double z)
{
    double dB = (z - Z0) /(earthRadius * (1 - qPow(fe, 2)) / qPow(w, 3));
    return baseLatitude + qRadiansToDegrees(dB);
}
