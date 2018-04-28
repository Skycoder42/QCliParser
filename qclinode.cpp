#include "qclinode.h"

QCliNode::QCliNode() :
	_options(),
	_keyCache(),
	_hidden(false)
{}

QCliNode::~QCliNode() = default;

bool QCliNode::addOption(const QCommandLineOption &commandLineOption)
{
	for(const auto &key : commandLineOption.names()) {
		if(_keyCache.contains(key))
			return false;
	}

	_options.append(commandLineOption);
	_keyCache.unite(QSet<QString>::fromList(commandLineOption.names()));
	return true;
}

bool QCliNode::addOptions(const QList<QCommandLineOption> &options)
{
	auto tSet(_keyCache);
	for(const auto &option : options) {
		for(const auto &key : option.names()) {
			if(tSet.contains(key))
				return false;
		}
		tSet.unite(QSet<QString>::fromList(option.names()));
	}

	_options.append(options);
	_keyCache.unite(tSet);
	return true;
}

void QCliNode::setHidden(bool hidden)
{
	_hidden = hidden;
}

bool QCliNode::isHidden() const
{
	return _hidden;
}


QCliLeaf::QCliLeaf() :
	QCliNode(),
	_arguments()
{}

void QCliLeaf::addPositionalArgument(const QString &name, const QString &description, const QString &syntax)
{
	_arguments.append(std::make_tuple(
						  name,
						  description,
						  syntax.isEmpty() ?
							QStringLiteral("<%1>").arg(name) :
							syntax
					  ));
}

QCliContext::QCliContext() :
	QCliNode(),
	_nodes(),
	_defaultNode()
{}

bool QCliContext::addCliNode(const QString &name, const QString &description, const QSharedPointer<QCliNode> &node)
{
	if(_nodes.contains(name))
		return false;
	else {
		_nodes.insert(name, {description, node});
		return true;
	}
}

QSharedPointer<QCliContext> QCliContext::addContextNode(const QString &name, const QString &description)
{
	auto ptr = QSharedPointer<QCliContext>::create();
	if(addCliNode(name, description, ptr))
		return ptr;
	else
		return {};
}

QSharedPointer<QCliLeaf> QCliContext::addLeafNode(const QString &name, const QString &description)
{
	auto ptr = QSharedPointer<QCliLeaf>::create();
	if(addCliNode(name, description, ptr))
		return ptr;
	else
		return {};
}

void QCliContext::setDefaultNode(const QString &name)
{
	_defaultNode = name;
}
