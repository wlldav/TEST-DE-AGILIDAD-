#include "database/databasemanager.h"
#include "ui/agilitywindow.h"
#include "ui/circlebutton.h"
#include "ui/logindialog.h"
#include "ui/mainwindow.h"
#include "ui/patientformdialog.h"
#include "ui/statsdialog.h"
#include "ui/userformdialog.h"

#include <QDir>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QtTest>

/**
 * @class TestGui
 * @brief Prueba de humo de la interfaz: recorre el flujo completo sin pantalla
 *        (QT_QPA_PLATFORM=offscreen) y, si se define SCREENSHOT_DIR, guarda capturas.
 */
class TestGui : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void screenshotsOfForms();
    void fullTestFlow();
    void statsDialogWithData();

private:
    void shot(QWidget *w, const QString &name);
    static void closeModalsLater();

    DatabaseManager m_db;
    User m_user;
    Patient m_patient;
    QString m_shotDir;
};

void TestGui::initTestCase()
{
    m_shotDir = qEnvironmentVariable("SCREENSHOT_DIR");
    if (!m_shotDir.isEmpty())
        QDir().mkpath(m_shotDir);

    QVERIFY(m_db.open(QStringLiteral(":memory:")));

    m_user.setUsername(QStringLiteral("dpalomino"));
    m_user.setFirstName(QStringLiteral("Daniel"));
    m_user.setLastName(QStringLiteral("Palomino"));
    m_user.setDocumentId(QStringLiteral("1010"));
    m_user.setBirthDate(QDate(1999, 3, 8));
    m_user.setId(m_db.addUser(m_user, QStringLiteral("clave123")));
    QVERIFY(m_user.id() > 0);

    m_patient.setFirstName(QStringLiteral("Ana"));
    m_patient.setLastName(QStringLiteral("Gomez"));
    m_patient.setDocumentId(QStringLiteral("52000111"));
    m_patient.setBirthDate(QDate(1985, 7, 21));
    m_patient.setAddress(QStringLiteral("Cra 7 # 12-34, Bogota"));
    m_patient.setRace(QStringLiteral("Mestizo"));
    m_patient.setGender(QStringLiteral("Femenino"));
    m_patient.setIncomeLevel(QStringLiteral("2 a 4 SMMLV"));
    m_patient.setId(m_db.addPatient(m_patient));
    QVERIFY(m_patient.id() > 0);
}

void TestGui::shot(QWidget *w, const QString &name)
{
    if (m_shotDir.isEmpty())
        return;
    QVERIFY(w->grab().save(m_shotDir + QLatin1Char('/') + name + QStringLiteral(".png")));
}

// Cierra cualquier dialogo modal (p. ej. el resumen final) para que el test no se bloquee.
void TestGui::closeModalsLater()
{
    QTimer::singleShot(100, qApp, []() {
        if (QWidget *modal = QApplication::activeModalWidget())
            modal->close();
        closeModalsLater();
    });
}

void TestGui::screenshotsOfForms()
{
    {
        DatabaseManager empty;  // sin usuarios, el login no se autoabre en este caso
        QVERIFY(empty.open(QStringLiteral(":memory:")));
        empty.addUser(m_user, QStringLiteral("x"));
        LoginDialog login(empty);
        login.show();
        QVERIFY(QTest::qWaitForWindowExposed(&login));
        shot(&login, QStringLiteral("01_login"));
    }
    UserFormDialog userForm(m_db);
    userForm.show();
    QVERIFY(QTest::qWaitForWindowExposed(&userForm));
    shot(&userForm, QStringLiteral("02_usuario"));

    PatientFormDialog patientForm(m_db);
    patientForm.show();
    QVERIFY(QTest::qWaitForWindowExposed(&patientForm));
    shot(&patientForm, QStringLiteral("03_paciente"));

    MainWindow main(m_db, m_user);
    main.show();
    QVERIFY(QTest::qWaitForWindowExposed(&main));
    shot(&main, QStringLiteral("04_principal"));
}

void TestGui::fullTestFlow()
{
    closeModalsLater();

    AgilityWindow window(m_db, m_patient, m_user, 3000);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    shot(&window, QStringLiteral("05_prueba_inicio"));

    window.startTest();
    QVERIFY(window.engine().isRunning());
    const QList<CircleButton *> buttons = window.findChildren<CircleButton *>();
    QCOMPARE(buttons.size(), 11);

    // Click sobre el boton encendido: debe registrar un acierto.
    const int lit = window.engine().currentButton();
    QVERIFY(lit >= 0);
    int litCount = 0;
    for (CircleButton *b : buttons)
        litCount += b->isLit() ? 1 : 0;
    QCOMPARE(litCount, 1);  // solo uno encendido a la vez

    // Los botones se crearon en orden; localizamos el encendido y lo presionamos.
    CircleButton *target = nullptr;
    for (CircleButton *b : buttons)
        if (b->isLit())
            target = b;
    QVERIFY(target != nullptr);
    shot(&window, QStringLiteral("06_prueba_en_curso"));
    QTest::mousePress(target, Qt::LeftButton);
    QTest::mouseRelease(target, Qt::LeftButton);
    QVERIFY(!window.engine().events().isEmpty());
    QVERIFY(window.engine().events().first().hit);

    QSignalSpy finished(&window.engine(), &AgilityEngine::finished);
    QVERIFY(finished.wait(6000));
    QVERIFY(window.sessionId() > 0);

    const QList<TestEvent> saved = m_db.events(window.sessionId());
    QVERIFY(saved.size() >= 3);
    QVERIFY(saved.first().hit);
    QCOMPARE(m_db.sessions(m_patient.id()).size(), 1);
}

void TestGui::statsDialogWithData()
{
    // Sesion sintetica de 2 minutos con ~65 % de aciertos, para una grafica representativa.
    QRandomGenerator rng(42);
    QList<TestEvent> events;
    qint64 t = 1000;
    int previous = -1;
    while (t < 120000) {
        int button = static_cast<int>(rng.bounded(11));
        while (button == previous)
            button = static_cast<int>(rng.bounded(11));
        previous = button;
        TestEvent e;
        e.elapsedMs = t;
        e.buttonIndex = button;
        e.hit = rng.bounded(100) < 65;
        events.append(e);
        t += e.hit ? static_cast<qint64>(400 + rng.bounded(500)) : 1000;
    }
    const int sid = m_db.createSession(m_patient.id(), m_user.id(),
                                       QDateTime(QDate(2026, 11, 24), QTime(19, 45)), 120000);
    QVERIFY(sid > 0);
    QVERIFY(m_db.addEvents(sid, events));

    StatsDialog dialog(m_db, sid);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    shot(&dialog, QStringLiteral("07_resultados"));
}

QTEST_MAIN(TestGui)
#include "tst_gui.moc"
