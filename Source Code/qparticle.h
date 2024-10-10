#ifndef QPARTICLE_H
#define QPARTICLE_H

#include <QString>
#include <map>


class QParticle
{
    public:
        QParticle() = default;
        QParticle(int z, QString& path, QString& shape, int num)
        {
            m_Z = z;
            filepath = path;
            m_shape = shape;
            m_num = num;
        }
        ~QParticle() = default;

        void Set_i_v(std::map<double, double>);
        void Set_weight(double);
        int Get_Z();
        QString Get_path();
        QString Get_shape();
        int Get_num();
        double Get_weight_perc();
        int Get_weight();
        std::map<double, double> Get_i_v() const;

    private:
        int m_Z;
        QString filepath;
        QString m_shape;
        int m_num;
        std::map<double, double> i_v;
        double m_weight;
};

#endif // QPARTICLE_H
