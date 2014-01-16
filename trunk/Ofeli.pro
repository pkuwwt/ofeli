TEMPLATE = app

VERSION = 1.0.8

TARGET = Ofeli

DEFINES += "HAS_8_CONNEXITY=false" \
           "IS_COLUM_WISE=false"

QT += widgets multimedia multimediawidgets
!isEmpty(QT.printsupport.name): QT += printsupport

simulator: warning(This example might not fully work on Simulator platform)

INCLUDEPATH += \
    $$PWD/sources/Qt_GUI/windows \
    $$PWD/sources/Qt_GUI/widgets \
    $$PWD/sources/Qt_GUI/interface \
    $$PWD/sources/image_processing/algorithms \
    $$PWD/sources/image_processing/data_structures

HEADERS += \
    $$PWD/sources/Qt_GUI/windows/*.hpp \
    $$PWD/sources/Qt_GUI/widgets/*.hpp \
    $$PWD/sources/Qt_GUI/interface/*.hpp \
    $$PWD/sources/image_processing/algorithms/*.hpp \
    $$PWD/sources/image_processing/data_structures/*.hpp

SOURCES += $$PWD/sources/main.cpp \
    $$PWD/sources/Qt_GUI/windows/*.cpp \
    $$PWD/sources/Qt_GUI/widgets/*.cpp \
    $$PWD/sources/Qt_GUI/interface/*.cpp \
    $$PWD/sources/image_processing/algorithms/*.cpp \
    $$PWD/sources/image_processing/data_structures/*.tpp

TRANSLATIONS = $$PWD/resources/$${TARGET}_fr.ts
CODECFORTR = UTF-8
CODECFORSRC = UTF-8

RESOURCES = $$PWD/resources/$${TARGET}.qrc

macx : ICON = $$PWD/$${TARGET}.icns
win32 : RC_FILE = $$PWD/$${TARGET}.rc

MOC_DIR = $$OUT_PWD/build/temp
OBJECTS_DIR = $$OUT_PWD/build/temp
RCC_DIR = $$OUT_PWD/build/temp
DESTDIR = $$OUT_PWD/build/bin

macx : QMAKE_POST_LINK += macdeployqt $$OUT_PWD/build/bin/$${TARGET}.app

# this project uses key words nullptr, override, auto
# and headers <random>, <chrono>, <functional> of C++11 standard
CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE += -O3
