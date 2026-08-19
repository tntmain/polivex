#include <QApplication>
#include <QIcon>
#include <QSettings>
#include <QTranslator>

#include "app/application_session.h"
#include "ui/main_window.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("Polivex");
    application.setOrganizationName("Polivex");
    application.setWindowIcon(QIcon(":/assets/polivex_icon.svg"));

    QTranslator translator;
    const auto apply_language = [&application, &translator](const QString& locale) {
        application.removeTranslator(&translator);

        if (!translator.load(QString(":/i18n/polivex_%1.qm").arg(locale))) {
            return false;
        }

        application.installTranslator(&translator);
        QSettings().setValue("interface/language", locale);
        return true;
    };

    const auto saved_language = QSettings().value("interface/language", "en").toString();
    const auto initial_language = apply_language(saved_language) ? saved_language : QString("en");

    polivex::app::ApplicationSession session;
    polivex::ui::MainWindow window(session);
    window.setWindowIcon(application.windowIcon());
    window.set_current_language(initial_language);

    QObject::connect(&window, &polivex::ui::MainWindow::language_requested, &application,
        [&apply_language, &window](const QString& locale) {
            if (apply_language(locale)) {
                window.set_current_language(locale);
            }
        });

    window.show();

    return application.exec();
}
