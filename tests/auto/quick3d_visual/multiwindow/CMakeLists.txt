# Generated from multiwindow.pro.

#####################################################################
## qquick3dmultiwindow Test:
#####################################################################

qt_internal_add_test(tst_qquick3dmultiwindow
    SOURCES
        ../shared/util.cpp ../shared/util.h
        tst_multiwindow.cpp
    INCLUDE_DIRECTORIES
        ../shared
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
)

#### Keys ignored in scope 1:.:.:multiwindow.pro:<TRUE>:
# OTHER_FILES = "data/cube.qml"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qquick3dmultiwindow CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_qquick3dmultiwindow CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
