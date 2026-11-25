/**
 * @file main.cpp
 * @brief Punto de entrada: abre la base de datos, autentica y muestra la ventana principal.
 */

#include "database/databasemanager.h"
#include "ui/logindialog.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

namespace {

/** @return Ruta del archivo SQLite (variable AGILITY_DB o carpeta de datos del usuario). */
QString databasePath()
{
    const QByteArray custom = qgetenv("AGILITY_DB");
    if (!custom.isEmpty())
        return QString::fromLocal8Bit(custom);

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/agility.db");
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("AgilityTest"));

    DatabaseManager db;
    if (!db.open(databasePath())) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("No se pudo abrir la base de datos:\n%1").arg(db.lastError()));
        return 1;
    }

    // Repite el ciclo login -> ventana principal mientras el usuario cierre sesión.
    bool again = true;
    while (again) {
        LoginDialog login(db);
        if (login.exec() != QDialog::Accepted)
            break;

        MainWindow window(db, login.user());
        window.show();
        app.exec();
        again = window.loggedOut();
    }
    return 0;
}
