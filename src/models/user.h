#ifndef USER_H
#define USER_H

#include "person.h"

/**
 * @class User
 * @brief Usuario registrado que opera la aplicacion (el examinador).
 *
 * Extiende Person con el nombre de usuario. La contrasena nunca se guarda
 * en este objeto: la base de datos almacena unicamente su hash con sal.
 */
class User : public Person
{
public:
    QString username() const { return m_username; }
    void setUsername(const QString &v) { m_username = v; }

private:
    QString m_username;
};

#endif // USER_H
