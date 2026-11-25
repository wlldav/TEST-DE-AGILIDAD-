#ifndef PERSON_H
#define PERSON_H

#include <QDate>
#include <QString>

/**
 * @file person.h
 * @brief Clase base que agrupa los datos comunes de usuarios y pacientes.
 */

/**
 * @class Person
 * @brief Datos de identificacion basicos de una persona.
 *
 * Es la clase base de User y Patient. Encapsula nombre, apellido,
 * documento de identidad y fecha de nacimiento, y calcula la edad.
 */
class Person
{
public:
    Person() = default;
    Person(const Person &) = default;
    Person &operator=(const Person &) = default;
    virtual ~Person() = default;

    /** @return Identificador en la base de datos (-1 si aun no se guarda). */
    int id() const { return m_id; }
    /** @param id Identificador asignado por la base de datos. */
    void setId(int id) { m_id = id; }

    QString firstName() const { return m_firstName; }
    void setFirstName(const QString &v) { m_firstName = v; }

    QString lastName() const { return m_lastName; }
    void setLastName(const QString &v) { m_lastName = v; }

    QString documentId() const { return m_documentId; }
    void setDocumentId(const QString &v) { m_documentId = v; }

    QDate birthDate() const { return m_birthDate; }
    void setBirthDate(const QDate &v) { m_birthDate = v; }

    /** @return "Nombre Apellido". */
    QString fullName() const;

    /**
     * @brief Calcula la edad en anios cumplidos.
     * @param on Fecha de referencia (por defecto, hoy).
     * @return Edad en anios, o -1 si la fecha de nacimiento no es valida
     *         o es posterior a la fecha de referencia.
     */
    int age(const QDate &on = QDate::currentDate()) const;

private:
    int m_id = -1;
    QString m_firstName;
    QString m_lastName;
    QString m_documentId;
    QDate m_birthDate;
};

#endif // PERSON_H
