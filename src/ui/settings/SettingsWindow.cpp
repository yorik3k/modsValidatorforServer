
#include "SettingsWindow.hpp"

#include "config/AppSettings.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

SettingsWindow::SettingsWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Настройки"));
    setMinimumWidth(560);

    auto* root = new QVBoxLayout(this);
    auto* form = new QFormLayout;

    // Лаунчер
    launcherEdit_ = new QLineEdit(AppSettings::launcherPath(), this);
    launcherEdit_->setPlaceholderText(
        QStringLiteral("Путь к исполняемому файлу")
    );

    auto* launcherBrowse = new QPushButton(
        QStringLiteral("Обзор..."), this
    );

    auto* launcherRow = new QWidget(this);
    auto* launcherRowLayout = new QHBoxLayout(launcherRow);
    launcherRowLayout->setContentsMargins(0, 0, 0, 0);
    launcherRowLayout->addWidget(launcherEdit_);
    launcherRowLayout->addWidget(launcherBrowse);

    connect(launcherBrowse, &QPushButton::clicked,
            this, &SettingsWindow::browseLauncher);

    form->addRow(QStringLiteral("Лаунчер:"), launcherRow);

    // Папка mods
    modsEdit_ = new QLineEdit(AppSettings::modsPath(), this);
    modsEdit_->setPlaceholderText(
        QStringLiteral("Путь к папке mods")
    );

    auto* modsBrowse = new QPushButton(
        QStringLiteral("Обзор..."), this
    );

    auto* modsRow = new QWidget(this);
    auto* modsRowLayout = new QHBoxLayout(modsRow);
    modsRowLayout->setContentsMargins(0, 0, 0, 0);
    modsRowLayout->addWidget(modsEdit_);
    modsRowLayout->addWidget(modsBrowse);

    connect(modsBrowse, &QPushButton::clicked,
            this, &SettingsWindow::browseModsDirectory);

    form->addRow(QStringLiteral("Папка mods:"), modsRow);

    // FTP
    ftpUrlEdit_ = new QLineEdit(AppSettings::ftpUrl(), this);
    ftpUrlEdit_->setPlaceholderText(
        QStringLiteral("ftp://example.com:21/modpack/")
    );
    form->addRow(QStringLiteral("FTP-адрес:"), ftpUrlEdit_);

    ftpUserEdit_ = new QLineEdit(AppSettings::ftpUser(), this);
    ftpUserEdit_->setPlaceholderText(
        QStringLiteral("Имя пользователя")
    );
    form->addRow(QStringLiteral("FTP-пользователь:"), ftpUserEdit_);

    ftpPasswordEdit_ = new QLineEdit(
        AppSettings::ftpPassword(), this
    );
    ftpPasswordEdit_->setEchoMode(QLineEdit::Password);
    ftpPasswordEdit_->setPlaceholderText(
        QStringLiteral("Пароль")
    );
    form->addRow(QStringLiteral("FTP-пароль:"), ftpPasswordEdit_);

    root->addLayout(form);

    auto* note = new QLabel(
        QStringLiteral(
            "FTP пока не подключён. Значения сохраняются "
            "для последующего подключения к серверу."
        ),
        this
    );
    note->setWordWrap(true);
    note->setStyleSheet("color: #929DA8;");
    root->addWidget(note);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel,
        this
    );

    connect(buttons, &QDialogButtonBox::accepted,
            this, &SettingsWindow::saveSettings);
    connect(buttons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    root->addWidget(buttons);

    setStyleSheet(QStringLiteral(R"(
        QDialog {
            background-color: #15191E;
            color: #F0F3F6;
        }

        QLineEdit {
            background-color: #0D1116;
            border: 1px solid #343B44;
            border-radius: 6px;
            padding: 8px;
            color: #F0F3F6;
        }

        QPushButton {
            background-color: #26313A;
            border: 1px solid #3B4651;
            border-radius: 6px;
            padding: 8px;
            color: #F0F3F6;
        }

        QPushButton:hover {
            background-color: #344554;
        }

        QLabel {
            color: #F0F3F6;
        }
    )"));
}

void SettingsWindow::browseLauncher()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Выберите лаунчер"),
        launcherEdit_->text()
    );

    if (!path.isEmpty()) {
        launcherEdit_->setText(path);
    }
}

void SettingsWindow::browseModsDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(
        this,
        QStringLiteral("Выберите папку mods"),
        modsEdit_->text()
    );

    if (!path.isEmpty()) {
        modsEdit_->setText(path);
    }
}

void SettingsWindow::saveSettings()
{
    const QString launcher = launcherEdit_->text().trimmed();
    const QString mods = modsEdit_->text().trimmed();
    const QString ftpUrl = ftpUrlEdit_->text().trimmed();

    if (!launcher.isEmpty()) {
        const QFileInfo info(launcher);

        if (!info.exists() || !info.isFile()) {
            QMessageBox::warning(
                this,
                QStringLiteral("Неверный путь"),
                QStringLiteral("Файл лаунчера не найден.")
            );
            return;
        }
    }

    if (!mods.isEmpty() && !QDir(mods).exists()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Неверный путь"),
            QStringLiteral("Указанная папка mods не найдена.")
        );
        return;
    }

    if (!ftpUrl.isEmpty() &&
        !ftpUrl.startsWith(QStringLiteral("ftp://"),
                           Qt::CaseInsensitive) &&
        !ftpUrl.startsWith(QStringLiteral("ftps://"),
                           Qt::CaseInsensitive)) {
        QMessageBox::warning(
            this,
            QStringLiteral("Неверный адрес"),
            QStringLiteral("Адрес должен начинаться с ftp:// или ftps://.")
        );
        return;
    }

    AppSettings::setLauncherPath(launcher);
    AppSettings::setModsPath(mods);
    AppSettings::setFtpUrl(ftpUrl);
    AppSettings::setFtpUser(ftpUserEdit_->text().trimmed());
    AppSettings::setFtpPassword(ftpPasswordEdit_->text());

    accept();
}

