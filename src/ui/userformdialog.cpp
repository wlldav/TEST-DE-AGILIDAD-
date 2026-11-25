#include "userformdialog.h"

#include "database/databasemanager.h"

#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

UserFormDialog::UserFormDialog(DatabaseManager &db, const User *existing, QWidget *parent)
    : QDialog(parent), m_db(db), m_editing(existing != nullptr)
{
    setWindowTitle(m_editing ? tr("Editar usuario") : tr("Crear usuario"));

    m_username = new QLineEdit(this);
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);
    m_confirm = new QLineEdit(this);
    m_confirm->setEchoMode(QLineEdit::Password);
    m_firstName = new QLineEdit(this);
    m_lastName = new QLineEdit(this);
    m_document = new QLineEdit(this);
    m_birthDate = new QDateEdit(QDate(2000, 1, 1), this);
    m_birthDate->setCalendarPopup(true);
    m_birthDate->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_birthDate->setMaximumDate(QDate::currentDate());

    if (m_editing) {
        m_user = *existing;
        m_username->setText(existing->username());
        m_firstName->setText(existing->firstName());
        m_lastName->setText(existing->lastName());
        m_document->setText(existing->documentId());
        if (existing->birthDate().isValid())
            m_birthDate->setDate(existing->birthDate());
        m_password->setPlaceholderText(tr("(dejar vacío para no cambiarla)"));
    }

    auto *form = new QFormLayout;
    form->addRow(tr("Usuario:"), m_username);
    form->addRow(tr("Contraseña:"), m_password);
    form->addRow(tr("Confirmar contraseña:"), m_confirm);
    form->addRow(tr("Nombre:"), m_firstName);
    form->addRow(tr("Apellido:"), m_lastName);
    form->addRow(tr("Documento de identidad:"), m_document);
    form->addRow(tr("Fecha de nacimiento:"), m_birthDate);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &UserFormDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &UserFormDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString UserFormDialog::validate() const
{
    const QString username = m_username->text().trimmed();
    if (username.size() < 3)
        return tr("El usuario debe tener al menos 3 caracteres.");
    if (m_db.usernameExists(username, m_editing ? m_user.id() : -1))
        return tr("Ese nombre de usuario ya existe.");

    const QString pass = m_password->text();
    if (!m_editing && pass.isEmpty())
        return tr("Debe escribir una contraseña.");
    if (!pass.isEmpty() && pass.size() < 4)
        return tr("La contraseña debe tener al menos 4 caracteres.");
    if (pass != m_confirm->text())
        return tr("Las contraseñas no coinciden.");

    if (m_firstName->text().trimmed().isEmpty() || m_lastName->text().trimmed().isEmpty())
        return tr("Nombre y apellido son obligatorios.");
    if (m_document->text().trimmed().isEmpty())
        return tr("El documento de identidad es obligatorio.");
    return QString();
}

void UserFormDialog::accept()
{
    const QString error = validate();
    if (!error.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), error);
        return;
    }

    m_user.setUsername(m_username->text().trimmed());
    m_user.setFirstName(m_firstName->text().trimmed());
    m_user.setLastName(m_lastName->text().trimmed());
    m_user.setDocumentId(m_document->text().trimmed());
    m_user.setBirthDate(m_birthDate->date());

    bool ok;
    if (m_editing) {
        ok = m_db.updateUser(m_user, m_password->text());
    } else {
        const int id = m_db.addUser(m_user, m_password->text());
        ok = id > 0;
        if (ok)
            m_user.setId(id);
    }
    if (!ok) {
        QMessageBox::critical(this, windowTitle(),
                              tr("No se pudo guardar (documento repetido?).\n%1").arg(m_db.lastError()));
        return;
    }
    QDialog::accept();
}
