#include "patientformdialog.h"

#include "database/databasemanager.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

PatientFormDialog::PatientFormDialog(DatabaseManager &db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle(tr("Nuevo paciente"));

    m_firstName = new QLineEdit(this);
    m_lastName = new QLineEdit(this);
    m_document = new QLineEdit(this);
    m_birthDate = new QDateEdit(QDate(1990, 1, 1), this);
    m_birthDate->setCalendarPopup(true);
    m_birthDate->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_birthDate->setMaximumDate(QDate::currentDate());
    m_address = new QLineEdit(this);

    m_race = new QComboBox(this);
    m_race->addItems({tr("Mestizo"), tr("Blanco"), tr("Afrocolombiano"), tr("Indígena"),
                      tr("Otro"), tr("Prefiere no decir")});
    m_gender = new QComboBox(this);
    m_gender->addItems({tr("Femenino"), tr("Masculino"), tr("Otro"), tr("Prefiere no decir")});
    m_income = new QComboBox(this);
    m_income->addItems({tr("Menos de 1 SMMLV"), tr("1 a 2 SMMLV"), tr("2 a 4 SMMLV"),
                        tr("4 a 8 SMMLV"), tr("Más de 8 SMMLV")});

    auto *form = new QFormLayout;
    form->addRow(tr("Nombre:"), m_firstName);
    form->addRow(tr("Apellido:"), m_lastName);
    form->addRow(tr("Documento de identidad:"), m_document);
    form->addRow(tr("Fecha de nacimiento:"), m_birthDate);
    form->addRow(tr("Dirección:"), m_address);
    form->addRow(tr("Raza:"), m_race);
    form->addRow(tr("Género:"), m_gender);
    form->addRow(tr("Ingresos familiares:"), m_income);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &PatientFormDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &PatientFormDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void PatientFormDialog::accept()
{
    if (m_firstName->text().trimmed().isEmpty() || m_lastName->text().trimmed().isEmpty()
        || m_document->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Nombre, apellido y documento son obligatorios."));
        return;
    }

    m_patient.setFirstName(m_firstName->text().trimmed());
    m_patient.setLastName(m_lastName->text().trimmed());
    m_patient.setDocumentId(m_document->text().trimmed());
    m_patient.setBirthDate(m_birthDate->date());
    m_patient.setAddress(m_address->text().trimmed());
    m_patient.setRace(m_race->currentText());
    m_patient.setGender(m_gender->currentText());
    m_patient.setIncomeLevel(m_income->currentText());

    const int id = m_db.addPatient(m_patient);
    if (id < 0) {
        QMessageBox::critical(this, windowTitle(),
                              tr("No se pudo guardar. Es posible que el documento ya este registrado.\n%1")
                                  .arg(m_db.lastError()));
        return;
    }
    m_patient.setId(id);
    QDialog::accept();
}
