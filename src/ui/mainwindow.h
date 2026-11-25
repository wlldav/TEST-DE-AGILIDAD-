#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "models/patient.h"
#include "models/user.h"

#include <QList>
#include <QMainWindow>

class DatabaseManager;
class QComboBox;
class QLabel;
class QSpinBox;

/**
 * @class MainWindow
 * @brief Ventana principal del usuario autenticado.
 *
 * Muestra el nombre y la edad del usuario (calculada con la fecha de hoy),
 * permite elegir o registrar un paciente, fijar la duracion de la prueba
 * (decision del usuario, no del paciente), iniciarla y consultar resultados.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(DatabaseManager &db, const User &user, QWidget *parent = nullptr);

    /** @return true si la ventana se cerro porque el usuario cerro sesion. */
    bool loggedOut() const { return m_loggedOut; }

private slots:
    void newPatient();
    void startTest();
    void showResults();
    void editProfile();
    void logout();

private:
    void reloadPatients(int selectId = -1);
    void updateWelcome();

    DatabaseManager &m_db;
    User m_user;
    QList<Patient> m_patients;
    bool m_loggedOut = false;
    QLabel *m_welcome;
    QComboBox *m_patientCombo;
    QSpinBox *m_minutes;
};

#endif // MAINWINDOW_H
