#ifndef QCLIPARSER_H
#define QCLIPARSER_H

#include "qclinode.h"

#include <QCommandLineParser>

class QCliParser : public QCommandLineParser, public QCliContext
{
	Q_DECLARE_TR_FUNCTIONS(QCliParser)
public:
	QCliParser();

	using QCliContext::addOption;
	using QCliContext::addOptions;

	void process(const QStringList &arguments);
	void process(const QCoreApplication &app);
	bool parse(const QStringList &arguments);

	bool enterContext(const QString &name);
	QString currentContext() const;
	bool leaveContext();

	QStringList contextChain() const;
	QString errorText() const;

private:
	QStringList _contextChain;
	QString _errorText;

	int _readContextIndex;

	//hide
	void addPositionalArgument(const QString &name, const QString &description, const QString &syntax = QString());
	void clearPositionalArguments();

	void parseContext(QCliContext *context, QStringList arguments);
	void parseLeaf(QCliLeaf *leaf, const QStringList &arguments);
};

#endif // QCLIPARSER_H
