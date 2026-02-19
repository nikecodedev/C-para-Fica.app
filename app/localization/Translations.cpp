#include "Translations.hpp"

namespace app {
namespace localization {

std::unordered_map<std::string, std::string> translationsEn() {
    return {
        {"home", "Home"},
        {"settings", "Settings"},
        {"tracking", "Tracking"},
        {"subscription", "Subscription"},
        {"start_tracking", "Start Tracking"},
        {"stop_tracking", "Stop Tracking"},
        {"tracking_active", "Tracking Active"},
        {"tracking_paused", "Tracking Paused"},
        {"speed", "Speed"},
        {"distance", "Distance"},
        {"elevation", "Elevation"},
        {"metric", "Metric"},
        {"imperial", "Imperial"},
        {"subscribe", "Subscribe"},
        {"subscribed", "Subscribed"},
        {"upgrade_for_100hz", "Upgrade for 100Hz precision tracking"},
        {"gps_only_mode", "GPS-only mode (1Hz)"},
        {"language", "Language"},
        {"units", "Units"},
        {"portuguese", "Portuguese"},
        {"english", "English"},
        {"ok", "OK"},
        {"cancel", "Cancel"},
        {"save", "Save"},
        {"error", "Error"},
        {"loading", "Loading..."},
    };
}

std::unordered_map<std::string, std::string> translationsPt() {
    return {
        {"home", "Início"},
        {"settings", "Configurações"},
        {"tracking", "Rastreamento"},
        {"subscription", "Assinatura"},
        {"start_tracking", "Iniciar Rastreamento"},
        {"stop_tracking", "Parar Rastreamento"},
        {"tracking_active", "Rastreamento Ativo"},
        {"tracking_paused", "Rastreamento Pausado"},
        {"speed", "Velocidade"},
        {"distance", "Distância"},
        {"elevation", "Elevação"},
        {"metric", "Métrico"},
        {"imperial", "Imperial"},
        {"subscribe", "Assinar"},
        {"subscribed", "Assinado"},
        {"upgrade_for_100hz", "Atualize para rastreamento de precisão 100Hz"},
        {"gps_only_mode", "Modo apenas GPS (1Hz)"},
        {"language", "Idioma"},
        {"units", "Unidades"},
        {"portuguese", "Português"},
        {"english", "Inglês"},
        {"ok", "OK"},
        {"cancel", "Cancelar"},
        {"save", "Salvar"},
        {"error", "Erro"},
        {"loading", "Carregando..."},
    };
}

}  // namespace localization
}  // namespace app
