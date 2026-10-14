
#include "MainWindow.hpp"

#include "settings/SettingsWindow.hpp"
#include "config/AppSettings.hpp"
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>
#include <QWidget>

namespace {

QString statusColor(ValidationStatus status)
{
    switch (status) {
    case ValidationStatus::Sync:
        return QStringLiteral("#55D98A");
    case ValidationStatus::ExtraMods:
        return QStringLiteral("#E9B949");
    case ValidationStatus::NotSync:
    case ValidationStatus::ServerOffline:
    case ValidationStatus::ManifestError:
        return QStringLiteral("#FF626B");
    case ValidationStatus::NotChecked:
        return QStringLiteral("#A0AAB5");
    }

    return QStringLiteral("#A0AAB5");
}

QString statusName(ValidationStatus status)
{
    switch (status) {
    case ValidationStatus::Sync:
        return QStringLiteral("SYNC");
    case ValidationStatus::ExtraMods:
        return QStringLiteral("EXTRA MODS");
    case ValidationStatus::NotSync:
        return QStringLiteral("NOT SYNC");
    case ValidationStatus::ServerOffline:
        return QStringLiteral("SERVER OFFLINE");
    case ValidationStatus::ManifestError:
        return QStringLiteral("MANIFEST ERROR");
    case ValidationStatus::NotChecked:
        return QStringLiteral("NOT CHECKED");
    }

    return QStringLiteral("UNKNOWN");
}

QString cardStyle()
{
    return QStringLiteral(
        "background-color: #20262D;"
        "border: 1px solid #343B44;"
        "border-radius: 10px;"
        "padding: 12px;"
    );
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("CLI-SRV Validator"));
    resize(620, 650);
    setMinimumSize(520, 570);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(22, 22, 22, 22);
    root->setSpacing(14);

    auto* header = new QHBoxLayout;

    auto* icon = new QLabel(QStringLiteral("◆"), central);
    icon->setStyleSheet(
        "color: #9CC7E8; font-size: 28px; font-weight: bold;"
    );

    auto* heading = new QVBoxLayout;
    auto* title = new QLabel(
        QStringLiteral("CLI-SRV VALIDATOR"), central
    );
    title->setStyleSheet(
        "font-size: 19px; font-weight: bold; color: #F0F3F6;"
    );

    auto* subtitle = new QLabel(
        QStringLiteral("CLIENT INTEGRITY CHECK"), central
    );
    subtitle->setStyleSheet(
        "font-size: 10px; color: #929DA8; letter-spacing: 2px;"
    );

    heading->addWidget(title);
    heading->addWidget(subtitle);

    auto* debugBadge = new QLabel(QStringLiteral("DEBUG"), central);
    debugBadge->setStyleSheet(
        "color: #9CC7E8;"
        "background: #26313A;"
        "border-radius: 5px;"
        "padding: 6px 10px;"
        "font-weight: bold;"
    );

    header->addWidget(icon);
    header->addLayout(heading);
    header->addStretch();
    header->addWidget(debugBadge);

    root->addLayout(header);

    auto* statusCaption = new QLabel(
        QStringLiteral("CLIENT STATUS"), central
    );
    statusCaption->setStyleSheet(
        "color: #929DA8; font-size: 10px; letter-spacing: 2px;"
    );
    root->addWidget(statusCaption);

    auto* statusCard = new QWidget(central);
    statusCard->setStyleSheet(cardStyle());

    auto* statusLayout = new QVBoxLayout(statusCard);

    statusTitle_ = new QLabel(
        QStringLiteral("NOT CHECKED"), statusCard
    );
    statusTitle_->setStyleSheet(
        "color: #A0AAB5; font-size: 25px; font-weight: bold;"
    );

    statusDescription_ = new QLabel(
        QStringLiteral("Нажмите «Проверить», чтобы сверить файлы."),
        statusCard
    );
    statusDescription_->setWordWrap(true);
    statusDescription_->setStyleSheet("color: #C2CBD4;");

    statusLayout->addWidget(statusTitle_);
    statusLayout->addWidget(statusDescription_);
    root->addWidget(statusCard);

    auto* serverCard = new QWidget(central);
    serverCard->setStyleSheet(cardStyle());

    auto* serverLayout = new QHBoxLayout(serverCard);

    auto* serverInfo = new QVBoxLayout;
    auto* serverName = new QLabel(
        QStringLiteral("GAME SERVER"), serverCard
    );
    serverName->setStyleSheet(
        "color: #F0F3F6; font-weight: bold;"
    );

    serverStatus_ = new QLabel(
        QStringLiteral("Не проверен"), serverCard
    );
    serverStatus_->setStyleSheet("color: #929DA8;");

    serverInfo->addWidget(serverName);
    serverInfo->addWidget(serverStatus_);

    auto* playersInfo = new QVBoxLayout;
    playersInfo->setAlignment(Qt::AlignRight);

    playerCount_ = new QLabel(QStringLiteral("—"), serverCard);
    playerCount_->setStyleSheet(
        "color: #F0F3F6; font-size: 18px; font-weight: bold;"
    );

    auto* playersCaption = new QLabel(
        QStringLiteral("PLAYERS"), serverCard
    );
    playersCaption->setStyleSheet(
        "color: #929DA8; font-size: 10px;"
    );

    playersInfo->addWidget(playerCount_);
    playersInfo->addWidget(playersCaption);

    serverLayout->addLayout(serverInfo);
    serverLayout->addStretch();
    serverLayout->addLayout(playersInfo);
    root->addWidget(serverCard);

    auto* debugLabel = new QLabel(
        QStringLiteral("Тестовый ответ сервера"), central
    );
    debugLabel->setStyleSheet("color: #929DA8;");
    root->addWidget(debugLabel);

    scenarioBox_ = new QComboBox(central);
    scenarioBox_->addItem(
        QStringLiteral("Всё совпадает"),
        static_cast<int>(DebugScenario::AllValid)
    );
    scenarioBox_->addItem(
        QStringLiteral("Есть лишние моды"),
        static_cast<int>(DebugScenario::ExtraMods)
    );
    scenarioBox_->addItem(
        QStringLiteral("Отсутствуют моды"),
        static_cast<int>(DebugScenario::MissingMods)
    );
    scenarioBox_->addItem(
        QStringLiteral("Не совпадает SHA-256"),
        static_cast<int>(DebugScenario::HashMismatch)
    );
    scenarioBox_->addItem(
        QStringLiteral("Сервер недоступен"),
        static_cast<int>(DebugScenario::ServerOffline)
    );
    scenarioBox_->addItem(
        QStringLiteral("Ошибка манифеста"),
        static_cast<int>(DebugScenario::InvalidManifest)
    );
    root->addWidget(scenarioBox_);

    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(10);

    auto* checkButton = new QPushButton(
        QStringLiteral("Проверить"), central
    );
    auto* syncButton = new QPushButton(
        QStringLiteral("Синхронизация"), central
    );
    launchButton_ = new QPushButton(
        QStringLiteral("Запуск лаунчера"), central
    );
    auto* settingsButton = new QPushButton(
        QStringLiteral("Настройки"), central
    );

    checkButton->setObjectName("primaryButton");
    launchButton_->setObjectName("primaryButton");

    for (auto* button : {
             checkButton, syncButton, launchButton_, settingsButton
         }) {
        button->setMinimumHeight(40);
        buttons->addWidget(button);
    }

    root->addLayout(buttons);

    auto* pathCaption = new QLabel(
        QStringLiteral("LAUNCHER"), central
    );
    pathCaption->setStyleSheet(
        "color: #929DA8; font-size: 10px; letter-spacing: 2px;"
    );

    launcherPath_ = new QLabel(central);
    launcherPath_->setWordWrap(true);
    launcherPath_->setTextInteractionFlags(
        Qt::TextSelectableByMouse
    );
    launcherPath_->setStyleSheet("color: #929DA8;");

    root->addWidget(pathCaption);
    root->addWidget(launcherPath_);

    auto* logCaption = new QLabel(
        QStringLiteral("EVENT LOG"), central
    );
    logCaption->setStyleSheet(
        "color: #929DA8; font-size: 10px; letter-spacing: 2px;"
    );
    root->addWidget(logCaption);

    eventLog_ = new QPlainTextEdit(central);
    eventLog_->setReadOnly(true);
    eventLog_->setMaximumBlockCount(300);
    eventLog_->setPlaceholderText(
        QStringLiteral("События появятся здесь...")
    );
    eventLog_->setMinimumHeight(100);
    root->addWidget(eventLog_, 1);

    setCentralWidget(central);

    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget {
            background-color: #15191E;
            color: #F0F3F6;
            font-family: sans-serif;
            font-size: 12px;
        }

        QComboBox, QPlainTextEdit {
            background-color: #0D1116;
            border: 1px solid #343B44;
            border-radius: 6px;
            padding: 8px;
        }

        QComboBox QAbstractItemView {
            background-color: #20262D;
            selection-background-color: #344554;
        }

        QPushButton {
            background-color: #26313A;
            border: 1px solid #3B4651;
            border-radius: 6px;
            padding: 8px;
        }

        QPushButton:hover {
            background-color: #344554;
        }

        QPushButton:disabled {
            color: #69737D;
            background-color: #1B2026;
        }

        QPushButton#primaryButton {
            background-color: #36556B;
            border: 1px solid #4D718B;
            font-weight: bold;
        }

        QPushButton#primaryButton:hover {
            background-color: #416982;
        }
    )"));

    connect(checkButton, &QPushButton::clicked,
            this, &MainWindow::validateClient);

    connect(syncButton, &QPushButton::clicked,
            this, &MainWindow::synchronizeClient);

    connect(launchButton_, &QPushButton::clicked,
            this, &MainWindow::launchGame);

    
    // Связываем кнопку настроек с методом openSettings().
    // При нажатии откроется отдельное окно SettingsWindow.
    connect(settingsButton, &QPushButton::clicked,
            this, &MainWindow::openSettings);

    // Читаем сохранённый путь к лаунчеру из AppSettings.
    // MainWindow больше не обязан знать, как именно
    // настройки хранятся на диске.
    const QString savedPath = AppSettings::launcherPath();

    // Показываем путь в главном окне.
    // Если пользователь ещё ничего не выбрал,
    // показываем понятную надпись вместо пустого поля.
    launcherPath_->setText(
        savedPath.isEmpty()
            ? QStringLiteral("Лаунчер не выбран")
            : savedPath
    );

    // Обновляем доступность кнопки запуска с учётом
    // текущего состояния валидации.
    updateLaunchButton();

    // Записываем стартовое событие в журнал.
    appendLog(QStringLiteral("Валидатор запущен в режиме DEBUG."));
}
// Validate Cli 
void MainWindow::validateClient()
{
    const auto scenario = static_cast<DebugScenario>(
        scenarioBox_->currentData().toInt()
    );

    const auto result = serverClient_.fetchManifest(scenario);
    applyResult(result);

    appendLog(
        QStringLiteral("Результат проверки: %1")
            .arg(statusName(result.status))
    );

    for (const auto& detail : result.details) {
        appendLog(QStringLiteral("  • %1").arg(detail));
    }
}

void MainWindow::synchronizeClient()
{
    const auto scenario = static_cast<DebugScenario>(
        scenarioBox_->currentData().toInt()
    );

    const auto result = serverClient_.fetchManifest(scenario);

    if (!result.serverOnline ||
        result.status == ValidationStatus::ManifestError) {
        applyResult(result);
        appendLog(
            QStringLiteral("Синхронизация отменена: сервер недоступен "
                           "или манифест некорректен.")
        );
        return;
    }

    if (result.status == ValidationStatus::Sync) {
        applyResult(result);
        appendLog(QStringLiteral("Изменения не требуются."));
        return;
    }

    // Заглушка: реального скачивания и изменения файлов пока нет.
    appendLog(
        QStringLiteral("DEBUG: имитация синхронизации. "
                       "Файлы на диске не изменялись.")
    );

    auto simulated = result;
    simulated.status = ValidationStatus::Sync;
    simulated.message =
        QStringLiteral("Тестовая синхронизация завершена. "
                       "Реальные файлы не изменялись.");
    simulated.details.clear();

    applyResult(simulated);
    appendLog(QStringLiteral("DEBUG: клиент условно синхронизирован."));
}

void MainWindow::launchGame()
{
    // Получаем путь к лаунчеру через единый интерфейс настроек.
    const QString path = AppSettings::launcherPath();
    if (path.isEmpty()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Лаунчер не настроен"),
            QStringLiteral("Сначала укажите путь к лаунчеру "
                           "в настройках.")
        );
        return;
    }

    const QFileInfo file(path);

    if (!file.exists() || !file.isFile()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Файл не найден"),
            QStringLiteral("Указанный исполняемый файл не существует.")
        );
        appendLog(QStringLiteral("Ошибка: лаунчер не найден: %1").arg(path));
        return;
    }

    if (!file.isExecutable()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Нет прав на запуск"),
            QStringLiteral("Файл не помечен как исполняемый.")
        );
        appendLog(QStringLiteral("Ошибка: файл не исполняемый: %1").arg(path));
        return;
    }

    if (lastResult_.status != ValidationStatus::Sync &&
        lastResult_.status != ValidationStatus::ExtraMods) {
        QMessageBox::warning(
            this,
            QStringLiteral("Проверка не пройдена"),
            QStringLiteral("Сначала проверьте клиент и устраните "
                           "обязательные ошибки синхронизации.")
        );
        return;
    }

    QString error;
    QProcess process;
    process.setProgram(path);
    process.setArguments({});

    if (!process.startDetached()) {
        error = process.errorString();
        QMessageBox::critical(
            this,
            QStringLiteral("Ошибка запуска"),
            QStringLiteral("Не удалось запустить лаунчер:\n%1").arg(error)
        );
        appendLog(QStringLiteral("Ошибка запуска: %1").arg(error));
        return;
    }

    appendLog(QStringLiteral("Лаунчер запущен: %1").arg(path));
    close();
}


void MainWindow::openSettings()
{
    // Создаём отдельное окно настроек.
    // Передаём this, чтобы оно принадлежало главному окну.
    SettingsWindow dialog(this);

    // exec() открывает модальный диалог и ждёт,
    // пока пользователь сохранит настройки или отменит изменения.
    const int result = dialog.exec();

    // Если пользователь нажал Cancel или закрыл окно,
    // ничего в главном окне не меняем.
    if (result != QDialog::Accepted) {
        return;
    }

    // SettingsWindow уже сохранил значения через AppSettings.
    // Повторно читаем путь, чтобы обновить отображение.
    const QString path = AppSettings::launcherPath();

    // Обновляем надпись LAUNCHER в главном окне.
    launcherPath_->setText(
        path.isEmpty()
            ? QStringLiteral("Лаунчер не выбран")
            : path
    );

    // Сообщаем в журнале, что настройки сохранены.
    appendLog(QStringLiteral("Настройки сохранены."));
}

void MainWindow::applyResult(const ValidationResult& result)
{
    lastResult_ = result;

    statusTitle_->setText(statusName(result.status));
    statusTitle_->setStyleSheet(
        QStringLiteral(
            "font-size: 25px; font-weight: bold; color: %1;"
        ).arg(statusColor(result.status))
    );

    statusDescription_->setText(result.message);

    if (result.serverOnline) {
        serverStatus_->setText(QStringLiteral("Online"));
        serverStatus_->setStyleSheet("color: #55D98A;");

        if (result.onlinePlayers >= 0 && result.maxPlayers >= 0) {
            playerCount_->setText(
                QStringLiteral("%1 / %2")
                    .arg(result.onlinePlayers)
                    .arg(result.maxPlayers)
            );
        } else {
            playerCount_->setText(QStringLiteral("—"));
        }
    } else {
        serverStatus_->setText(
            result.status == ValidationStatus::NotChecked
                ? QStringLiteral("Не проверен")
                : QStringLiteral("Offline")
        );
        serverStatus_->setStyleSheet("color: #FF626B;");
        playerCount_->setText(QStringLiteral("—"));
    }

    updateLaunchButton();
}

void MainWindow::appendLog(const QString& message)
{
    const QString time = QDateTime::currentDateTime().toString(
        QStringLiteral("HH:mm:ss")
    );

    eventLog_->appendPlainText(
        QStringLiteral("[%1] %2").arg(time, message)
    );
}

void MainWindow::updateLaunchButton()
{
    const bool canLaunch =
        lastResult_.status == ValidationStatus::Sync ||
        lastResult_.status == ValidationStatus::ExtraMods;

    launchButton_->setEnabled(canLaunch);
}

