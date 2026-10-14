
#include "AppSettings.hpp"

#include <QSettings>

QString AppSettings::launcherPath()
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    return s.value("launcher/path").toString();
}

void AppSettings::setLauncherPath(const QString& path)
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    s.setValue("launcher/path", path);
}

QString AppSettings::modsPath()
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    return s.value("minecraft/modsPath").toString();
}

void AppSettings::setModsPath(const QString& path)
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    s.setValue("minecraft/modsPath", path);
}

QString AppSettings::ftpUrl()
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    return s.value("server/ftpUrl").toString();
}

void AppSettings::setFtpUrl(const QString& url)
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    s.setValue("server/ftpUrl", url);
}

QString AppSettings::ftpUser()
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    return s.value("server/ftpUser").toString();
}

void AppSettings::setFtpUser(const QString& user)
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    s.setValue("server/ftpUser", user);
}

QString AppSettings::ftpPassword()
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    return s.value("server/ftpPassword").toString();
}

void AppSettings::setFtpPassword(const QString& password)
{
    QSettings s("CliSrvValidator", "CliSrvValidator");
    s.setValue("server/ftpPassword", password);
}

