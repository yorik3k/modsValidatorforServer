
#include "MainWindow.hpp"

#include "settings/SettingsWindow.hpp"
#include "config/AppSettings.hpp"

#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QProcess>
#include <QWidget>

namespace {

class BackgroundWidget final : public QWidget
{
public:
    explicit BackgroundWidget(QWidget* parent = nullptr)
        : QWidget(parent),
          background_(":/images/background.png")
    {
        setAttribute(Qt::WA_StyledBackground, false);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);

        if (!background_.isNull()) {
            painter.drawPixmap(rect(), background_);
        } else {
            painter.fillRect(rect(), QColor("#15191E"));
        }
    }

private:
    QPixmap background_;
};

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
        "background-color: rgba(32, 38, 45, 215);"
        "border: 1px solid rgba(100, 115, 130, 150);"
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
    setMinimumSize(520, 0);

    auto* central = new BackgroundWidget(this);
    central->setObjectName("centralWidget");

    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(22, 22, 22, 22);
    root->setSpacing(14);
    root->setAlignment(Qt::AlignTop);

    // Header

    auto* header = new QHBoxLayout;

    auto* icon = new QLabel(QStringLiteral("◆"), central);
    icon->setStyleSheet(
        "color: #9CC7E8;"
        "font-size: 28px;"
        "font-weight: bold;"
    );

    auto* heading = new QVBoxLayout;

    auto* title = new QLabel(
        QStringLiteral("CLI-SRV VALIDATOR"),
        central
    );
    title->setStyleSheet(
        "font-size: 19px;"
        "font-weight: bold;"
        "color: #F0F3F6;"
    );

    auto* subtitle = new QLabel(
        QStringLiteral("CLIENT INTEGRITY CHECK"),
        central
    );
    subtitle->setStyleSheet(
        "font-size: 10px;"
        "color: #929DA8;"
        "letter-spacing: 2px;"
    );

    heading->addWidget(title);
    heading->addWidget(subtitle);

    modeBox_ = new QComboBox(central);
    modeBox_->addItem(QStringLiteral("User Mode"));
    modeBox_->addItem(QStringLiteral("Dev Mode"));
    modeBox_->setCurrentIndex(1);
    modeBox_->setToolTip(
        QStringLiteral(
            "Переключение пользовательского режима "
            "и режима разработчика"
        )
    );

    header->addWidget(icon);
    header->addLayout(heading);
    header->addStretch();
    header->addWidget(modeBox_);

    root->addLayout(header);

    // Client status

    auto* statusCaption = new QLabel(
        QStringLiteral("CLIENT STATUS"),
        central
    );
    statusCaption->setStyleSheet(
        "color: #929DA8;"
        "font-size: 10px;"
        "letter-spacing: 2px;"
    );
    root->addWidget(statusCaption);

    auto* statusCard = new QWidget(central);
    statusCard->setStyleSheet(
        "background-color: rgba(32, 38, 45, 215);"
        "border: 1px solid rgba(100, 115, 130, 150);"
        "border-radius: 8px;"
    );
    statusCard->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
    );
    statusCard->setFixedHeight(40);

    auto* statusLayout = new QVBoxLayout(statusCard);
    statusLayout->setContentsMargins(10, 3, 10, 3);
    statusLayout->setSpacing(0);

    auto* statusHeader = new QHBoxLayout;
    statusHeader->setSpacing(7);

    statusIndicator_ = new QLabel(statusCard);
    statusIndicator_->setFixedSize(10, 10);
    statusIndicator_->setStyleSheet(
        "background-color: #A0AAB5;"
        "border-radius: 5px;"
    );

    statusTitle_ = new QLabel(
        QStringLiteral("NOT CHECKED"),
        statusCard
    );
    statusTitle_->setStyleSheet(
        "color: #A0AAB5;"
        "font-size: 15px;"
        "font-weight: bold;"
    );

    statusDescription_ = new QLabel(
        QStringLiteral("Ожидает проверки"),
        statusCard
    );
    statusDescription_->setWordWrap(false);
    statusDescription_->setSizePolicy(
        QSizePolicy::Ignored,
        QSizePolicy::Preferred
    );
    statusDescription_->setStyleSheet(
        "color: #C2CBD4;"
        "font-size: 12px;"
    );

    statusHeader->addWidget(statusIndicator_);
    statusHeader->addWidget(statusTitle_);
    statusHeader->addWidget(statusDescription_, 1);

    statusLayout->addLayout(statusHeader);
    root->addWidget(statusCard);

    // Server card

    auto* serverCard = new QWidget(central);
    serverCard->setStyleSheet(cardStyle());
    serverCard->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
    );

    auto* serverLayout = new QHBoxLayout(serverCard);

    auto* serverInfo = new QVBoxLayout;

    auto* serverName = new QLabel(
        QStringLiteral("GAME SERVER"),
        serverCard
    );
    serverName->setStyleSheet(
        "color: #F0F3F6;"
        "font-weight: bold;"
    );

    serverStatus_ = new QLabel(
        QStringLiteral("Не проверен"),
        serverCard
    );
    serverStatus_->setStyleSheet("color: #929DA8;");

    serverInfo->addWidget(serverName);
    serverInfo->addWidget(serverStatus_);

    auto* playersInfo = new QVBoxLayout;
    playersInfo->setAlignment(Qt::AlignRight);

    playerCount_ = new QLabel(
        QStringLiteral("—"),
        serverCard
    );
    playerCount_->setStyleSheet(
        "color: #F0F3F6;"
        "font-size: 18px;"
        "font-weight: bold;"
    );

    auto* playersCaption = new QLabel(
        QStringLiteral("PLAYERS"),
        serverCard
    );
    playersCaption->setStyleSheet(
        "color: #929DA8;"
        "font-size: 10px;"
    );

    playersInfo->addWidget(playerCount_);
    playersInfo->addWidget(playersCaption);

    serverLayout->addLayout(serverInfo);
    serverLayout->addStretch();
    serverLayout->addLayout(playersInfo);

    root->addWidget(serverCard);

    // Developer mode: test scenarios

    debugLabel_ = new QLabel(
        QStringLiteral("Тестовый ответ сервера"),
        central
    );
    debugLabel_->setStyleSheet("color: #929DA8;");
    root->addWidget(debugLabel_);

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

    // Action buttons

    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(10);

    auto* checkButton = new QPushButton(
        QStringLiteral("Проверить"),
        central
    );
    auto* syncButton = new QPushButton(
        QStringLiteral("Синхронизация"),
        central
    );

    launchButton_ = new QPushButton(
        QStringLiteral("Запуск лаунчера"),
        central
    );

    auto* settingsButton = new QPushButton(
        QStringLiteral("Настройки"),
        central
    );

    checkButton->setObjectName("primaryButton");
    launchButton_->setObjectName("primaryButton");

    for (auto* button : {
             checkButton,
             syncButton,
             launchButton_,
             settingsButton
         }) {
        button->setMinimumHeight(40);
        buttons->addWidget(button);
    }

    root->addLayout(buttons);

    // Launcher path

    auto* pathCaption = new QLabel(
        QStringLiteral("LAUNCHER"),
        central
    );
    pathCaption->setStyleSheet(
        "color: #929DA8;"
        "font-size: 10px;"
        "letter-spacing: 2px;"
    );

    launcherPath_ = new QLabel(central);
    launcherPath_->setWordWrap(true);
    launcherPath_->setTextInteractionFlags(
        Qt::TextSelectableByMouse
    );
    launcherPath_->setStyleSheet("color: #929DA8;");

    root->addWidget(pathCaption);
    root->addWidget(launcherPath_);

    // Developer mode: event log

    logCaption_ = new QLabel(
        QStringLiteral("EVENT LOG"),
        central
    );
    logCaption_->setStyleSheet(
        "color: #929DA8;"
        "font-size: 10px;"
        "letter-spacing: 2px;"
    );
    root->addWidget(logCaption_);

    eventLog_ = new QPlainTextEdit(central);
    eventLog_->setReadOnly(true);
    eventLog_->setMaximumBlockCount(300);
    eventLog_->setPlaceholderText(
        QStringLiteral("События появятся здесь...")
    );
    eventLog_->setMinimumHeight(100);
    eventLog_->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );

   root->addWidget(eventLog_, 1);

   // Свободное пространство в User Mode остаётся внизу.
   // В Dev Mode его будет занимать журнал событий.
   root->addStretch(0);

setCentralWidget(central);
    // Application stylesheet

    setStyleSheet(QStringLiteral(R"(
        QMainWindow {
            background-color: #15191E;
        }

        QWidget {
            background-color: transparent;
            color: #F0F3F6;
            font-family: sans-serif;
            font-size: 12px;
        }

        QWidget#centralWidget {
            background-color: transparent;
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

    // Signals

    connect(
        modeBox_,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int) {
            updateDisplayMode();
        }
    );

    connect(
        checkButton,
        &QPushButton::clicked,
        this,
        &MainWindow::validateClient
    );

    connect(
        syncButton,
        &QPushButton::clicked,
        this,
        &MainWindow::synchronizeClient
    );

    connect(
        launchButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::launchGame
    );

    connect(
        settingsButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openSettings
    );

    // Load settings

    const QString savedPath = AppSettings::launcherPath();

    launcherPath_->setText(
        savedPath.isEmpty()
            ? QStringLiteral("Лаунчер не выбран")
            : savedPath
    );

    updateLaunchButton();

    appendLog(QStringLiteral("Валидатор запущен."));
    updateDisplayMode();
}

// Client validation

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

// Client synchronization

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
            QStringLiteral(
                "Синхронизация отменена: сервер недоступен "
                "или манифест некорректен."
            )
        );

        return;
    }

    if (result.status == ValidationStatus::Sync) {
        applyResult(result);
        appendLog(QStringLiteral("Изменения не требуются."));
        return;
    }

    appendLog(
        QStringLiteral(
            "DEBUG: имитация синхронизации. "
            "Файлы на диске не изменялись."
        )
    );

    auto simulated = result;
    simulated.status = ValidationStatus::Sync;
    simulated.message = QStringLiteral(
        "Тестовая синхронизация завершена. "
        "Реальные файлы не изменялись."
    );
    simulated.details.clear();

    applyResult(simulated);

    appendLog(
        QStringLiteral("DEBUG: клиент условно синхронизирован.")
    );
}

// Launcher

void MainWindow::launchGame()
{
    const QString path = AppSettings::launcherPath();

    if (path.isEmpty()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Лаунчер не настроен"),
            QStringLiteral(
                "Сначала укажите путь к лаунчеру "
                "в настройках."
            )
        );
        return;
    }

    const QFileInfo file(path);

    if (!file.exists() || !file.isFile()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Файл не найден"),
            QStringLiteral(
                "Указанный исполняемый файл не существует."
            )
        );

        appendLog(
            QStringLiteral("Ошибка: лаунчер не найден: %1")
                .arg(path)
        );
        return;
    }

    if (!file.isExecutable()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Нет прав на запуск"),
            QStringLiteral(
                "Файл не помечен как исполняемый."
            )
        );

        appendLog(
            QStringLiteral("Ошибка: файл не исполняемый: %1")
                .arg(path)
        );
        return;
    }

    if (lastResult_.status != ValidationStatus::Sync &&
        lastResult_.status != ValidationStatus::ExtraMods) {
        QMessageBox::warning(
            this,
            QStringLiteral("Проверка не пройдена"),
            QStringLiteral(
                "Сначала проверьте клиент и устраните "
                "обязательные ошибки синхронизации."
            )
        );
        return;
    }

    QProcess process;
    process.setProgram(path);
    process.setArguments({});

    QString error;

    if (!process.startDetached()) {
        error = process.errorString();

        QMessageBox::critical(
            this,
            QStringLiteral("Ошибка запуска"),
            QStringLiteral(
                "Не удалось запустить лаунчер:\n%1"
            ).arg(error)
        );

        appendLog(
            QStringLiteral("Ошибка запуска: %1").arg(error)
        );
        return;
    }

    appendLog(
        QStringLiteral("Лаунчер запущен: %1").arg(path)
    );

    close();
}

// Settings

void MainWindow::openSettings()
{
    SettingsWindow dialog(this);

    const int result = dialog.exec();

    if (result != QDialog::Accepted) {
        return;
    }

    const QString path = AppSettings::launcherPath();

    launcherPath_->setText(
        path.isEmpty()
            ? QStringLiteral("Лаунчер не выбран")
            : path
    );

    appendLog(QStringLiteral("Настройки сохранены."));
}

// Apply validation result

void MainWindow::applyResult(const ValidationResult& result)
{
    lastResult_ = result;

    statusTitle_->setText(statusName(result.status));

    statusTitle_->setStyleSheet(
        QStringLiteral(
            "font-size: 15px;"
            "font-weight: bold;"
            "color: %1;"
        ).arg(statusColor(result.status))
    );

    statusIndicator_->setStyleSheet(
        QStringLiteral(
            "background-color: %1;"
            "border-radius: 5px;"
        ).arg(statusColor(result.status))
    );

    statusDescription_->setText(result.message);

    if (result.serverOnline) {
        serverStatus_->setText(QStringLiteral("Online"));
        serverStatus_->setStyleSheet("color: #55D98A;");

        if (result.onlinePlayers >= 0 &&
            result.maxPlayers >= 0) {
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

// Event log

void MainWindow::appendLog(const QString& message)
{
    const QString time =
        QDateTime::currentDateTime().toString(
            QStringLiteral("HH:mm:ss")
        );

    eventLog_->appendPlainText(
        QStringLiteral("[%1] %2").arg(time, message)
    );
}

// Launch button state

void MainWindow::updateLaunchButton()
{
    const bool canLaunch =
        lastResult_.status == ValidationStatus::Sync ||
        lastResult_.status == ValidationStatus::ExtraMods;

    launchButton_->setEnabled(canLaunch);
}

// Display mode

void MainWindow::updateDisplayMode()
{
    const bool devMode = modeBox_->currentIndex() == 1;

    debugLabel_->setVisible(devMode);
    scenarioBox_->setVisible(devMode);
    logCaption_->setVisible(devMode);
    eventLog_->setVisible(devMode);

    if (auto* layout =
            qobject_cast<QVBoxLayout*>(centralWidget()->layout())) {
        const int logIndex = layout->indexOf(eventLog_);
        const int spacerIndex = layout->count() - 1;

        // Dev Mode: свободная высота достаётся журналу.
        // User Mode: свободная высота уходит в нижний отступ.
        if (logIndex >= 0) {
            layout->setStretch(logIndex, devMode ? 1 : 0);
        }

        if (spacerIndex >= 0) {
            layout->setStretch(spacerIndex, devMode ? 0 : 1);
        }

        layout->invalidate();
        layout->activate();
    }
}

