#include "statsdialog.h"

#include "barchartwidget.h"
#include "database/databasemanager.h"
#include "logic/agilityengine.h"
#include "logic/statistics.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

namespace {
constexpr int kButtons = 11;  // coincide con AgilityWindow::kButtonCount

QString timeText(const TestEvent &e)
{
    return QStringLiteral("%1:%2").arg(e.minute(), 2, 10, QLatin1Char('0'))
                                  .arg(e.second(), 2, 10, QLatin1Char('0'));
}
}

StatsDialog::StatsDialog(DatabaseManager &db, int preselectSessionId, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle(tr("Resultados y estadísticas"));
    resize(640, 560);

    m_combo = new QComboBox(this);
    m_summary = new QLabel(this);
    m_summary->setWordWrap(true);

    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels({tr("Minuto:Segundo"), tr("Botón encendido"), tr("Resultado")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_chart = new BarChartWidget(this);

    auto *exportButton = new QPushButton(tr("Exportar CSV"), this);
    connect(exportButton, &QPushButton::clicked, this, &StatsDialog::exportCsv);
    auto *closeButton = new QPushButton(tr("Cerrar"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    auto *footer = new QHBoxLayout;
    footer->addStretch();
    footer->addWidget(exportButton);
    footer->addWidget(closeButton);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_combo);
    layout->addWidget(m_summary);
    layout->addWidget(m_chart, 1);
    layout->addWidget(m_table, 2);
    layout->addLayout(footer);

    m_sessions = m_db.sessions();
    int select = 0;
    for (int i = 0; i < m_sessions.size(); ++i) {
        const TestSession &s = m_sessions.at(i);
        m_combo->addItem(tr("%1 - %2 - %3 s")
                             .arg(s.patientName,
                                  s.startedAt.toString(QStringLiteral("dd/MM/yyyy HH:mm")))
                             .arg(s.durationMs / 1000));
        if (s.id == preselectSessionId)
            select = i;
    }
    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &StatsDialog::showSession);

    if (m_sessions.isEmpty()) {
        m_summary->setText(tr("Aún no hay pruebas registradas."));
        exportButton->setEnabled(false);
    } else {
        m_combo->setCurrentIndex(select);
        showSession(select);
    }
}

void StatsDialog::showSession(int comboIndex)
{
    if (comboIndex < 0 || comboIndex >= m_sessions.size())
        return;
    const TestSession &session = m_sessions.at(comboIndex);
    m_currentEvents = m_db.events(session.id);

    const SessionStats stats = Statistics::compute(m_currentEvents, kButtons);
    m_summary->setText(
        tr("Total: %1   Aciertos: %2   Fallos: %3   Precisión: %4 %   Aciertos/min: %5")
            .arg(stats.total)
            .arg(stats.hits)
            .arg(stats.misses)
            .arg(stats.accuracy(), 0, 'f', 1)
            .arg(Statistics::hitsPerMinute(stats, session.durationMs), 0, 'f', 1));
    m_chart->setStats(stats);

    m_table->setRowCount(static_cast<int>(m_currentEvents.size()));
    for (int row = 0; row < m_currentEvents.size(); ++row) {
        const TestEvent &e = m_currentEvents.at(row);
        m_table->setItem(row, 0, new QTableWidgetItem(timeText(e)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(e.buttonIndex + 1)));
        m_table->setItem(row, 2, new QTableWidgetItem(e.hit ? tr("Acierto") : tr("Fallo")));
    }
}

void StatsDialog::exportCsv()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Exportar CSV"),
                                                      QStringLiteral("resultados.csv"),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, windowTitle(), tr("No se pudo escribir el archivo."));
        return;
    }
    QTextStream out(&file);
    out << "minuto_segundo,boton,resultado\n";
    for (const TestEvent &e : m_currentEvents)
        out << timeText(e) << ',' << (e.buttonIndex + 1) << ',' << (e.hit ? "acierto" : "fallo") << '\n';
}
