qt_internal_add_manual_test(dynamicenvmap
    GUI
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

set(dynamicenvmap_resource_files
    "main.qml"
    "InvertedSphere.mesh"
    "00455_OpenfootageNET_field_low_resized.jpg"
)

qt6_add_resources(dynamicenvmap "dynamicenvmap"
    PREFIX
        "/"
    FILES
        ${dynamicenvmap_resource_files}
)
