#ifndef QCLINODE_H
#define QCLINODE_H

#include <tuple>

#include <QtCore/QCommandLineOption>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QSharedPointer>

class Q_CLI_PARSER_EXPORT QCliNode
{
	friend class QCliParser;
	Q_DISABLE_COPY(QCliNode)

public:
	QCliNode();
	virtual ~QCliNode();

	bool addOption(const QCommandLineOption &commandLineOption);
	bool addOptions(const QList<QCommandLineOption> &options);

	void setHidden(bool hidden);
	bool isHidden() const;

private:
	QList<QCommandLineOption> _options;
	QSet<QString> _keyCache;
	bool _hidden;
};

class Q_CLI_PARSER_EXPORT QCliLeaf : public QCliNode
{
	friend class QCliParser;
public:
	QCliLeaf();

	void addPositionalArgument(const QString &name, const QString &description, const QString &syntax = {});

private:
	QList<std::tuple<QString, QString, QString>> _arguments;
};

class Q_CLI_PARSER_EXPORT QCliContext : public QCliNode
{
	friend class QCliParser;

public:
	QCliContext();

	bool addCliNode(const QString &name, const QString &description, const QSharedPointer<QCliNode> &node);
	QSharedPointer<QCliContext> addContextNode(const QString &name, const QString &description);
	QSharedPointer<QCliLeaf> addLeafNode(const QString &name, const QString &description);
	void setDefaultNode(const QString &name);

	template <typename TNode = QCliNode>
	QSharedPointer<TNode> getNode(const QString &name) const;

private:
	QMap<QString, QPair<QString, QSharedPointer<QCliNode>>> _nodes;
	QString _defaultNode;
};

// ------------- GENERIC IMPLEMENTATION -------------

template<typename TNode>
QSharedPointer<TNode> QCliContext::getNode(const QString &name) const
{
	return _nodes.value(name).first;
}


#endif // QCLINODE_H
