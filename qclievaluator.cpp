#include "qclievaluator.h"
#include <QtCore/QMetaMethod>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>

namespace {

#ifdef QT_NO_DEBUG
constexpr auto LogScope = QtFatalMsg;
#else
constexpr auto LogScope = QtDebugMsg;
#endif

Q_LOGGING_CATEGORY(cliEval, "QCliEvaluator", LogScope)

}

QCliEvaluator::QCliEvaluator(QObject *parent) :
	QObject{parent}
{}

bool QCliEvaluator::doesAutoResolveObjects() const
{
	return _autoResolveObjects;
}

int QCliEvaluator::exec(const QCliParser &parser)
{
	return execImpl(parser, parser.contextChain());
}

int QCliEvaluator::exec(const QCommandLineParser &parser)
{
	return execImpl(parser, {});
}

bool QCliEvaluator::registerEvaluator(const QByteArray &className, const QStringList &path)
{
	const auto mo = metaObjectForName(className);
	if (!mo)
		return false;
	return registerEvaluator(mo, path);
}

bool QCliEvaluator::registerEvaluator(const QMetaObject *metaObject, const QStringList &path)
{
	if (!metaObject->inherits(&QObject::staticMetaObject))
		return false;

	auto evalNode = _evaluators.find(path);
	evalNode->setValue(metaObject);
	return true;
}

void QCliEvaluator::setAutoResolveObjects(bool autoResolveObjects)
{
	if (_autoResolveObjects == autoResolveObjects)
		return;

	_autoResolveObjects = autoResolveObjects;
	emit autoResolveObjectsChanged(_autoResolveObjects);
}

QByteArray QCliEvaluator::evaluatorClassName(const QStringList &contextList) const
{
	return contextList.isEmpty() ?
				QByteArrayLiteral("Evaluator") :
				(QByteArrayLiteral("Evaluator_") + contextList.join(QLatin1Char('_')).toUtf8());
}

QByteArray QCliEvaluator::evaluatorMethodName(const QMetaObject *metaObject, const QStringList &contextList) const
{
	Q_UNUSED(metaObject)
	return contextList.isEmpty() ?
				QByteArrayLiteral("exec") :
				(QByteArrayLiteral("exec_") + contextList.join(QLatin1Char('_')).toUtf8());
}

const QMetaObject *QCliEvaluator::metaObjectForName(const QByteArray &className)
{
	const auto typeId = QMetaType::type(className.data());
	if (typeId == QMetaType::UnknownType ||
		!QMetaType::typeFlags(typeId).testFlag(QMetaType::PointerToQObject))
		return nullptr;
	return QMetaType::metaObjectForType(typeId);
}

int QCliEvaluator::execImpl(const QCommandLineParser &parser, const QStringList &contextList)
{
	// find the evaluator node chain
	auto pNode = _evaluators.find(contextList);
	do {
		const auto depth = pNode->depth();
		// first: check if explicit evaluator was set
		if (pNode->hasValue()) {
			const auto res = tryExec(pNode->value(), parser, contextList.mid(depth));
			if (res)
				return res.value();
		}
		// second: if allowed, try to auto-resolve
		if(_autoResolveObjects) {
			const auto evaluatorName = evaluatorClassName(contextList.mid(0, depth));
			const auto metaObj = metaObjectForName(evaluatorName);
			if (metaObj) {
				const auto res = tryExec(metaObj, parser, contextList.mid(depth));
				if (res)
					return res.value();
			}
		}
	} while((pNode = pNode->parent()));
	// no evaluator found...
	qCCritical(cliEval) << "Unable to find any evaluators capable of executing" << contextList;
	return EXIT_FAILURE;
}

std::optional<int> QCliEvaluator::tryExec(const QMetaObject *metaObject, const QCommandLineParser &parser, const QStringList &contextList)
{
	// find a method that matches the generated name and parameters
	const auto methodName = evaluatorMethodName(metaObject, contextList);
	auto pCountMinTotal = std::numeric_limits<int>::max();
	auto pCountMaxTotal = 0;
	for (auto mIdx = 0; mIdx < metaObject->methodCount(); ++mIdx) {
		const auto method = metaObject->method(mIdx);
		// skip non matching methods
		if ((method.methodType() != QMetaMethod::Method &&
			method.methodType() != QMetaMethod::Slot) ||  // slots are allowed, but still must have the int return type
			method.access() != QMetaMethod::Public ||
			method.returnType() != QMetaType::Int ||
			method.name() != methodName)
			continue;

		// find parameter count with any Args
		auto pCount = method.parameterCount();
		auto anyArgs = false;
		if (pCount > 0) {
			const auto pLastType = method.parameterType(pCount - 1);
			switch (pLastType) {
			case QMetaType::QStringList:
			case QMetaType::QByteArrayList: // TODO needs conversion
			case QMetaType::QVariantList:
				anyArgs = true;
				--pCount;
				break;
			default:
				break;
			}
		}
		pCountMinTotal = std::min(pCountMinTotal, pCount);
		pCountMaxTotal = std::max(pCountMaxTotal, anyArgs ? std::numeric_limits<int>::max() : pCount);

		// check if pos args can be passed to this method
		const auto &pArgs = parser.positionalArguments();
		const auto argSize = pArgs.size();
		if (argSize < pCount ||
			(!anyArgs && argSize > pCount))
			continue;

		// if acceptable -> convert params into variant list
		QVariantList varList;
		varList.reserve(anyArgs ? pCount + 1 : pCount);
		for (auto aIdx = 0; aIdx < pCount; ++aIdx)
			varList.append(pArgs[aIdx]);
		if (anyArgs && argSize > pCount)
			varList.append(QVariant::fromValue(pArgs.mid(pCount)));
	}
}
