#include "databasemanager.h"

#include <QCryptographicHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace {

const char *const kDateFormat = "yyyy-MM-dd";

User userFromQuery(const QSqlQuery &q)
{
    User u;
    u.setId(q.value(0).toInt());
    u.setUsername(q.value(1).toString());
    u.setFirstName(q.value(2).toString());
    u.setLastName(q.value(3).toString());
    u.setDocumentId(q.value(4).toString());
    u.setBirthDate(QDate::fromString(q.value(5).toString(), kDateFormat));
    return u;
}

Patient patientFromQuery(const QSqlQuery &q)
{
    Patient p;
    p.setId(q.value(0).toInt());
    p.setFirstName(q.value(1).toString());
    p.setLastName(q.value(2).toString());
    p.setDocumentId(q.value(3).toString());
    p.setBirthDate(QDate::fromString(q.value(4).toString(), kDateFormat));
    p.setAddress(q.value(5).toString());
    p.setRace(q.value(6).toString());
    p.setGender(q.value(7).toString());
    p.setIncomeLevel(q.value(8).toString());
    return p;
}

} // namespace

DatabaseManager::~DatabaseManager()
{
    close();
}

QSqlDatabase DatabaseManager::db() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

bool DatabaseManager::fail(const QString &message)
{
    m_lastError = message;
    return false;
}

bool DatabaseManager::isOpen() const
{
    return !m_connectionName.isEmpty() && db().isOpen();
}

bool DatabaseManager::open(const QString &path)
{
    close();
    m_connectionName = QStringLiteral("agility_%1").arg(QUuid::createUuid().toString(QUuid::Id128));

    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    database.setDatabaseName(path);
    if (!database.open())
        return fail(database.lastError().text());

    QSqlQuery pragma(database);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    return createSchema();
}

void DatabaseManager::close()
{
    if (m_connectionName.isEmpty())
        return;
    {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName, false);
        if (database.isOpen())
            database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();
}

bool DatabaseManager::createSchema()
{
    static const char *const statements[] = {
        "CREATE TABLE IF NOT EXISTS users ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username TEXT NOT NULL UNIQUE,"
        " password_hash TEXT NOT NULL,"
        " salt TEXT NOT NULL,"
        " first_name TEXT NOT NULL,"
        " last_name TEXT NOT NULL,"
        " document_id TEXT NOT NULL UNIQUE,"
        " birth_date TEXT NOT NULL)",

        "CREATE TABLE IF NOT EXISTS patients ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " first_name TEXT NOT NULL,"
        " last_name TEXT NOT NULL,"
        " document_id TEXT NOT NULL UNIQUE,"
        " birth_date TEXT NOT NULL,"
        " address TEXT,"
        " race TEXT,"
        " gender TEXT,"
        " income_level TEXT)",

        "CREATE TABLE IF NOT EXISTS test_sessions ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " patient_id INTEGER NOT NULL REFERENCES patients(id) ON DELETE CASCADE,"
        " user_id INTEGER NOT NULL REFERENCES users(id),"
        " started_at TEXT NOT NULL,"
        " duration_ms INTEGER NOT NULL CHECK (duration_ms > 0))",

        "CREATE TABLE IF NOT EXISTS test_events ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " session_id INTEGER NOT NULL REFERENCES test_sessions(id) ON DELETE CASCADE,"
        " elapsed_min INTEGER NOT NULL,"
        " elapsed_sec INTEGER NOT NULL,"
        " elapsed_ms INTEGER NOT NULL,"
        " button_index INTEGER NOT NULL,"
        " hit INTEGER NOT NULL CHECK (hit IN (0, 1)))",

        "CREATE INDEX IF NOT EXISTS idx_events_session ON test_events(session_id)",
        "CREATE INDEX IF NOT EXISTS idx_sessions_patient ON test_sessions(patient_id)",
    };

    QSqlQuery q(db());
    for (const char *sql : statements) {
        if (!q.exec(QString::fromLatin1(sql)))
            return fail(q.lastError().text());
    }
    return true;
}

QString DatabaseManager::hashPassword(const QString &salt, const QString &password)
{
    const QByteArray data = (salt + password).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

// ---------------------------------------------------------------- Usuarios

int DatabaseManager::addUser(const User &user, const QString &password)
{
    const QString salt = QUuid::createUuid().toString(QUuid::Id128);
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "INSERT INTO users (username, password_hash, salt, first_name, last_name,"
        " document_id, birth_date) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(user.username());
    q.addBindValue(hashPassword(salt, password));
    q.addBindValue(salt);
    q.addBindValue(user.firstName());
    q.addBindValue(user.lastName());
    q.addBindValue(user.documentId());
    q.addBindValue(user.birthDate().toString(kDateFormat));
    if (!q.exec()) {
        fail(q.lastError().text());
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool DatabaseManager::updateUser(const User &user, const QString &newPassword)
{
    QSqlQuery q(db());
    if (newPassword.isEmpty()) {
        q.prepare(QStringLiteral(
            "UPDATE users SET username=?, first_name=?, last_name=?, document_id=?,"
            " birth_date=? WHERE id=?"));
    } else {
        q.prepare(QStringLiteral(
            "UPDATE users SET username=?, first_name=?, last_name=?, document_id=?,"
            " birth_date=?, password_hash=?, salt=? WHERE id=?"));
    }
    q.addBindValue(user.username());
    q.addBindValue(user.firstName());
    q.addBindValue(user.lastName());
    q.addBindValue(user.documentId());
    q.addBindValue(user.birthDate().toString(kDateFormat));
    if (!newPassword.isEmpty()) {
        const QString salt = QUuid::createUuid().toString(QUuid::Id128);
        q.addBindValue(hashPassword(salt, newPassword));
        q.addBindValue(salt);
    }
    q.addBindValue(user.id());
    if (!q.exec())
        return fail(q.lastError().text());
    return q.numRowsAffected() == 1;
}

std::optional<User> DatabaseManager::authenticate(const QString &username,
                                                  const QString &password)
{
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT id, username, first_name, last_name, document_id, birth_date,"
        " password_hash, salt FROM users WHERE username = ?"));
    q.addBindValue(username);
    if (!q.exec() || !q.next())
        return std::nullopt;

    const QString stored = q.value(6).toString();
    const QString salt = q.value(7).toString();
    if (hashPassword(salt, password) != stored)
        return std::nullopt;
    return userFromQuery(q);
}

bool DatabaseManager::usernameExists(const QString &username, int excludeId)
{
    QSqlQuery q(db());
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM users WHERE username = ? AND id != ?"));
    q.addBindValue(username);
    q.addBindValue(excludeId);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

int DatabaseManager::userCount()
{
    QSqlQuery q(db());
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM users")) && q.next())
        return q.value(0).toInt();
    return 0;
}

// --------------------------------------------------------------- Pacientes

int DatabaseManager::addPatient(const Patient &p)
{
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "INSERT INTO patients (first_name, last_name, document_id, birth_date,"
        " address, race, gender, income_level) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(p.firstName());
    q.addBindValue(p.lastName());
    q.addBindValue(p.documentId());
    q.addBindValue(p.birthDate().toString(kDateFormat));
    q.addBindValue(p.address());
    q.addBindValue(p.race());
    q.addBindValue(p.gender());
    q.addBindValue(p.incomeLevel());
    if (!q.exec()) {
        fail(q.lastError().text());
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool DatabaseManager::updatePatient(const Patient &p)
{
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "UPDATE patients SET first_name=?, last_name=?, document_id=?, birth_date=?,"
        " address=?, race=?, gender=?, income_level=? WHERE id=?"));
    q.addBindValue(p.firstName());
    q.addBindValue(p.lastName());
    q.addBindValue(p.documentId());
    q.addBindValue(p.birthDate().toString(kDateFormat));
    q.addBindValue(p.address());
    q.addBindValue(p.race());
    q.addBindValue(p.gender());
    q.addBindValue(p.incomeLevel());
    q.addBindValue(p.id());
    if (!q.exec())
        return fail(q.lastError().text());
    return q.numRowsAffected() == 1;
}

QList<Patient> DatabaseManager::patients()
{
    QList<Patient> list;
    QSqlQuery q(db());
    if (!q.exec(QStringLiteral(
            "SELECT id, first_name, last_name, document_id, birth_date, address, race,"
            " gender, income_level FROM patients ORDER BY last_name, first_name"))) {
        fail(q.lastError().text());
        return list;
    }
    while (q.next())
        list.append(patientFromQuery(q));
    return list;
}

// ------------------------------------------------------------------ Pruebas

int DatabaseManager::createSession(int patientId, int userId,
                                   const QDateTime &startedAt, qint64 durationMs)
{
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "INSERT INTO test_sessions (patient_id, user_id, started_at, duration_ms)"
        " VALUES (?, ?, ?, ?)"));
    q.addBindValue(patientId);
    q.addBindValue(userId);
    q.addBindValue(startedAt.toString(Qt::ISODate));
    q.addBindValue(durationMs);
    if (!q.exec()) {
        fail(q.lastError().text());
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool DatabaseManager::addEvents(int sessionId, const QList<TestEvent> &events)
{
    QSqlDatabase database = db();
    if (!database.transaction())
        return fail(database.lastError().text());

    QSqlQuery q(database);
    q.prepare(QStringLiteral(
        "INSERT INTO test_events (session_id, elapsed_min, elapsed_sec, elapsed_ms,"
        " button_index, hit) VALUES (?, ?, ?, ?, ?, ?)"));
    for (const TestEvent &e : events) {
        q.addBindValue(sessionId);
        q.addBindValue(e.minute());
        q.addBindValue(e.second());
        q.addBindValue(e.elapsedMs);
        q.addBindValue(e.buttonIndex);
        q.addBindValue(e.hit ? 1 : 0);
        if (!q.exec()) {
            const QString err = q.lastError().text();
            database.rollback();
            return fail(err);
        }
    }
    if (!database.commit())
        return fail(database.lastError().text());
    return true;
}

QList<TestSession> DatabaseManager::sessions(int patientId)
{
    QList<TestSession> list;
    QSqlQuery q(db());
    QString sql = QStringLiteral(
        "SELECT s.id, s.patient_id, s.user_id, s.started_at, s.duration_ms,"
        " p.first_name || ' ' || p.last_name FROM test_sessions s"
        " JOIN patients p ON p.id = s.patient_id");
    if (patientId >= 0)
        sql += QStringLiteral(" WHERE s.patient_id = ?");
    sql += QStringLiteral(" ORDER BY s.started_at DESC");
    q.prepare(sql);
    if (patientId >= 0)
        q.addBindValue(patientId);
    if (!q.exec()) {
        fail(q.lastError().text());
        return list;
    }
    while (q.next()) {
        TestSession s;
        s.id = q.value(0).toInt();
        s.patientId = q.value(1).toInt();
        s.userId = q.value(2).toInt();
        s.startedAt = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
        s.durationMs = q.value(4).toLongLong();
        s.patientName = q.value(5).toString();
        list.append(s);
    }
    return list;
}

QList<TestEvent> DatabaseManager::events(int sessionId)
{
    QList<TestEvent> list;
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT elapsed_ms, button_index, hit FROM test_events"
        " WHERE session_id = ? ORDER BY elapsed_ms, id"));
    q.addBindValue(sessionId);
    if (!q.exec()) {
        fail(q.lastError().text());
        return list;
    }
    while (q.next()) {
        TestEvent e;
        e.elapsedMs = q.value(0).toLongLong();
        e.buttonIndex = q.value(1).toInt();
        e.hit = q.value(2).toInt() != 0;
        list.append(e);
    }
    return list;
}
