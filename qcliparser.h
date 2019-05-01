#ifndef QCLIPARSER_H
#define QCLIPARSER_H

#include "qclinode.h"

#include <QtCore/QCommandLineParser>

class Q_CLI_PARSER_EXPORT QCliParser : public QCommandLineParser, public QCliContext
{
	Q_DECLARE_TR_FUNCTIONS(QCliParser)

public:
	QCliParser();

	using QCliContext::addOption;
	using QCliContext::addOptions;

	void process(const QStringList &arguments, bool colored = false);
	void process(const QCoreApplication &app, bool colored = false);
	bool parse(const QStringList &arguments);

	bool enterContext(const QString &name);
	QString currentContext() const;
	bool leaveContext();

	QStringList contextChain() const;
	QString errorText() const;

private:
	friend class QCliEvaluator;

	QStringList _contextChain;
	QString _errorText;

	int _readContextIndex;

	static void showParserMessage(const QString &message);

	//hide
	Q_NORETURN void addPositionalArgument(const QString &name, const QString &description, const QString &syntax = QString());
	Q_NORETURN void clearPositionalArguments();

	void parseContext(QCliContext *context, QStringList arguments);
	void parseLeaf(QCliLeaf *leaf, const QStringList &arguments);
};

#endif // QCLIPARSER_H
