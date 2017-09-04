#ifndef QCLINODE_H
#define QCLINODE_H

#include <QCommandLineOption>
#include <QSet>
#include <QSharedPointer>

#include <tuple>

class QCliNode
{
	friend class QCliParser;
public:
	QCliNode();
	virtual ~QCliNode();

	bool addOption(const QCommandLineOption &commandLineOption);
	bool addOptions(const QList<QCommandLineOption> &options);

private:
	QList<QCommandLineOption> _options;
	QSet<QString> _keyCache;
};

class QCliLeaf : public QCliNode
{
	friend class QCliParser;
public:
	QCliLeaf();

	void addPositionalArgument(const QString &name, const QString &description, const QString &syntax = {});

private:
	QList<std::tuple<QString, QString, QString>> _arguments;
};

class QCliContext : public QCliNode
{
	friend class QCliParser;
public:
	QCliContext();

	bool addCliNode(const QString &name, const QString &description, const QSharedPointer<QCliNode> &node);
	template <typename TNode>
	QSharedPointer<TNode> addCliNode(const QString &name, const QString &description);
	void setDefaultNode(const QString &name);

	template <typename TNode = QCliNode>
	QSharedPointer<TNode> getNode(const QString &name) const;

private:
	QHash<QString, QPair<QString, QSharedPointer<QCliNode>>> _nodes;
	QString _defaultNode;
};

// ------------- GENERIC IMPLEMENTATION -------------

template<typename TNode>
QSharedPointer<TNode> QCliContext::addCliNode(const QString &name, const QString &description)
{
	auto node = QSharedPointer<TNode>::create();
	if(addCliNode(name, description, node))
		return node;
	else
		return {};
}

template<typename TNode>
QSharedPointer<TNode> QCliContext::getNode(const QString &name) const
{
	return _nodes.value(name).first;
}


#endif // QCLINODE_H
