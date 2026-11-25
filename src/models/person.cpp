#include "person.h"

QString Person::fullName() const
{
    return QStringLiteral("%1 %2").arg(m_firstName, m_lastName).trimmed();
}

int Person::age(const QDate &on) const
{
    if (!m_birthDate.isValid() || !on.isValid() || m_birthDate > on)
        return -1;

    int years = on.year() - m_birthDate.year();
    const bool birthdayPending =
        on.month() < m_birthDate.month()
        || (on.month() == m_birthDate.month() && on.day() < m_birthDate.day());
    if (birthdayPending)
        --years;
    return years;
}
