
#pragma once

#include <QString>

class AppSettings final {
public:
    [[nodiscard]] static QString launcherPath();
    static void setLauncherPath(const QString& path);

    [[nodiscard]] static QString modsPath();
    static void setModsPath(const QString& path);

    [[nodiscard]] static QString ftpUrl();
    static void setFtpUrl(const QString& url);

    [[nodiscard]] static QString ftpUser();
    static void setFtpUser(const QString& user);

    [[nodiscard]] static QString ftpPassword();
    static void setFtpPassword(const QString& password);
};

