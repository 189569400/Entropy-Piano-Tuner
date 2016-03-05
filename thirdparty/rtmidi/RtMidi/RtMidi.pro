TEMPLATE = lib

CONFIG += c++11
CONFIG += staticlib

SOURCES = RtMidi.cpp
HEADERS = RtMidi.h

include(../../../entropypianotuner_config.pri)

DESTDIR = $$EPT_THIRDPARTY_OUT_DIR

linux:!android {
    # chose RtMidi backend
    DEFINES += __LINUX_ALSA__
}
