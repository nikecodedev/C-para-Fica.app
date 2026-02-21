#include "../license/LicenseKeyWithValidity.hpp"
#include <iostream>
#include <cassert>

int main() {
    app::license::LicenseKeyWithValidity license;

    assert(!license.hasKey());
    assert(!license.isValid(1735689600.0));  // 2025-01-01

    license.setKey("FICA-TEST-1738368000");  // end 2025-02-01
    assert(license.hasKey());
    assert(license.isValid(1735689600.0));   // before end
    assert(license.isValid(1738367999.0));   // just before end
    assert(!license.isValid(1738368000.0));  // at end
    assert(!license.isValid(1738368001.0));  // after end

    license.clear();
    assert(!license.hasKey());
    assert(!license.isValid(1735689600.0));

    // Invalid format: no expiry parsed, hasKey but validityEndUnix_=0 → isValid true (dev fallback)
    license.setKey("FICA-TEST-invalid");
    assert(license.hasKey());
    assert(license.isValid(1735689600.0));

    std::cout << "LicenseKeyWithValidity test OK" << std::endl;
    return 0;
}
