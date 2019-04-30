HEADERS += \
	$$PWD/qcliparser.h \
	$$PWD/qclinode.h \
	$$PWD/qmultitree.h

SOURCES += \
	$$PWD/qcliparser.cpp \
	$$PWD/qclinode.cpp

win32: LIBS += -luser32

INCLUDEPATH += $$PWD

QDEP_PACKAGE_EXPORTS += Q_CLI_PARSER_EXPORT
!qdep_build: DEFINES += "Q_CLI_PARSER_EXPORT="
