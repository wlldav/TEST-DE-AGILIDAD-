#ifndef TESTRECORDS_H
#define TESTRECORDS_H

#include <QDateTime>
#include <QString>

/**
 * @file testrecords.h
 * @brief Registros de una prueba de agilidad: eventos y sesiones.
 */

/**
 * @struct TestEvent
 * @brief Una accion registrada durante la prueba.
 *
 * Cada vez que un boton se enciende se genera exactamente un evento:
 * acierto (el paciente hizo click mientras estaba encendido) o fallo
 * (el boton se apago sin que se hiciera click).
 */
struct TestEvent
{
    qint64 elapsedMs = 0;  ///< Milisegundos desde el inicio de la prueba.
    int buttonIndex = 0;   ///< Boton que estaba encendido (0..N-1).
    bool hit = false;      ///< true si el paciente logro dar click.

    /** @return Minuto transcurrido desde el inicio de la prueba. */
    int minute() const { return static_cast<int>(elapsedMs / 60000); }
    /** @return Segundo (0..59) dentro del minuto actual. */
    int second() const { return static_cast<int>((elapsedMs / 1000) % 60); }
};

/**
 * @struct TestSession
 * @brief Cabecera de una prueba realizada por un paciente.
 */
struct TestSession
{
    int id = -1;               ///< Identificador en la base de datos.
    int patientId = -1;        ///< Paciente evaluado.
    int userId = -1;           ///< Usuario que aplico la prueba.
    QDateTime startedAt;       ///< Fecha y hora de inicio.
    qint64 durationMs = 0;     ///< Duracion configurada de la prueba.
    QString patientName;       ///< Nombre del paciente (solo lectura, via JOIN).
};

#endif // TESTRECORDS_H
