#ifndef STATISTICS_H
#define STATISTICS_H

#include "models/testrecords.h"

#include <QList>
#include <QVector>

/**
 * @struct SessionStats
 * @brief Resumen numerico de una prueba.
 */
struct SessionStats
{
    int total = 0;                 ///< Botones encendidos en total.
    int hits = 0;                  ///< Aciertos.
    int misses = 0;                ///< Fallos.
    QVector<int> hitsPerButton;    ///< Aciertos por boton.
    QVector<int> missesPerButton;  ///< Fallos por boton.

    /** @return Porcentaje de aciertos (0..100); 0 si no hubo eventos. */
    double accuracy() const;
};

/**
 * @class Statistics
 * @brief Calcula estadisticas a partir de la lista de eventos.
 */
class Statistics
{
public:
    /**
     * @brief Calcula el resumen de una lista de eventos.
     * @param events Eventos de la prueba.
     * @param buttonCount Cantidad de botones de la interfaz.
     * @return Estadisticas agregadas.
     */
    static SessionStats compute(const QList<TestEvent> &events, int buttonCount);

    /**
     * @brief Calcula el ritmo de aciertos por minuto.
     * @param stats Estadisticas ya calculadas.
     * @param durationMs Duracion de la prueba en milisegundos.
     * @return Aciertos por minuto (0 si la duracion es 0).
     */
    static double hitsPerMinute(const SessionStats &stats, qint64 durationMs);
};

#endif // STATISTICS_H
