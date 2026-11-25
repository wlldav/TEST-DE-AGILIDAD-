#include "agilitywindow.h"

#include "circlebutton.h"
#include "database/databasemanager.h"
#include "logic/statistics.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AgilityWindow::AgilityWindow(DatabaseManager &db, const Patient &patient, const User &user,
                             qint64 durationMs, QWidget *parent)
    : QDialog(parent),
      m_db(db),
      m_patient(patient),
      m_user(user),
      m_durationMs(durationMs),
      m_engine(kButtonCount, 1000, this)
{
    setWindowTitle(QStringLiteral("Agility"));
    buildUi();

    connect(&m_engine, &AgilityEngine::buttonLit, this, [this](int i) {
        m_buttons.at(i)->setLit(true);
    });
    connect(&m_engine, &AgilityEngine::buttonOff, this, [this](int i) {
        m_buttons.at(i)->setLit(false);
    });
    connect(&m_engine, &AgilityEngine::finished, this, &AgilityWindow::onFinished);

    m_refreshTimer.setInterval(200);
    connect(&m_refreshTimer, &QTimer::timeout, this, &AgilityWindow::updateRemaining);
}

void AgilityWindow::buildUi()
{
    auto *title = new QLabel(tr("Prueba de agilidad"), this);
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);

    // Dos filas de botones (6 y 5), como en la interfaz propuesta.
    auto *top = new QHBoxLayout;
    auto *bottom = new QHBoxLayout;
    top->addStretch();
    bottom->addStretch();
    for (int i = 0; i < kButtonCount; ++i) {
        auto *button = new CircleButton(48, this);
        m_buttons.append(button);
        connect(button, &QAbstractButton::pressed, this, [this, i]() {
            m_engine.registerClick(i);
        });
        QHBoxLayout *row = (i < 6) ? top : bottom;
        row->addWidget(button);
        row->addStretch();
    }

    auto *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->addLayout(top);
    frameLayout->addLayout(bottom);

    m_remaining = new QLabel(this);
    m_remaining->setMinimumWidth(200);
    m_startButton = new QPushButton(tr("Iniciar"), this);
    connect(m_startButton, &QPushButton::clicked, this, &AgilityWindow::startTest);

    auto *footer = new QHBoxLayout;
    footer->addWidget(m_remaining);
    footer->addStretch();
    footer->addWidget(m_startButton);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(frame, 1);
    layout->addLayout(footer);

    const int totalSec = static_cast<int>(m_durationMs / 1000);
    m_remaining->setText(tr("Duración: %1:%2")
                             .arg(totalSec / 60)
                             .arg(totalSec % 60, 2, 10, QLatin1Char('0')));
    resize(560, 260);
}

void AgilityWindow::startTest()
{
    if (m_engine.isRunning())
        return;
    m_startButton->hide();
    m_startedAt = QDateTime::currentDateTime();
    m_engine.start(m_durationMs);
    m_refreshTimer.start();
    updateRemaining();
}

void AgilityWindow::updateRemaining()
{
    const int left = static_cast<int>((m_engine.remainingMs() + 999) / 1000);
    m_remaining->setText(tr("Tiempo restante: %1:%2")
                             .arg(left / 60)
                             .arg(left % 60, 2, 10, QLatin1Char('0')));
}

void AgilityWindow::reject()
{
    // Si la prueba esta en curso, se detiene y se guardan los datos parciales.
    if (m_engine.isRunning()) {
        m_engine.stop();
        return;
    }
    QDialog::reject();
}

void AgilityWindow::onFinished()
{
    m_refreshTimer.stop();
    m_remaining->setText(tr("Prueba finalizada"));

    const QList<TestEvent> &events = m_engine.events();
    m_sessionId = m_db.createSession(m_patient.id(), m_user.id(), m_startedAt, m_durationMs);
    if (m_sessionId < 0 || !m_db.addEvents(m_sessionId, events)) {
        QMessageBox::critical(this, windowTitle(),
                              tr("No se pudieron guardar los resultados.\n%1").arg(m_db.lastError()));
        QDialog::reject();
        return;
    }

    const SessionStats s = Statistics::compute(events, kButtonCount);
    QMessageBox::information(
        this, tr("Resultados"),
        tr("Botones encendidos: %1\nAciertos: %2\nFallos: %3\nPrecisión: %4 %")
            .arg(s.total)
            .arg(s.hits)
            .arg(s.misses)
            .arg(s.accuracy(), 0, 'f', 1));
    QDialog::accept();
}
