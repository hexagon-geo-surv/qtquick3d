# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qquick3dinput
    SOURCES
        ../shared/util.cpp ../shared/util.h
        tst_input.cpp
    INCLUDE_DIRECTORIES
        ../shared
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qquick3dinput CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_qquick3dinput CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)

