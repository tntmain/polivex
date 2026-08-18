#pragma once

#include <string>

namespace polivex::core {

class ProjectDocument {
public:
    explicit ProjectDocument(std::string name = "Untitled");

    [[nodiscard]] const std::string& name() const noexcept;
    void rename(std::string new_name);

    [[nodiscard]] bool is_dirty() const noexcept;
    void mark_dirty(bool dirty = true) noexcept;

private:
    std::string name_;
    bool dirty_ = false;
};

}  // namespace polivex::core
