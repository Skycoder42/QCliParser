HEADERS += \
	$$PWD/qcliparser.h \
	$$PWD/qclinode.h

SOURCES += \
	$$PWD/qcliparser.cpp \
	$$PWD/qclinode.cpp

win32: LIBS += -luser32

INCLUDEPATH += $$PWD
