#include "core/project_document.h"

#include <utility>

namespace polivex::core {

ProjectDocument::ProjectDocument(std::string name)
    : name_(std::move(name))
{
}

const std::string& ProjectDocument::name() const noexcept
{
    return name_;
}

void ProjectDocument::rename(std::string new_name)
{
    name_ = std::move(new_name);
    dirty_ = true;
}

bool ProjectDocument::is_dirty() const noexcept
{
    return dirty_;
}

void ProjectDocument::mark_dirty(bool dirty) noexcept
{
    dirty_ = dirty;
}

}  // namespace polivex::core
