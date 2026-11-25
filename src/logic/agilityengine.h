#ifndef AGILITYENGINE_H
#define AGILITYENGINE_H

#include "models/testrecords.h"

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>

/**
 * @class AgilityEngine
 * @brief Logica de la prueba de agilidad, independiente de la interfaz.
 *
 * Enciende un solo boton a la vez, elegido al azar (distinto del anterior),
 * durante @p litMs milisegundos. Si el paciente hace click mientras esta
 * encendido se registra un acierto y se pasa al siguiente; si el tiempo se
 * agota se registra un fallo. La prueba termina al cumplirse la duracion.
 *
 * La separacion entre logica e interfaz permite probar el motor sin GUI.
 */
class AgilityEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * @param buttonCount Numero de botones disponibles.
     * @param litMs Tiempo que cada boton permanece encendido (1000 por enunciado).
     * @param parent Objeto padre de Qt.
     */
    explicit AgilityEngine(int buttonCount = 11, int litMs = 1000,
                           QObject *parent = nullptr);

    /** @brief Inicia una prueba nueva (descarta eventos anteriores). */
    void start(qint64 durationMs);

    /** @brief Detiene la prueba; emite finished() si estaba corriendo. */
    void stop();

    bool isRunning() const { return m_running; }
    int buttonCount() const { return m_buttonCount; }
    /** @return Boton encendido o -1 si ninguno. */
    int currentButton() const { return m_current; }
    qint64 durationMs() const { return m_durationMs; }
    /** @return Milisegundos transcurridos desde el inicio. */
    qint64 elapsedMs() const;
    /** @return Milisegundos restantes (0 si no esta corriendo). */
    qint64 remainingMs() const;
    /** @return Eventos registrados hasta el momento. */
    const QList<TestEvent> &events() const { return m_events; }

public slots:
    /** @brief Registra un click del paciente sobre el boton @p index. */
    void registerClick(int index);

signals:
    void buttonLit(int index);                 ///< Un boton se encendio.
    void buttonOff(int index);                 ///< Un boton se apago.
    void eventRecorded(const TestEvent &event);///< Se registro un acierto o fallo.
    void finished();                           ///< La prueba termino.

private slots:
    void onLitTimeout();
    void onDurationElapsed();

private:
    void lightNext();
    void record(bool hit);

    int m_buttonCount;
    int m_litMs;
    int m_current = -1;
    int m_previous = -1;
    bool m_running = false;
    qint64 m_durationMs = 0;
    QTimer m_litTimer;
    QTimer m_durationTimer;
    QElapsedTimer m_clock;
    QList<TestEvent> m_events;
};

#endif // AGILITYENGINE_H
