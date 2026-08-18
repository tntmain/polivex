#include <QApplication>

#include "app/application_session.h"
#include "ui/main_window.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("Polivex");
    application.setOrganizationName("Polivex");

    polivex::app::ApplicationSession session;
    polivex::ui::MainWindow window(session);
    window.show();

    return application.exec();
}
