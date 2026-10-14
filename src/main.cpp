
#include "ui/MainWindow.hpp"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(
        QStringLiteral("CliSrvValidator")
    );
    QCoreApplication::setApplicationName(
        QStringLiteral("CliSrvValidator")
    );

    MainWindow window;
    window.show();

    return app.exec();
}

