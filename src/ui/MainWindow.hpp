
#pragma once

#include "core/ValidationState.hpp"
#include "services/ServerClient.hpp"

#include <QMainWindow>

class QLabel;
class QComboBox;
class QPlainTextEdit;
class QPushButton;
class QWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void validateClient();

    void synchronizeClient();
    void launchGame();
    void openSettings();

    void applyResult(const ValidationResult& result);
    void appendLog(const QString& message);
    void updateLaunchButton();

    void updateDisplayMode();


    ServerClient serverClient_;
    ValidationResult lastResult_;

    QLabel* statusTitle_ = nullptr;
    QLabel* statusDescription_ = nullptr;
    QLabel* serverStatus_ = nullptr;
    QLabel* playerCount_ = nullptr;
    QLabel* launcherPath_ = nullptr;

    QComboBox* scenarioBox_ = nullptr;
    QPlainTextEdit* eventLog_ = nullptr;

    QPushButton* launchButton_ = nullptr;

    QLabel* statusIndicator_ = nullptr;
    QComboBox* modeBox_ = nullptr;
    QWidget* devPanel_ = nullptr;
    QLabel* logCaption_ = nullptr;
    QLabel* scenarioCaption_ = nullptr;

    QLabel* debugLabel_ = nullptr;

};

