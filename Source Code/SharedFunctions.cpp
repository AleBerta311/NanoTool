#include "sharedfunctions.h"

void DefineAtomicRadiusMap(map<int, double>& map)       // atomic radii in Angstrom for useful elements
{
    map[6] = 0.77;
    map[10] = 0.62;
    map[11] = 1.86;
    map[12] = 1.60;
    map[13] = 1.48;
    map[14] = 1.18;
    map[18] = 1.0;
    map[19] = 2.03;
    map[20] = 1.76;
    map[22] = 1.60;
    map[24] = 1.39;
    map[26] = 1.32;
    map[27] = 1.26;
    map[28] = 1.24;
    map[29] = 1.28;
    map[30] = 1.24;
    map[31] = 1.26;
    map[32] = 1.22;
    map[37] = 2.20;
    map[41] = 1.64;
    map[43] = 1.47;
    map[44] = 1.46;
    map[45] = 1.44;
    map[46] = 1.39;
    map[47] = 1.44;
    map[48] = 1.44;
    map[49] = 1.42;
    map[52] = 1.35;
    map[78] = 1.39;
    map[79] = 1.44;
    map[82] = 1.46;
    map[91] = 2.0;

}

double CalculateCutoffRadius(int Z, map<int, double>& map)
{
    double geom_factor = 2.0;
    double dilat = 1.2;

    return map[Z] * geom_factor * dilat;
}

void DefineSurfaceCNValues(map<string, int>& map)
{
    // If an atom on the mentioned geometry has <= than these nearest neighbor values, it's on the surface

    map["Icosahedron"] = 10;
    map["Cuboctahedron"] = 10;  //cuboctahedron
    map["Octahedron"] = 10;
    map["Truncated Octahedron"] = 10;
    map["Decahedron"] = 10;
    map["FCC111"] = 9;
    map["Tetrahedron"] = 10;
    map["FCC-Cube"] = 9;
    map["FCC100"] = 8;
    map["BCC"] = 6;
    map["HCP"] = 9;
    map["SC"] = 4;      //simple cubic, probably never used
    map["Generic/Amorphous"] = 11;
}

void DefineBulkCN(map<string, int>& map)
{

    map["Icosahedron"] = 12;
    map["Cuboctahedron"] = 12;
    map["Octahedron"] = 12;
    map["Truncated Octahedron"] = 12;
    map["Decahedron"] = 12;
    map["FCC111"] = 12;
    map["FCC100"] = 12;
    map["FCC-Cube"] = 12;
    map["Tetrahedron"] = 12;
    map["BCC"] = 8;
    map["HCP"] = 12;
    map["SC"] = 6;
    map["Generic/Amorphous"] = 12;
}
void DefineMassMap(map<int, double>& map)
{
    // atomic masses in a.u.
    map[6] = 12.011;
    map[10] = 20.180;
    map[11] = 22.99;
    map[12] = 24.305;
    map[13] = 26.982;
    map[14] = 28.085;
    map[18] = 39.95;
    map[19] = 39.098;
    map[20] = 40.078;
    map[22] = 47.867;
    map[24] = 51.996;
    map[26] = 55.845;
    map[27] = 58.933;
    map[28] = 58.693;
    map[29] = 63.546;
    map[30] = 65.38;
    map[31] = 69.723;
    map[32] = 72.630;
    map[37] = 85.468;
    map[41] = 92.906;
    map[43] = 98.;
    map[44] = 101.07;
    map[45] = 102.91;
    map[46] = 106.42;
    map[47] = 107.87;
    map[48] = 112.41;
    map[49] = 114.82;
    map[52] = 127.60;
    map[78] = 195.08;
    map[79] = 107.87;
    map[82] = 207.2;
    map[91] = 231.04;
}

bool compare_by_X(const QPair<qreal, qreal> & a, const QPair<qreal, qreal> & b)
{
    return a.first < b.first;
}

/////////////////////////////// All this stuff is to make the couts work
#include <QDebug>
#include <iostream>
#include <streambuf>

// Custom stream buffer for QDebug
class QDebugStreamBuf : public std::streambuf
{
public:
    QDebugStreamBuf(QDebug& debug) : debug(debug) {}

protected:
    virtual int overflow(int c) override
    {
        if (c != EOF)
        {
            debug << static_cast<char>(c);
        }
        return c;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        for (std::streamsize i = 0; i < n; ++i)
        {
            if (s[i] == '\n')
            {
                debug << "\n";
            } else
            {
                debug << s[i];
            }
        }
        return n;
    }

private:
    QDebug& debug;
};

QDebug debug = QDebug(QtDebugMsg);  // Default QDebug for general messages
QDebugStreamBuf debugStreamBuf(debug);

std::streambuf* oldCoutBuf = std::cout.rdbuf();
std::streambuf* oldCerrBuf = std::cerr.rdbuf();

void RedirectOutput()
{
    std::cout.rdbuf(&debugStreamBuf);
    std::cerr.rdbuf(&debugStreamBuf);
}

void RestoreOutput()
{
    std::cout.rdbuf(oldCoutBuf);
    std::cerr.rdbuf(oldCerrBuf);
}
