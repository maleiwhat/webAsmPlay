#include <glm/vec2.hpp>
#include <geos/geom/Polygon.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <webAsmPlay/Debug.h>
#include <webAsmPlay/Attributes.h>
#include <webAsmPlay/GeometryConverter.h>

using namespace std;
using namespace glm;
using namespace geos::geom;

string GeometryConverter::convert(const geos::geom::Polygon * poly, const Attributes * attrs)
{
    stringstream ret;

    convert(poly, attrs, ret);

    return ret.str();
}

void GeometryConverter::convert(const Polygon * poly, const Attributes * attrs, stringstream & data)
{
    attrs->write(data);

    convert(poly->getExteriorRing(), data);

    const uint32_t numInteriorRings = poly->getNumInteriorRing();

    data.write((const char *)&numInteriorRings, sizeof(uint32_t));

    for(size_t i = 0; i < numInteriorRings; ++i) { convert(poly->getInteriorRingN(i), data) ;}
}

void GeometryConverter::convert(const LineString * lineString, stringstream & data)
{
    const vector<Coordinate> & coords = *lineString->getCoordinatesRO()->toVector();

    const uint32_t numVerts = coords.size();

    data.write((const char *)&numVerts, sizeof(uint32_t));

    for(size_t i = 0; i < numVerts; ++i)
    {
        const Coordinate & C = coords[i];

        const double coord[] = { C.x, C.y };
        
        data.write((const char *)coord, sizeof(coord));
    }
}

CoordinateSequence * GeometryConverter::getGeosCoordinateSequence(const char *& lineString)
{
    const uint32_t numVerts = *(uint32_t *)lineString; lineString += sizeof(uint32_t);

    vector<Coordinate> * coords = new vector<Coordinate>(numVerts);

    for(size_t i = 0; i < numVerts; ++i)
    {
        const double x = *(double *)lineString; lineString += sizeof(double);
        const double y = *(double *)lineString; lineString += sizeof(double);

        (*coords)[i] = Coordinate(x,y);
    }

    return new CoordinateArraySequence(coords, 2);
}

AttributedGeometry GeometryConverter::getGeosPolygon(const char *& poly)
{
    Attributes * attrs = new Attributes(poly);

    const GeometryFactory * factory = GeometryFactory::getDefaultInstance();

    LinearRing * externalRing = factory->createLinearRing(getGeosCoordinateSequence(poly));

    const uint32_t numHoles = *(uint32_t *)poly; poly += sizeof(uint32_t);

    vector<Geometry *> * holes = new vector<Geometry *>(numHoles);

    for(size_t i = 0; i < numHoles; ++i)
    {
        (*holes)[i] = (Geometry *)factory->createLinearRing(getGeosCoordinateSequence(poly));
    }

    return AttributedGeometry(attrs, factory->createPolygon(externalRing, holes));
}

vector<AttributedGeometry> GeometryConverter::getGeosPolygons(const char *& polys)
{
    const uint32_t numPolygons = *(uint32_t *)polys; polys += sizeof(uint32_t);

    vector<AttributedGeometry> ret(numPolygons);

    for(size_t i = 0; i < numPolygons; ++i)
    {
        ret[i] = getGeosPolygon(polys);
    }

    return ret;
}
