#pragma once

#include "core/project_document.h"

namespace polivex::app {

class ApplicationSession {
public:
    ApplicationSession();

    [[nodiscard]] const polivex::core::ProjectDocument& active_document() const noexcept;
    [[nodiscard]] polivex::core::ProjectDocument& active_document() noexcept;

    void create_new_document();

private:
    polivex::core::ProjectDocument active_document_;
};

}  // namespace polivex::app
