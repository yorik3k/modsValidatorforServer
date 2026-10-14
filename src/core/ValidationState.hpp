
#pragma once

#include <QString>
#include <QStringList>

enum class ValidationStatus {
    NotChecked,
    Sync,
    ExtraMods,
    NotSync,
    ServerOffline,
    ManifestError
};

enum class DebugScenario {
    AllValid,
    ExtraMods,
    MissingMods,
    HashMismatch,
    ServerOffline,
    InvalidManifest
};

struct ValidationResult {
    ValidationStatus status = ValidationStatus::NotChecked;
    QString message = QStringLiteral("Проверка ещё не выполнялась.");
    QStringList details;
    bool serverOnline = false;
    int onlinePlayers = -1;
    int maxPlayers = -1;
};

