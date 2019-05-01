#ifndef QCLIEVALUATOR_H
#define QCLIEVALUATOR_H

#include "qcliparser.h"
#include "qmultitree.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QVariant>

class Q_CLI_PARSER_EXPORT QCliEvaluator : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool autoResolveObjects READ doesAutoResolveObjects WRITE setAutoResolveObjects NOTIFY autoResolveObjectsChanged)

public:
	explicit QCliEvaluator(QObject *parent = nullptr);

	template <typename TEvaluator>
	bool registerEvaluator(const QStringList &path);

	bool doesAutoResolveObjects() const;

	Q_INVOKABLE int exec(const QCliParser &parser);
	Q_INVOKABLE int exec(const QCommandLineParser &parser);

public Q_SLOTS:
	bool registerEvaluator(const QByteArray &className, const QStringList &path);
	bool registerEvaluator(const QMetaObject *metaObject, const QStringList &path);

	void setAutoResolveObjects(bool autoResolveObjects);

signals:
	void autoResolveObjectsChanged(bool autoResolveObjects);

protected:
	virtual QByteArray evaluatorClassName(const QStringList &contextList) const;
	virtual QByteArray evaluatorMethodName(const QMetaObject *metaObject, const QStringList &contextList) const;

private:
	struct LogBlocker {
		LogBlocker();
		~LogBlocker();
	};

	using EvaluatorTree = QMultiTree<QString, const QMetaObject *>;

	bool _autoResolveObjects = true;

	EvaluatorTree _evaluators;

	static const QMetaObject *metaObjectForName(const QByteArray &className);

	int execImpl(const QCommandLineParser &parser, const QStringList &contextList);
	std::optional<int> tryExec(const QMetaObject *metaObject,
							   const QCommandLineParser &parser,
							   const QStringList &contextList);
	void setOptionProperties(QObject *instance, const QCommandLineParser &parser) const;
	int callMetaMethod(QObject *instance, const QMetaMethod &method, QVariantList arguments) const;
};

template<typename TEvaluator>
bool QCliEvaluator::registerEvaluator(const QStringList &path)
{
	static_assert(std::is_base_of_v<QObject, TEvaluator>, "TEvaluator must extend QObject!");
	return registerEvaluator(&TEvaluator::staticMetaObject, path);
}

#endif // QCLIEVALUATOR_H
