#ifndef PATIENTFORMDIALOG_H
#define PATIENTFORMDIALOG_H

#include "models/patient.h"

#include <QDialog>

class DatabaseManager;
class QComboBox;
class QDateEdit;
class QLineEdit;

/**
 * @class PatientFormDialog
 * @brief Formulario de informacion sociodemografica del paciente.
 *
 * Recolecta nombre, apellido, documento, fecha de nacimiento, direccion,
 * raza, genero y nivel de ingresos familiares, y los guarda en la base de datos.
 */
class PatientFormDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatientFormDialog(DatabaseManager &db, QWidget *parent = nullptr);

    /** @return Paciente guardado (valido tras aceptar el dialogo). */
    const Patient &patient() const { return m_patient; }

protected:
    void accept() override;

private:
    DatabaseManager &m_db;
    Patient m_patient;
    QLineEdit *m_firstName;
    QLineEdit *m_lastName;
    QLineEdit *m_document;
    QDateEdit *m_birthDate;
    QLineEdit *m_address;
    QComboBox *m_race;
    QComboBox *m_gender;
    QComboBox *m_income;
};

#endif // PATIENTFORMDIALOG_H
