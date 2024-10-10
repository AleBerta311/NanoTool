#ifndef SHAREDFUNCTIONS_H
#define SHAREDFUNCTIONS_H

#include <map>
#include <QString>

using namespace std;

void RedirectOutput();
void RestoreOutput();
//////////////////////////////// other custom functions for nanocode
void DefineAtomicRadiusMap(map<int, double>&);
double CalculateCutoffRadius(int, map<int, double>&); // atomic radius * geom_factor(2.) * dilat factor (20%)
void DefineSurfaceCNValues(map<string, int>&);
void DefineBulkCN(map<string, int>&);
void DefineMassMap(map<int, double>&);
bool compare_by_X(const QPair<qreal, qreal> &, const QPair<qreal, qreal> &);
/////////////////////////////////////////////

#endif // SHAREDFUNCTIONS_H
