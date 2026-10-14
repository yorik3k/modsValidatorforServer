
#pragma once

#include "core/ValidationState.hpp"

class ServerClient final {
public:
    [[nodiscard]]
    ValidationResult fetchManifest(DebugScenario scenario) const
    {
        ValidationResult result;
        result.serverOnline = true;
        result.onlinePlayers = 12;
        result.maxPlayers = 32;

        switch (scenario) {
        case DebugScenario::AllValid:
            result.status = ValidationStatus::Sync;
            result.message =
                QStringLiteral("Все обязательные файлы совпадают.");
            break;

        case DebugScenario::ExtraMods:
            result.status = ValidationStatus::ExtraMods;
            result.message =
                QStringLiteral("Обнаружены лишние локальные моды.");
            result.details = {
                QStringLiteral("example-client-mod.jar"),
                QStringLiteral("example-minimap.jar")
            };
            break;

        case DebugScenario::MissingMods:
            result.status = ValidationStatus::NotSync;
            result.message =
                QStringLiteral("Отсутствуют обязательные моды.");
            result.details = {
                QStringLiteral("required-mod-a.jar"),
                QStringLiteral("required-mod-b.jar")
            };
            break;

        case DebugScenario::HashMismatch:
            result.status = ValidationStatus::NotSync;
            result.message =
                QStringLiteral("Хеш файла не совпадает.");
            result.details = {
                QStringLiteral("example-mod.jar — SHA-256 mismatch")
            };
            break;

        case DebugScenario::ServerOffline:
            result.status = ValidationStatus::ServerOffline;
            result.message =
                QStringLiteral("Сервер недоступен.");
            result.serverOnline = false;
            result.onlinePlayers = -1;
            result.maxPlayers = -1;
            break;

        case DebugScenario::InvalidManifest:
            result.status = ValidationStatus::ManifestError;
            result.message =
                QStringLiteral("Некорректный манифест сервера.");
            break;
        }

        return result;
    }
};

