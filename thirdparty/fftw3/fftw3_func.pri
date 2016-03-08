include(fftw3_config.pri)

defineReplace(fftw3Core) {
    CONFIG += c++11 noqt
    CONFIG -= qt

    # include paths (complete library)
    INCLUDEPATH += $$FFTW_CONFIG_PATH
    INCLUDEPATH += $$FFTW_ROOT/api
    INCLUDEPATH += $$FFTW_ROOT/dft
    INCLUDEPATH += $$FFTW_ROOT/dft/scalar
    INCLUDEPATH += $$FFTW_ROOT/dft/simd
    INCLUDEPATH += $$FFTW_ROOT/kernel
    INCLUDEPATH += $$FFTW_ROOT/rdft
    INCLUDEPATH += $$FFTW_ROOT/rdft/scalar
    INCLUDEPATH += $$FFTW_ROOT/rdft/simd
    INCLUDEPATH += $$FFTW_ROOT/reodft
    INCLUDEPATH += $$FFTW_ROOT/simd-support

    LIBS += -L$$FFTW_SUBPART_OUT

    contains(FFTW_LIB_MODE_CONFIG, dll) {
        DEFINES += "DLL_EXPORT=1"
    }

    export(CONFIG)
    export(INCLUDEPATH)
    export(LIBS)
    export(DEFINES)

    return(true)
}

defineReplace(fftw3SubPart) {
    $$fftw3Core()

    TEMPLATE = lib
    CONFIG += staticlib c++11 noqt
    CONFIG -= qt
    DESTDIR = $$FFTW_SUBPART_OUT

    contains(FFTW_LIB_MODE_CONFIG, dll) {
        DEFINES += "DLL_EXPORT=1"
    }

    # include paths (complete library)
    INCLUDEPATH += $$FFTW_CONFIG_PATH
    INCLUDEPATH += $$FFTW_ROOT/api
    INCLUDEPATH += $$FFTW_ROOT/dft
    INCLUDEPATH += $$FFTW_ROOT/dft/scalar
    INCLUDEPATH += $$FFTW_ROOT/dft/simd
    INCLUDEPATH += $$FFTW_ROOT/kernel
    INCLUDEPATH += $$FFTW_ROOT/rdft
    INCLUDEPATH += $$FFTW_ROOT/rdft/scalar
    INCLUDEPATH += $$FFTW_ROOT/rdft/simd
    INCLUDEPATH += $$FFTW_ROOT/reodft
    INCLUDEPATH += $$FFTW_ROOT/simd-support

    export(TEMPLATE)
    export(CONFIG)
    export(DESTDIR)
    export(INCLUDEPATH)
    export(LIBS)
    export(DEFINES)

    return(true)
}

SUBDIRS += \
    $$PWD/fftw3/dft/scalar/scalar/scalar.pro
