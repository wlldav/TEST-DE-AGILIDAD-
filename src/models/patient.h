#ifndef PATIENT_H
#define PATIENT_H

#include "person.h"

/**
 * @class Patient
 * @brief Paciente que realiza la prueba de agilidad.
 *
 * Extiende Person con la informacion sociodemografica solicitada en
 * el enunciado: direccion, raza, genero y nivel de ingresos familiares.
 */
class Patient : public Person
{
public:
    QString address() const { return m_address; }
    void setAddress(const QString &v) { m_address = v; }

    QString race() const { return m_race; }
    void setRace(const QString &v) { m_race = v; }

    QString gender() const { return m_gender; }
    void setGender(const QString &v) { m_gender = v; }

    QString incomeLevel() const { return m_incomeLevel; }
    void setIncomeLevel(const QString &v) { m_incomeLevel = v; }

private:
    QString m_address;
    QString m_race;
    QString m_gender;
    QString m_incomeLevel;
};

#endif // PATIENT_H
