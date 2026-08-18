#include "app/application_session.h"

namespace polivex::app {

ApplicationSession::ApplicationSession()
    : active_document_("Untitled")
{
}

const polivex::core::ProjectDocument& ApplicationSession::active_document() const noexcept
{
    return active_document_;
}

polivex::core::ProjectDocument& ApplicationSession::active_document() noexcept
{
    return active_document_;
}

void ApplicationSession::create_new_document()
{
    active_document_ = polivex::core::ProjectDocument("Untitled");
}

}  // namespace polivex::app
