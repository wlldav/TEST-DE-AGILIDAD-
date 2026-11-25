#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "models/user.h"

#include <QDialog>

class DatabaseManager;
class QLineEdit;

/**
 * @class LoginDialog
 * @brief Autenticacion por nombre de usuario y contrasena.
 *
 * Ofrece ademas el acceso al formulario de creacion de usuarios. Si la
 * base de datos aun no tiene usuarios, lo abre automaticamente.
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(DatabaseManager &db, QWidget *parent = nullptr);

    /** @return Usuario autenticado (valido tras aceptar el dialogo). */
    const User &user() const { return m_user; }

protected:
    void accept() override;

private slots:
    void createUser();

private:
    DatabaseManager &m_db;
    User m_user;
    QLineEdit *m_username;
    QLineEdit *m_password;
};

#endif // LOGINDIALOG_H
