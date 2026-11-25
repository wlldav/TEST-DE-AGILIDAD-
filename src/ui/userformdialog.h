#ifndef USERFORMDIALOG_H
#define USERFORMDIALOG_H

#include "models/user.h"

#include <QDialog>

class DatabaseManager;
class QDateEdit;
class QLineEdit;

/**
 * @class UserFormDialog
 * @brief Formulario de creacion y edicion de usuarios del sistema.
 *
 * Si se entrega un usuario existente, el formulario funciona en modo
 * edicion (la contrasena es opcional); en caso contrario, crea uno nuevo.
 */
class UserFormDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param db Base de datos abierta.
     * @param existing Usuario a editar, o nullptr para crear uno nuevo.
     * @param parent Widget padre.
     */
    UserFormDialog(DatabaseManager &db, const User *existing = nullptr,
                   QWidget *parent = nullptr);

    /** @return El usuario guardado (valido tras aceptar el dialogo). */
    const User &user() const { return m_user; }

protected:
    void accept() override;

private:
    QString validate() const;

    DatabaseManager &m_db;
    User m_user;
    bool m_editing;
    QLineEdit *m_username;
    QLineEdit *m_password;
    QLineEdit *m_confirm;
    QLineEdit *m_firstName;
    QLineEdit *m_lastName;
    QLineEdit *m_document;
    QDateEdit *m_birthDate;
};

#endif // USERFORMDIALOG_H
