#include "config.h"

namespace cfg {

void Config::Update() {
    // Передать всем объектам факт обновления конфигурации.
    const auto callbacks = cbs_;
    for (auto const& cb : callbacks) {
        if (cb != nullptr) {
            cb();
        }
    }
}

} // namespace cfg
