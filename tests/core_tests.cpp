#include <cstdlib>
#include <iostream>

#include "app/application_session.h"

int main()
{
    polivex::app::ApplicationSession session;

    if (session.active_document().name() != "Untitled") {
        std::cerr << "expected new session document to be Untitled\n";
        return EXIT_FAILURE;
    }

    session.active_document().rename("Bracket");

    if (!session.active_document().is_dirty()) {
        std::cerr << "expected renamed document to be dirty\n";
        return EXIT_FAILURE;
    }

    session.create_new_document();

    if (session.active_document().name() != "Untitled") {
        std::cerr << "expected reset document to be Untitled\n";
        return EXIT_FAILURE;
    }

    if (session.active_document().is_dirty()) {
        std::cerr << "expected new document to be clean\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
