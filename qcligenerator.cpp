#include "qcligenerator.h"

QCliGenerator::QCliGenerator(QObject *parent) :
	QObject(parent)
{}

bool QCliGenerator::setOrchestrator(QCliOrchestrator *orchestrator, bool takeOwnership)
{

}

bool QCliGenerator::addContext(const QMetaObject *generatorMetaObject)
{

}

bool QCliGenerator::addContext(QObject *generator)
{

}

void QCliGenerator::prepareParser(QCliParser &parser)
{

}

void QCliGenerator::prepareParser(QCommandLineParser &parser)
{

}

int QCliGenerator::exec(QCliParser &parser)
{

}

int QCliGenerator::exec(QCommandLineParser &parser)
{

}



QCliOrchestrator::QCliOrchestrator() = default;

QCliOrchestrator::~QCliOrchestrator() = default;



QCliMetaOrchestrator::QCliMetaOrchestrator(QObject *parent) :
	QObject{parent}
{}

QList<QPair<QString, QString>> QCliMetaOrchestrator::contextList(const QStringList &prefix) const
{

}

const QObject *QCliMetaOrchestrator::createGenerator(const QStringList &context, QObject *parent)
{

}
