#include "../localization/Localizer.hpp"
#include "../localization/LocalizationKeys.hpp"
#include <iostream>

int main() {
    app::localization::Localizer localizer;

    localizer.setLocale("en");
    std::string enStart = localizer.t(app::localization::Keys::START_TRACKING);
    std::cout << "EN: " << enStart << std::endl;

    localizer.setLocaleFromSystem("pt-BR");
    std::string ptStart = localizer.t(app::localization::Keys::START_TRACKING);
    std::cout << "PT: " << ptStart << std::endl;

    bool ok = (enStart == "Start Tracking" && ptStart == "Iniciar Rastreamento");
    std::cout << (ok ? "Localization OK" : "FAIL") << std::endl;

    return ok ? 0 : 1;
}
