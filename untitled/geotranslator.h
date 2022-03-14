#ifndef GEOTRANSLATOR_H
#define GEOTRANSLATOR_H

#include <QtMath>

class GeoTranslator
{
public:
    GeoTranslator();
    const double earthRadius = 6378137; //экваториальный радиус Земли в WGS-84
    const double ellipseContraction = 298.257223563; //сжатие элипсоида в WGS-84
    const double f = 1.0 / ellipseContraction;

    double fe = qSqrt(f * (2 - f));
    double bo;
    double lo;
    double w;
    double X0 = 0;
    double Z0 = 0;

    void setBaseLatitude(double lat);
    void setBaseLongitude(double lon);
    double getBaseLatitude();
    double getBaseLongitude();
    double LatToZ(double lat);
    double LonToX(double lon);
    double ZToLat(double z);
    double XToLon(double x);
private:
    double baseLatitude;
    double baseLongitude;
};

#endif // GEOTRANSLATOR_H
