
#pragma once

#include <QDialog>

class QLineEdit;

class SettingsWindow final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget* parent = nullptr);

private:
    QLineEdit* launcherEdit_ = nullptr;
    QLineEdit* modsEdit_ = nullptr;
    QLineEdit* ftpUrlEdit_ = nullptr;
    QLineEdit* ftpUserEdit_ = nullptr;
    QLineEdit* ftpPasswordEdit_ = nullptr;

    void browseLauncher();
    void browseModsDirectory();
    void saveSettings();
};

