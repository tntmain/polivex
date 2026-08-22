#pragma once

#include <optional>
#include <vector>

#include "core/rectangle_entity.h"

namespace polivex::app {

class SelectionSet {
public:
    void clear() noexcept;
    void set_single(std::optional<polivex::core::EntityId> entity_id) noexcept;
    void set_many(const std::vector<polivex::core::EntityId>& entity_ids) noexcept;
    void toggle(polivex::core::EntityId entity_id) noexcept;

    [[nodiscard]] std::optional<polivex::core::EntityId> primary() const noexcept;
    [[nodiscard]] const std::vector<polivex::core::EntityId>& ids() const noexcept;

private:
    void normalize() noexcept;

    std::vector<polivex::core::EntityId> ids_;
};

}  // namespace polivex::app
