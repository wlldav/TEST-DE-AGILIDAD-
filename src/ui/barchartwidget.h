#ifndef BARCHARTWIDGET_H
#define BARCHARTWIDGET_H

#include "logic/statistics.h"

#include <QWidget>

/**
 * @class BarChartWidget
 * @brief Grafica de barras agrupadas: aciertos y fallos por boton.
 *
 * Dibujada con QPainter para evitar dependencias adicionales (QtCharts)
 * y mantener el ejecutable pequeno.
 */
class BarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarChartWidget(QWidget *parent = nullptr);

    /** @brief Define los datos a graficar y repinta. */
    void setStats(const SessionStats &stats);

    QSize minimumSizeHint() const override { return QSize(320, 180); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    SessionStats m_stats;
};

#endif // BARCHARTWIDGET_H
