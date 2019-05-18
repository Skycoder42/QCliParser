HEADERS += \
	$$PWD/qclievaluator.h \
	$$PWD/qcligenerator.h \
	$$PWD/qcligenerator_meta.h \
	$$PWD/qcliparser.h \
	$$PWD/qclinode.h \
	$$PWD/qmultitree.h

SOURCES += \
	$$PWD/qclievaluator.cpp \
	$$PWD/qcligenerator.cpp \
	$$PWD/qcliparser.cpp \
	$$PWD/qclinode.cpp

win32: LIBS += -luser32

INCLUDEPATH += $$PWD

QDEP_PACKAGE_EXPORTS += Q_CLI_PARSER_EXPORT
!qdep_build: DEFINES += "Q_CLI_PARSER_EXPORT="
