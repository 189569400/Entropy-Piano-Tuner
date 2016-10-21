HEADERS += \
    $$PWD/winrtplatformtools.h \
    $$PWD/winrtmidiadapterinterface.h

SOURCES += \
    $$PWD/winrtplatformtools.cpp \
    $$PWD/winrtmidiadapterinterface.cpp

$$depends_winrtbridge()


# need to copy the midi plugin manually on win rt
CONFIG(debug, debug|release) {
    PLUGIN_DIR = $$shadowed($$EPT_TARGET_DIR)
    PLUGIN_FILE=qtwinrt_midid.dll
}
else {
    PLUGIN_DIR = $$shadowed($$EPT_TARGET_DIR)
    PLUGIN_FILE=qtwinrt_midi.dll
}

message($$PLUGIN_DIR)
PLUGIN_DIR ~= s,/,\\,g

QMAKE_POST_LINK += \
    $$sprintf($$QMAKE_MKDIR_CMD, $$quote($$PLUGIN_DIR\\midi)) $$escape_expand(\\n\\t) \
    $$QMAKE_COPY $$quote($$(QTDIR)\\plugins\\midi\\$$PLUGIN_FILE) $$quote($$PLUGIN_DIR\\midi\\$$PLUGIN_FILE) $$escape_expand(\\n\\t)
