add_library(polivex_core STATIC
    core/project_document.cpp
    core/project_document.h
    core/point_2d.h
    core/rectangle_2d.h
    core/rectangle_entity.h
    core/sketch_plane.h
    core/vector_style.h
)

target_compile_features(polivex_core PUBLIC cxx_std_20)
target_include_directories(polivex_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(polivex_app STATIC
    app/application_session.cpp
    app/application_session.h
    app/workspace.h
)

target_compile_features(polivex_app PUBLIC cxx_std_20)
target_include_directories(polivex_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(polivex_app PUBLIC polivex_core)

add_library(polivex_ui STATIC
    ui/main_window.cpp
    ui/main_window.h
    ui/viewport_widget.cpp
    ui/viewport_widget.h
)

target_compile_features(polivex_ui PUBLIC cxx_std_20)
target_include_directories(polivex_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(polivex_ui PUBLIC Qt6::Widgets polivex_app)

qt_add_executable(polivex
    main.cpp
)

target_link_libraries(polivex PRIVATE Qt6::Widgets polivex_ui polivex_app)

qt_add_translations(polivex
    TS_FILES
        "${PROJECT_SOURCE_DIR}/resources/i18n/polivex_en.ts"
        "${PROJECT_SOURCE_DIR}/resources/i18n/polivex_ru.ts"
    RESOURCE_PREFIX "/i18n"
    SOURCES
        main.cpp
        ui/main_window.cpp
        ui/main_window.h
        ui/viewport_widget.cpp
        ui/viewport_widget.h
)
