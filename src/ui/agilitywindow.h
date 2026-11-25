#ifndef AGILITYWINDOW_H
#define AGILITYWINDOW_H

#include "logic/agilityengine.h"
#include "models/patient.h"
#include "models/user.h"

#include <QDialog>
#include <QList>
#include <QTimer>

class CircleButton;
class DatabaseManager;
class QLabel;
class QPushButton;

/**
 * @class AgilityWindow
 * @brief Ventana de la prueba de agilidad que ve el paciente.
 *
 * Muestra los botones circulares; el motor (AgilityEngine) enciende uno a
 * la vez y esta ventana traduce los clicks del paciente. La duracion la
 * define el usuario antes de abrir la ventana: el paciente no puede
 * modificarla. Al terminar guarda la sesion y todos los eventos.
 */
class AgilityWindow : public QDialog
{
    Q_OBJECT

public:
    /** Numero de botones, igual al de la interfaz propuesta (6 + 5). */
    static constexpr int kButtonCount = 11;

    /**
     * @param db Base de datos abierta.
     * @param patient Paciente evaluado (ya guardado en la base de datos).
     * @param user Usuario que aplica la prueba.
     * @param durationMs Duracion de la prueba en milisegundos.
     * @param parent Widget padre.
     */
    AgilityWindow(DatabaseManager &db, const Patient &patient, const User &user,
                  qint64 durationMs, QWidget *parent = nullptr);

    /** @return Id de la sesion guardada, o -1 si aun no se guardo. */
    int sessionId() const { return m_sessionId; }

    /** @return El motor de la prueba (util para pruebas automaticas). */
    AgilityEngine &engine() { return m_engine; }

public slots:
    /** @brief Inicia la prueba (equivale a pulsar "Iniciar"). */
    void startTest();
    void reject() override;

private slots:
    void onFinished();
    void updateRemaining();

private:
    void buildUi();

    DatabaseManager &m_db;
    Patient m_patient;
    User m_user;
    qint64 m_durationMs;
    QDateTime m_startedAt;
    int m_sessionId = -1;
    AgilityEngine m_engine;
    QTimer m_refreshTimer;
    QList<CircleButton *> m_buttons;
    QLabel *m_remaining = nullptr;
    QPushButton *m_startButton = nullptr;
};

#endif // AGILITYWINDOW_H
