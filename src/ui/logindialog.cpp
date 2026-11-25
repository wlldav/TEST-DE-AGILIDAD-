#include "logindialog.h"

#include "database/databasemanager.h"
#include "userformdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

LoginDialog::LoginDialog(DatabaseManager &db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle(tr("Iniciar sesión"));

    m_username = new QLineEdit(this);
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);

    auto *form = new QFormLayout;
    form->addRow(tr("Usuario:"), m_username);
    form->addRow(tr("Contraseña:"), m_password);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Entrar"));
    auto *createButton = buttons->addButton(tr("Crear usuario"), QDialogButtonBox::ActionRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &LoginDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::createUser);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if (m_db.userCount() == 0)
        QTimer::singleShot(0, this, &LoginDialog::createUser);
}

void LoginDialog::accept()
{
    const auto user = m_db.authenticate(m_username->text().trimmed(), m_password->text());
    if (!user) {
        QMessageBox::warning(this, windowTitle(), tr("Usuario o contraseña incorrectos."));
        m_password->clear();
        m_password->setFocus();
        return;
    }
    m_user = *user;
    QDialog::accept();
}

void LoginDialog::createUser()
{
    UserFormDialog form(m_db, nullptr, this);
    if (form.exec() == QDialog::Accepted) {
        m_username->setText(form.user().username());
        m_password->clear();
        m_password->setFocus();
    }
}
