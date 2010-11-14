CONFIG += debug_and_release
CONFIG *= dll
CONFIG -= app_bundle
CONFIG -= rtti exceptions

CONFIG(debug, debug|release) {
    MOC_DIR     = tmp/debug/moc
    OBJECTS_DIR = tmp/debug/obj
    UI_DIR      = tmp/debug/ui
    RCC_DIR     = tmp/debug/rcc

    unix {
        TARGET = $$join(TARGET,,,_debug)
    } else {
        TARGET = $$join(TARGET,,,d)
    }
} else {
    MOC_DIR     = tmp/release/moc
    OBJECTS_DIR = tmp/release/obj
    UI_DIR      = tmp/release/ui
    RCC_DIR     = tmp/release/rcc
}
