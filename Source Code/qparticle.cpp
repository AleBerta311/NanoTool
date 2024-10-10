#include "qparticle.h"
#include <QDebug>

using namespace std;

int QParticle::Get_Z()
{
    return m_Z;
}
QString QParticle::Get_path()
{
    return filepath;
}
QString QParticle::Get_shape()
{
    return m_shape;
}
int QParticle::Get_num()
{
    return m_num;
}
double QParticle::Get_weight_perc()
{
    return m_weight/100.;
}
int QParticle::Get_weight()
{
    return m_weight;
}
void QParticle::Set_i_v(map<double, double> curve)
{
    qDebug() << "Setting I-V Curve for Particle with" << curve.size() << "entries.";
    for (const auto& entry : curve)
    {
        qDebug() << "Potential:" << entry.first << "Current Density:" << entry.second;
    }
    i_v = curve;
}

void QParticle::Set_weight(double w)
{
    m_weight = w;
}

map<double, double> QParticle::Get_i_v() const
{
    return i_v;
}
