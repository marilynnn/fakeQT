TEMPLATE = lib
TARGET = CppEditor
DEFINES += CPPEDITOR_LIBRARY
include(../../libs/utils/utils.pri)
include(../../qworkbenchplugin.pri)
include(cppeditor_dependencies.pri)
HEADERS += cppplugin.h \
    cppeditor.h \
    cppeditoractionhandler.h \
    cpphighlighter.h \
    cppfilewizard.h \
    cppeditorconstants.h \
    cppeditorenums.h \
    cppeditor_global.h \
    cppclasswizard.h
SOURCES += cppplugin.cpp \
    cppeditoractionhandler.cpp \
    cppeditor.cpp \
    cpphighlighter.cpp \
    cppfilewizard.cpp \
    cppclasswizard.cpp
RESOURCES += cppeditor.qrc
