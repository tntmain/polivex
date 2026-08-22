#include "app/selection_set.h"

#include <algorithm>

namespace polivex::app {

void SelectionSet::clear() noexcept
{
    ids_.clear();
}

void SelectionSet::set_single(std::optional<polivex::core::EntityId> entity_id) noexcept
{
    ids_.clear();
    if (entity_id.has_value()) {
        ids_.push_back(*entity_id);
    }
}

void SelectionSet::set_many(const std::vector<polivex::core::EntityId>& entity_ids) noexcept
{
    ids_ = entity_ids;
    normalize();
}

void SelectionSet::toggle(polivex::core::EntityId entity_id) noexcept
{
    const auto iterator = std::find(ids_.begin(), ids_.end(), entity_id);
    if (iterator != ids_.end()) {
        ids_.erase(iterator);
        return;
    }

    ids_.insert(ids_.begin(), entity_id);
}

std::optional<polivex::core::EntityId> SelectionSet::primary() const noexcept
{
    if (ids_.empty()) {
        return std::nullopt;
    }

    return ids_.front();
}

const std::vector<polivex::core::EntityId>& SelectionSet::ids() const noexcept
{
    return ids_;
}

void SelectionSet::normalize() noexcept
{
    std::vector<polivex::core::EntityId> normalized;
    normalized.reserve(ids_.size());
    for (const auto id : ids_) {
        if (std::find(normalized.begin(), normalized.end(), id) == normalized.end()) {
            normalized.push_back(id);
        }
    }

    ids_ = std::move(normalized);
}

}  // namespace polivex::app
