#include "statistics.h"

double SessionStats::accuracy() const
{
    return total == 0 ? 0.0 : 100.0 * hits / total;
}

SessionStats Statistics::compute(const QList<TestEvent> &events, int buttonCount)
{
    SessionStats s;
    s.hitsPerButton = QVector<int>(buttonCount, 0);
    s.missesPerButton = QVector<int>(buttonCount, 0);

    for (const TestEvent &e : events) {
        ++s.total;
        const bool valid = e.buttonIndex >= 0 && e.buttonIndex < buttonCount;
        if (e.hit) {
            ++s.hits;
            if (valid)
                ++s.hitsPerButton[e.buttonIndex];
        } else {
            ++s.misses;
            if (valid)
                ++s.missesPerButton[e.buttonIndex];
        }
    }
    return s;
}

double Statistics::hitsPerMinute(const SessionStats &stats, qint64 durationMs)
{
    if (durationMs <= 0)
        return 0.0;
    return stats.hits * 60000.0 / static_cast<double>(durationMs);
}
