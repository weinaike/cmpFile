TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    jsoncpp.cpp

HEADERS += \
    json/json-forwards.h \
    json/json.h

DISTFILES += \
    wordsmap.cfg
