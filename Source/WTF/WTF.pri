# -------------------------------------------------------------------
# This file contains shared rules used both when building WTF itself
# and for targets that depend in some way on WTF.
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

# All external modules should include WTF headers by prefixing with "wtf" (#include <wtf/some/thing.h>).
INCLUDEPATH += $$PWD

#equals(QT_ARCH, s390)|equals(QT_ARCH, arm)|equals(QT_ARCH, mips)|equals(QT_ARCH, i386)|equals(QT_ARCH, i686)|equals(QT_ARCH, x86_64) {
#    message("WTF workaround for QtWebkit: do not build with -g, but with -g1")
#    QMAKE_CXXFLAGS -= -g
#    QMAKE_CXXFLAGS += -g1
#}

mac {
    # Mac OS does ship libicu but not the associated header files.
    # Therefore WebKit provides adequate header files.
    INCLUDEPATH = $${ROOT_WEBKIT_DIR}/Source/WTF/icu $$INCLUDEPATH
    LIBS += -licucore
} else:!use?(wchar_unicode): {
    win32:!mingw {
        CONFIG(static, static|shared) {
            CONFIG(debug, debug|release) {
                LIBS += -lsicuind -lsicuucd -lsicudtd
            } else {
                LIBS += -lsicuin -lsicuuc -lsicudt
            }
        } else {
            LIBS += -licuin -licuuc -licudt
        }
    }
    else:!contains(QT_CONFIG,no-pkg-config):packagesExist("icu-i18n"): PKGCONFIG *= icu-i18n
    else:android: LIBS += -licui18n -licuuc
    else: LIBS += -licui18n -licuuc -licudata
}

use?(GLIB) {
    PKGCONFIG *= glib-2.0 gio-2.0
}

win32-* {
    LIBS += -lwinmm
    LIBS += -lgdi32
}

qnx {
    # required for timegm
    LIBS += -lnbutil
}

mac {
    LIBS += -framework AppKit
}

# MSVC is lacking stdint.h as well as inttypes.h.
win32-msvc2005|win32-msvc2008|win32-msvc2010|win32-msvc2012|win32-msvc2013|win32-icc|wince*: INCLUDEPATH += $$ROOT_WEBKIT_DIR/Source/JavaScriptCore/os-win32
