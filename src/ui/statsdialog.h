#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include "models/testrecords.h"

#include <QDialog>
#include <QList>

class BarChartWidget;
class DatabaseManager;
class QComboBox;
class QLabel;
class QTableWidget;

/**
 * @class StatsDialog
 * @brief Consulta de resultados: resumen, tabla de eventos y grafica.
 *
 * Permite elegir una prueba guardada y ver el minuto:segundo de cada
 * accion, el boton encendido y si el paciente acerto. Tambien exporta
 * los eventos a un archivo CSV.
 */
class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param db Base de datos abierta.
     * @param preselectSessionId Sesion a mostrar al abrir (-1 = la mas reciente).
     * @param parent Widget padre.
     */
    explicit StatsDialog(DatabaseManager &db, int preselectSessionId = -1,
                         QWidget *parent = nullptr);

private slots:
    void showSession(int comboIndex);
    void exportCsv();

private:
    DatabaseManager &m_db;
    QList<TestSession> m_sessions;
    QList<TestEvent> m_currentEvents;
    QComboBox *m_combo;
    QLabel *m_summary;
    QTableWidget *m_table;
    BarChartWidget *m_chart;
};

#endif // STATSDIALOG_H
