#include "database/databasemanager.h"
#include "logic/agilityengine.h"
#include "logic/statistics.h"
#include "models/patient.h"
#include "models/user.h"

#include <QtTest>

/**
 * @class TestCore
 * @brief Pruebas automaticas del nucleo: modelos, estadisticas, BD y motor.
 */
class TestCore : public QObject
{
    Q_OBJECT

private slots:
    void ageBeforeAndAfterBirthday();
    void ageInvalidDate();
    void eventMinuteAndSecond();
    void statisticsCounts();
    void authentication();
    void userUpdateAndPassword();
    void duplicatedUserRejected();
    void patientsRoundTrip();
    void duplicatedPatientRejected();
    void sessionAndEventsRoundTrip();
    void engineOneLitAtATimeAndRecords();
    void engineRecordsHitOnClick();
    void engineIgnoresClickOnOffButton();
};

static Person personBorn(int y, int m, int d)
{
    Person p;
    p.setBirthDate(QDate(y, m, d));
    return p;
}

static User sampleUser(const QString &name = QStringLiteral("daniel"))
{
    User u;
    u.setUsername(name);
    u.setFirstName(QStringLiteral("Daniel"));
    u.setLastName(QStringLiteral("Palomino"));
    u.setDocumentId(QStringLiteral("100%1").arg(name));
    u.setBirthDate(QDate(2000, 5, 17));
    return u;
}

static Patient samplePatient(const QString &doc = QStringLiteral("1234"))
{
    Patient p;
    p.setFirstName(QStringLiteral("Ana"));
    p.setLastName(QStringLiteral("Gomez"));
    p.setDocumentId(doc);
    p.setBirthDate(QDate(1990, 1, 31));
    p.setAddress(QStringLiteral("Cra 7 # 1-23"));
    p.setRace(QStringLiteral("Mestizo"));
    p.setGender(QStringLiteral("Femenino"));
    p.setIncomeLevel(QStringLiteral("1 a 2 SMMLV"));
    return p;
}

void TestCore::ageBeforeAndAfterBirthday()
{
    const Person p = personBorn(2000, 6, 15);
    QCOMPARE(p.age(QDate(2026, 6, 14)), 25);  // un dia antes del cumpleanos
    QCOMPARE(p.age(QDate(2026, 6, 15)), 26);  // el dia del cumpleanos
    QCOMPARE(p.age(QDate(2026, 12, 31)), 26);
    QCOMPARE(p.age(QDate(2000, 6, 15)), 0);
}

void TestCore::ageInvalidDate()
{
    QCOMPARE(Person().age(), -1);
    QCOMPARE(personBorn(2030, 1, 1).age(QDate(2026, 1, 1)), -1);
}

void TestCore::eventMinuteAndSecond()
{
    TestEvent e;
    e.elapsedMs = 2 * 60000 + 37 * 1000 + 500;
    QCOMPARE(e.minute(), 2);
    QCOMPARE(e.second(), 37);
}

void TestCore::statisticsCounts()
{
    QList<TestEvent> ev;
    ev.append({1000, 0, true});
    ev.append({2000, 0, false});
    ev.append({3000, 3, true});
    ev.append({4000, 3, true});
    const SessionStats s = Statistics::compute(ev, 4);
    QCOMPARE(s.total, 4);
    QCOMPARE(s.hits, 3);
    QCOMPARE(s.misses, 1);
    QCOMPARE(s.hitsPerButton[3], 2);
    QCOMPARE(s.missesPerButton[0], 1);
    QCOMPARE(s.accuracy(), 75.0);
    QCOMPARE(Statistics::hitsPerMinute(s, 60000), 3.0);
    QCOMPARE(SessionStats().accuracy(), 0.0);
}

void TestCore::authentication()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    QCOMPARE(db.userCount(), 0);
    QVERIFY(db.addUser(sampleUser(), QStringLiteral("secreto")) > 0);
    QCOMPARE(db.userCount(), 1);

    const auto ok = db.authenticate(QStringLiteral("daniel"), QStringLiteral("secreto"));
    QVERIFY(ok.has_value());
    QCOMPARE(ok->fullName(), QStringLiteral("Daniel Palomino"));
    QCOMPARE(ok->birthDate(), QDate(2000, 5, 17));

    QVERIFY(!db.authenticate(QStringLiteral("daniel"), QStringLiteral("otra")).has_value());
    QVERIFY(!db.authenticate(QStringLiteral("nadie"), QStringLiteral("secreto")).has_value());
}

void TestCore::userUpdateAndPassword()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    User u = sampleUser();
    u.setId(db.addUser(u, QStringLiteral("secreto")));

    u.setFirstName(QStringLiteral("Dani"));
    QVERIFY(db.updateUser(u));  // sin cambiar contrasena
    QVERIFY(db.authenticate(QStringLiteral("daniel"), QStringLiteral("secreto")).has_value());

    QVERIFY(db.updateUser(u, QStringLiteral("nueva")));
    QVERIFY(!db.authenticate(QStringLiteral("daniel"), QStringLiteral("secreto")).has_value());
    const auto ok = db.authenticate(QStringLiteral("daniel"), QStringLiteral("nueva"));
    QVERIFY(ok.has_value());
    QCOMPARE(ok->firstName(), QStringLiteral("Dani"));
}

void TestCore::duplicatedUserRejected()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    QVERIFY(db.addUser(sampleUser(), QStringLiteral("a")) > 0);
    QCOMPARE(db.addUser(sampleUser(), QStringLiteral("b")), -1);
    QVERIFY(db.usernameExists(QStringLiteral("daniel")));
    QVERIFY(!db.usernameExists(QStringLiteral("daniel"), 1));  // excluyendose a si mismo
    QVERIFY(!db.lastError().isEmpty());
}

void TestCore::patientsRoundTrip()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    Patient p = samplePatient();
    p.setId(db.addPatient(p));
    QVERIFY(p.id() > 0);

    QList<Patient> list = db.patients();
    QCOMPARE(list.size(), 1);
    QCOMPARE(list[0].address(), QStringLiteral("Cra 7 # 1-23"));
    QCOMPARE(list[0].incomeLevel(), QStringLiteral("1 a 2 SMMLV"));
    QCOMPARE(list[0].birthDate(), QDate(1990, 1, 31));

    p.setAddress(QStringLiteral("Calle 1"));
    QVERIFY(db.updatePatient(p));
    QCOMPARE(db.patients()[0].address(), QStringLiteral("Calle 1"));
}

void TestCore::duplicatedPatientRejected()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    QVERIFY(db.addPatient(samplePatient()) > 0);
    QCOMPARE(db.addPatient(samplePatient()), -1);
}

void TestCore::sessionAndEventsRoundTrip()
{
    DatabaseManager db;
    QVERIFY(db.open(QStringLiteral(":memory:")));
    const int uid = db.addUser(sampleUser(), QStringLiteral("x"));
    const int pid = db.addPatient(samplePatient());

    const QDateTime start(QDate(2026, 11, 24), QTime(19, 30, 0));
    const int sid = db.createSession(pid, uid, start, 120000);
    QVERIFY(sid > 0);

    QList<TestEvent> ev;
    ev.append({1000, 2, true});
    ev.append({2000, 5, false});
    ev.append({65500, 10, true});
    QVERIFY(db.addEvents(sid, ev));

    const QList<TestEvent> back = db.events(sid);
    QCOMPARE(back.size(), 3);
    QCOMPARE(back[2].minute(), 1);
    QCOMPARE(back[2].second(), 5);
    QCOMPARE(back[1].hit, false);
    QCOMPARE(back[0].buttonIndex, 2);

    const QList<TestSession> sessions = db.sessions(pid);
    QCOMPARE(sessions.size(), 1);
    QCOMPARE(sessions[0].startedAt, start);
    QCOMPARE(sessions[0].durationMs, qint64(120000));
    QCOMPARE(sessions[0].patientName, QStringLiteral("Ana Gomez"));
    QCOMPARE(db.sessions(pid + 99).size(), 0);

    // Integridad referencial: no se acepta un evento de una sesion inexistente.
    QVERIFY(!db.addEvents(9999, ev));
    QCOMPARE(db.events(sid).size(), 3);
}

void TestCore::engineOneLitAtATimeAndRecords()
{
    AgilityEngine engine(11, 30);
    int lit = 0;
    int off = 0;
    int maxSimultaneous = 0;
    connect(&engine, &AgilityEngine::buttonLit, this, [&](int) {
        ++lit;
        maxSimultaneous = qMax(maxSimultaneous, lit - off);
    });
    connect(&engine, &AgilityEngine::buttonOff, this, [&](int) { ++off; });

    QSignalSpy finished(&engine, &AgilityEngine::finished);
    engine.start(400);
    QVERIFY(engine.isRunning());
    QVERIFY(finished.wait(2000));
    QVERIFY(!engine.isRunning());

    QCOMPARE(maxSimultaneous, 1);
    QCOMPARE(lit, off);  // al terminar, todo queda apagado
    QVERIFY(engine.events().size() >= 8);  // ~13 ciclos de 30 ms en 400 ms
    for (const TestEvent &e : engine.events()) {
        QVERIFY(!e.hit);  // nadie hizo click: todos son fallos
        QVERIFY(e.buttonIndex >= 0 && e.buttonIndex < 11);
        QVERIFY(e.elapsedMs <= 400 + 50);
    }
}

void TestCore::engineRecordsHitOnClick()
{
    AgilityEngine engine(11, 500);
    int litButton = -1;
    connect(&engine, &AgilityEngine::buttonLit, this, [&](int i) { litButton = i; });

    engine.start(5000);
    QVERIFY(litButton >= 0);
    const int first = litButton;
    engine.registerClick(first);  // click correcto

    QCOMPARE(engine.events().size(), 1);
    QVERIFY(engine.events()[0].hit);
    QCOMPARE(engine.events()[0].buttonIndex, first);
    QVERIFY(litButton != first);  // el siguiente es otro boton
    engine.stop();
}

void TestCore::engineIgnoresClickOnOffButton()
{
    AgilityEngine engine(11, 500);
    int litButton = -1;
    connect(&engine, &AgilityEngine::buttonLit, this, [&](int i) { litButton = i; });

    engine.start(5000);
    engine.registerClick((litButton + 1) % 11);  // boton apagado
    QCOMPARE(engine.events().size(), 0);
    engine.stop();
    engine.registerClick(litButton);  // prueba detenida
    QCOMPARE(engine.events().size(), 0);
}

QTEST_MAIN(TestCore)
#include "tst_core.moc"
