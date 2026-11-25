#include "mainwindow.h"

#include "agilitywindow.h"
#include "database/databasemanager.h"
#include "patientformdialog.h"
#include "statsdialog.h"
#include "userformdialog.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

MainWindow::MainWindow(DatabaseManager &db, const User &user, QWidget *parent)
    : QMainWindow(parent), m_db(db), m_user(user)
{
    setWindowTitle(tr("Prueba de agilidad"));

    m_welcome = new QLabel(this);

    // Paciente
    m_patientCombo = new QComboBox(this);
    m_patientCombo->setMinimumWidth(260);
    auto *newPatientButton = new QPushButton(tr("Nuevo paciente"), this);
    connect(newPatientButton, &QPushButton::clicked, this, &MainWindow::newPatient);
    auto *patientBox = new QGroupBox(tr("Paciente"), this);
    auto *patientLayout = new QHBoxLayout(patientBox);
    patientLayout->addWidget(m_patientCombo, 1);
    patientLayout->addWidget(newPatientButton);

    // Prueba
    m_minutes = new QSpinBox(this);
    m_minutes->setRange(1, 60);
    m_minutes->setValue(1);
    m_minutes->setSuffix(tr(" min"));
    auto *startButton = new QPushButton(tr("Iniciar prueba"), this);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startTest);
    auto *testBox = new QGroupBox(tr("Prueba"), this);
    auto *testLayout = new QHBoxLayout(testBox);
    testLayout->addWidget(new QLabel(tr("Duración:"), this));
    testLayout->addWidget(m_minutes);
    testLayout->addStretch();
    testLayout->addWidget(startButton);

    // Acciones
    auto *resultsButton = new QPushButton(tr("Resultados"), this);
    auto *profileButton = new QPushButton(tr("Editar mi perfil"), this);
    auto *logoutButton = new QPushButton(tr("Cerrar sesión"), this);
    connect(resultsButton, &QPushButton::clicked, this, &MainWindow::showResults);
    connect(profileButton, &QPushButton::clicked, this, &MainWindow::editProfile);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
    auto *actions = new QHBoxLayout;
    actions->addWidget(resultsButton);
    actions->addWidget(profileButton);
    actions->addStretch();
    actions->addWidget(logoutButton);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->addWidget(m_welcome);
    layout->addWidget(patientBox);
    layout->addWidget(testBox);
    layout->addLayout(actions);
    setCentralWidget(central);

    updateWelcome();
    reloadPatients();
}

void MainWindow::updateWelcome()
{
    m_welcome->setText(tr("Bienvenido, %1  (edad: %2 años)")
                           .arg(m_user.fullName())
                           .arg(m_user.age()));
}

void MainWindow::reloadPatients(int selectId)
{
    m_patients = m_db.patients();
    m_patientCombo->clear();
    int select = -1;
    for (int i = 0; i < m_patients.size(); ++i) {
        const Patient &p = m_patients.at(i);
        m_patientCombo->addItem(tr("%1, %2 (%3)").arg(p.lastName(), p.firstName(), p.documentId()));
        if (p.id() == selectId)
            select = i;
    }
    if (select >= 0)
        m_patientCombo->setCurrentIndex(select);
}

void MainWindow::newPatient()
{
    PatientFormDialog form(m_db, this);
    if (form.exec() == QDialog::Accepted)
        reloadPatients(form.patient().id());
}

void MainWindow::startTest()
{
    const int index = m_patientCombo->currentIndex();
    if (index < 0 || index >= m_patients.size()) {
        QMessageBox::information(this, windowTitle(),
                                 tr("Seleccione o registre un paciente antes de iniciar la prueba."));
        return;
    }
    const qint64 durationMs = static_cast<qint64>(m_minutes->value()) * 60 * 1000;
    AgilityWindow window(m_db, m_patients.at(index), m_user, durationMs, this);
    window.exec();
}

void MainWindow::showResults()
{
    StatsDialog dialog(m_db, -1, this);
    dialog.exec();
}

void MainWindow::editProfile()
{
    UserFormDialog form(m_db, &m_user, this);
    if (form.exec() == QDialog::Accepted) {
        m_user = form.user();
        updateWelcome();
    }
}

void MainWindow::logout()
{
    m_loggedOut = true;
    close();
}
