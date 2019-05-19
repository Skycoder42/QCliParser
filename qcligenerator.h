#ifndef QCLIGENERATOR_H
#define QCLIGENERATOR_H

#include <QObject>
#include "qcliparser.h"

#include "qcligenerator_meta.h"

class Q_CLI_PARSER_EXPORT QCliOrchestrator
{
	Q_DISABLE_COPY(QCliOrchestrator)
public:
	QCliOrchestrator();
	virtual ~QCliOrchestrator();

	virtual QList<QPair<QString, QString>> contextList(const QStringList &prefix) const = 0;
	virtual const QObject *createGenerator(const QStringList &context, QObject *parent) = 0;
};

class Q_CLI_PARSER_EXPORT QCliMetaOrchestrator : public QObject, public QCliOrchestrator
{
	Q_OBJECT
	Q_INTERFACES(QCliMetaOrchestrator)

public:
	QCliMetaOrchestrator(QObject *parent = nullptr);

	QList<QPair<QString, QString> > contextList(const QStringList &prefix) const override;
	const QObject *createGenerator(const QStringList &context, QObject *parent) override;

private:

};

class Q_CLI_PARSER_EXPORT QCliGenerator : public QObject
{
	Q_OBJECT

public:
	explicit QCliGenerator(QObject *parent = nullptr);

	template <typename TOchestrator>
	bool setOrchestrator();
	bool setOrchestrator(QCliOrchestrator *orchestrator, bool takeOwnership = false);

	template <typename TGenerator>
	bool addContext(const QStringList &path);
	bool addContext(const QMetaObject *generatorMetaObject);
	bool addContext(QObject *generator);

	void prepareParser(QCliParser &parser);
	void prepareParser(QCommandLineParser &parser);

	Q_INVOKABLE int exec(QCliParser &parser);
	Q_INVOKABLE int exec(QCommandLineParser &parser);
};

#define QCliOrchestratorIid "de.skycoder42.qcliparser.QCliOrchestrator"
Q_DECLARE_INTERFACE(QCliOrchestrator, QCliOrchestratorIid)

#endif // QCLIGENERATOR_H
