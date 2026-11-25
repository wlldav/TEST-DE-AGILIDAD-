#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "models/patient.h"
#include "models/testrecords.h"
#include "models/user.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include <optional>

/**
 * @class DatabaseManager
 * @brief Acceso a la base de datos embebida SQLite3.
 *
 * Es la unica clase que contiene SQL. Crea el esquema normalizado
 * (users, patients, test_sessions, test_events) y ofrece operaciones
 * de alto nivel sobre los modelos del dominio.
 *
 * Las contrasenas se guardan como SHA-256(sal + contrasena) con una
 * sal aleatoria por usuario.
 */
class DatabaseManager
{
public:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    /**
     * @brief Abre (y crea si hace falta) la base de datos.
     * @param path Ruta del archivo SQLite, o ":memory:" para pruebas.
     * @return true si se abrio y el esquema esta listo.
     */
    bool open(const QString &path);
    /** @brief Cierra la conexion. */
    void close();
    bool isOpen() const;
    /** @return Texto del ultimo error ocurrido. */
    QString lastError() const { return m_lastError; }

    // ---- Usuarios -------------------------------------------------------
    /** @return Id del usuario creado o -1 si falla (p. ej. usuario duplicado). */
    int addUser(const User &user, const QString &password);
    /** @brief Actualiza datos; si @p newPassword no esta vacia, tambien la contrasena. */
    bool updateUser(const User &user, const QString &newPassword = QString());
    /** @return El usuario si las credenciales son correctas. */
    std::optional<User> authenticate(const QString &username, const QString &password);
    bool usernameExists(const QString &username, int excludeId = -1);
    int userCount();

    // ---- Pacientes ------------------------------------------------------
    /** @return Id del paciente creado o -1 si falla (p. ej. documento duplicado). */
    int addPatient(const Patient &patient);
    bool updatePatient(const Patient &patient);
    QList<Patient> patients();

    // ---- Pruebas --------------------------------------------------------
    /** @return Id de la sesion creada o -1. */
    int createSession(int patientId, int userId, const QDateTime &startedAt,
                      qint64 durationMs);
    /** @brief Guarda todos los eventos de una sesion en una sola transaccion. */
    bool addEvents(int sessionId, const QList<TestEvent> &events);
    /** @param patientId Filtra por paciente; -1 devuelve todas. */
    QList<TestSession> sessions(int patientId = -1);
    QList<TestEvent> events(int sessionId);

private:
    bool createSchema();
    bool fail(const QString &message);
    QSqlDatabase db() const;
    static QString hashPassword(const QString &salt, const QString &password);

    QString m_connectionName;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
