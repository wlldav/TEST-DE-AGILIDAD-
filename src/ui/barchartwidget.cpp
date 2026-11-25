#include "barchartwidget.h"

#include <QPainter>

#include <algorithm>

namespace {
const QColor kHitColor(0x2e, 0x9e, 0x5b);
const QColor kMissColor(0xd9, 0x3a, 0x3a);
}

BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent) {}

void BarChartWidget::setStats(const SessionStats &stats)
{
    m_stats = stats;
    update();
}

void BarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().base());

    const int n = static_cast<int>(m_stats.hitsPerButton.size());
    const QRect plot = rect().adjusted(36, 28, -12, -26);
    p.setPen(palette().color(QPalette::Text));

    // Leyenda
    p.fillRect(QRect(plot.left(), 8, 10, 10), kHitColor);
    p.drawText(plot.left() + 14, 17, tr("Aciertos"));
    const int missX = plot.left() + 14 + fontMetrics().horizontalAdvance(tr("Aciertos")) + 18;
    p.fillRect(QRect(missX, 8, 10, 10), kMissColor);
    p.drawText(missX + 14, 17, tr("Fallos"));

    // Ejes
    p.drawLine(plot.bottomLeft(), plot.bottomRight());
    p.drawLine(plot.bottomLeft(), plot.topLeft());
    if (n == 0)
        return;

    int maxValue = 1;
    for (int i = 0; i < n; ++i)
        maxValue = std::max({maxValue, m_stats.hitsPerButton[i], m_stats.missesPerButton[i]});
    p.drawText(QRect(0, plot.top() - 7, 32, 14), Qt::AlignRight | Qt::AlignVCenter,
               QString::number(maxValue));
    p.drawText(QRect(0, plot.bottom() - 7, 32, 14), Qt::AlignRight | Qt::AlignVCenter,
               QStringLiteral("0"));

    const double groupWidth = static_cast<double>(plot.width()) / n;
    const double barWidth = groupWidth * 0.34;
    for (int i = 0; i < n; ++i) {
        const double x0 = plot.left() + i * groupWidth + groupWidth * 0.12;
        const double hitH = plot.height() * static_cast<double>(m_stats.hitsPerButton[i]) / maxValue;
        const double missH = plot.height() * static_cast<double>(m_stats.missesPerButton[i]) / maxValue;
        p.fillRect(QRectF(x0, plot.bottom() - hitH, barWidth, hitH), kHitColor);
        p.fillRect(QRectF(x0 + barWidth + 1, plot.bottom() - missH, barWidth, missH), kMissColor);
        p.drawText(QRectF(plot.left() + i * groupWidth, plot.bottom() + 2, groupWidth, 16),
                   Qt::AlignHCenter | Qt::AlignTop, QString::number(i + 1));
    }
}
