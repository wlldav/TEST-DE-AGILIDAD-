#include "agilityengine.h"

#include <QRandomGenerator>

AgilityEngine::AgilityEngine(int buttonCount, int litMs, QObject *parent)
    : QObject(parent), m_buttonCount(buttonCount), m_litMs(litMs)
{
    m_litTimer.setSingleShot(true);
    m_litTimer.setTimerType(Qt::PreciseTimer);
    m_durationTimer.setSingleShot(true);
    m_durationTimer.setTimerType(Qt::PreciseTimer);

    connect(&m_litTimer, &QTimer::timeout, this, &AgilityEngine::onLitTimeout);
    connect(&m_durationTimer, &QTimer::timeout, this, &AgilityEngine::onDurationElapsed);
}

void AgilityEngine::start(qint64 durationMs)
{
    if (m_running || m_buttonCount <= 0 || durationMs <= 0)
        return;

    m_events.clear();
    m_current = -1;
    m_previous = -1;
    m_durationMs = durationMs;
    m_running = true;
    m_clock.start();
    m_durationTimer.start(static_cast<int>(durationMs));
    lightNext();
}

void AgilityEngine::stop()
{
    if (!m_running)
        return;

    m_litTimer.stop();
    m_durationTimer.stop();
    m_running = false;
    if (m_current >= 0) {
        // El boton encendido no completo su segundo: no cuenta como fallo.
        const int old = m_current;
        m_current = -1;
        emit buttonOff(old);
    }
    emit finished();
}

qint64 AgilityEngine::elapsedMs() const
{
    return m_clock.isValid() ? m_clock.elapsed() : 0;
}

qint64 AgilityEngine::remainingMs() const
{
    if (!m_running)
        return 0;
    const qint64 left = m_durationMs - elapsedMs();
    return left > 0 ? left : 0;
}

void AgilityEngine::registerClick(int index)
{
    if (!m_running || index != m_current)
        return;  // Click sobre un boton apagado: se ignora.

    m_litTimer.stop();
    record(true);
    lightNext();
}

void AgilityEngine::onLitTimeout()
{
    if (!m_running)
        return;
    record(false);
    lightNext();
}

void AgilityEngine::onDurationElapsed()
{
    stop();
}

void AgilityEngine::record(bool hit)
{
    TestEvent e;
    e.elapsedMs = elapsedMs();
    e.buttonIndex = m_current;
    e.hit = hit;
    m_events.append(e);

    const int old = m_current;
    m_current = -1;
    emit buttonOff(old);
    emit eventRecorded(e);
}

void AgilityEngine::lightNext()
{
    int next = static_cast<int>(QRandomGenerator::global()->bounded(m_buttonCount));
    if (m_buttonCount > 1) {
        while (next == m_previous)
            next = static_cast<int>(QRandomGenerator::global()->bounded(m_buttonCount));
    }
    m_previous = next;
    m_current = next;
    emit buttonLit(next);
    m_litTimer.start(m_litMs);
}
