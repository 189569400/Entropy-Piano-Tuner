# Include guard
isEmpty(EPT_CONFIG_INCLUDED):EPT_CONFIG_INCLUDED=true
else:return()

#------------------------------------------------
# Global config

CONFIG += c++11

# include example algorithm by default in debug builts

# on mobile platforms use static algorithm build
android|ios|winrt|winphone: EPT_CONFIG += no_shared_algorithms static_algorithms
else:                       EPT_CONFIG += shared_algorithms no_static_algorithms

# on linux build static core
linux:!android: EPT_CONFIG += static_core

#------------------------------------------------
# Path defines

EPT_ROOT_DIR = $$PWD

EPT_BASE_DIR = $$EPT_ROOT_DIR
EPT_CORE_DIR = $$EPT_BASE_DIR/core
EPT_APPSTORE_DIR = $$EPT_ROOT_DIR/appstore
EPT_ALGORITHMS_DIR = $$EPT_ROOT_DIR/algorithms
EPT_THIRDPARTY_DIR = $$EPT_ROOT_DIR/thirdparty
EPT_DEPENDENCIES_DIR = $$EPT_ROOT_DIR/dependencies
EPT_TRANSLATIONS_DIR = $$EPT_BASE_DIR/translations
EPT_TUTORIAL_DIR = $$EPT_ROOT_DIR/tutorial

EPT_THIRDPARTY_OUT_DIR = $$shadowed($$PWD)/lib
EPT_CORE_OUT_DIR = $$shadowed($$PWD)
EPT_ALGORITHMS_OUT_DIR = $$shadowed($$PWD)/algorithms

EPT_ANDROID_PACKAGE_SOURCE_DIR = $$EPT_ROOT_DIR/platforms/android

# Install paths if not defines
isEmpty(EPT_INSTALL_BIN_DIR):EPT_INSTALL_BIN_DIR=bin
isEmpty(EPT_INSTALL_DATA_DIR):EPT_INSTALL_DATA_DIR=shared
isEmpty(EPT_INSTALL_LIB_DIR):EPT_INSTALL_LIB_DIR=lib

#------------------------------------------------
# third party modules
CONFIG += qwt fftw3 getmemorysize

# add libuv when shared algorithms are enabled
contains(EPT_CONFIG, shared_algorithms) {
    CONFIG += libuv
}

winrt|winphone {
    CONFIG += timesupport
}
else:win32 {
    CONFIG += timesupport rtmidi dirent
}

linux:!android {
    CONFIG += getmemorysize rtmidi
}

android {
    CONFIG += getmemorysize
}

macx {
    CONFIG += getmemorysize rtmidi
}

# run tests
load(configure)

# use modules that are installed on the system
qtCompileTest(qwt_exists):  EPT_THIRDPARTY_CONFIG+=system_qwt

packagesExist(qwt):       EPT_THIRDPARTY_CONFIG+=system_qwt
packagesExist(fftw3):     EPT_THIRDPARTY_CONFIG+=system_fftw3
packagesExist(libuv):     EPT_THIRDPARTY_CONFIG+=system_libuv

#--------------------------------------------------
# global settings

linux-g++*:!android {
    QMAKE_CXXFLAGS_DEBUG += -D_GLIBCXX_DEBUG
}


contains(EPT_CONFIG, no_shared_algorithms):DEFINES+="EPT_NO_SHARED_ALGORITHMS=1" "EPT_STATIC_ALGORITHMS=1"
contains(EPT_CONFIG, shared_algorithms):DEFINES+="EPT_SHARED_ALGORITHMS=1"

LIBS += -L$$EPT_THIRDPARTY_OUT_DIR
