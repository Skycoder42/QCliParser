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

auto logFilterEnabled = false;
QLoggingCategory::CategoryFilter oldCategoryFilter = nullptr;
QLoggingCategory *dCat = nullptr;

void cliLogFilter(QLoggingCategory *category)
{
	oldCategoryFilter(category);
	if (qstrcmp(category->categoryName(), "default") == 0) {
		dCat = category;
		category->setEnabled(QtWarningMsg, !logFilterEnabled);
	}
}

}

QCliEvaluator::QCliEvaluator(QObject *parent) :
	QObject{parent}
{
	if (!oldCategoryFilter)
		oldCategoryFilter = QLoggingCategory::installFilter(cliLogFilter);
}

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
				QByteArrayLiteral("Evaluator*") :
				(QByteArrayLiteral("Evaluator_") + contextList.join(QLatin1Char('_')).toUtf8() + '*');
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
	const auto &pArgs = parser.positionalArguments();
	const auto argSize = pArgs.size();
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
			switch (method.parameterType(pCount - 1)) {
			case QMetaType::QStringList:
			case QMetaType::QByteArrayList:
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
		if (argSize < pCount ||
			(!anyArgs && argSize > pCount))
			continue;

		// if acceptable -> convert params into variant list
		QVariantList varList;
		varList.reserve(anyArgs ? pCount + 1 : pCount);
		for (auto aIdx = 0; aIdx < pCount; ++aIdx)
			varList.append(pArgs[aIdx]);
		if (anyArgs && argSize > pCount) {
			QVariant listArg;
			switch (method.parameterType(pCount)) {
			case QMetaType::QVariantList:
			case QMetaType::QStringList:
				listArg = QVariant::fromValue(pArgs.mid(pCount));
				break;
			case QMetaType::QByteArrayList: {
				QByteArrayList baList;
				baList.reserve(argSize - pCount);
				for (const auto &arg : pArgs.mid(pCount))
					baList.append(arg.toUtf8());
				listArg = QVariant::fromValue(baList);
				break;
			}
			default:
				Q_UNREACHABLE();
			}
			varList.append(listArg);
		}

		// create the object and call the method
		QScopedPointer<QObject> instance {metaObject->newInstance(Q_ARG(QObject*, this))};
		if (!instance) {
			qCCritical(cliEval) << "Failed to create instance of class" << metaObject->className()
								<< "- make shure the constructor has the following signature: "
								   "Q_INVOKABLE constructor(QObject*);";
			return EXIT_FAILURE;
		}
		// set options
		setOptionProperties(instance.data(), parser);
		// call method with positional args
		return callMetaMethod(instance.data(), method, varList);
	}

	// no method found that matches the given name
	if (pCountMinTotal == std::numeric_limits<int>::max())
		return std::nullopt;
	else { // method was found, but arguments do not match
		auto message = tr("Expected ");
		if (pCountMinTotal > 0)
			message += tr("at least %L1 ").arg(pCountMinTotal);
		if (pCountMaxTotal != std::numeric_limits<int>::max()) {
			if (pCountMinTotal > 0)
				message += tr("and ");
			message += tr("at most %L1 ").arg(pCountMaxTotal);
		}
		message += tr("arguments, but %L1 have been passed!\n").arg(argSize);
		QCliParser::showParserMessage(message);
		return EXIT_FAILURE;
	}
}

void QCliEvaluator::setOptionProperties(QObject *instance, const QCommandLineParser &parser) const
{
	const auto metaObject = instance->metaObject();
	for (auto i = 1; i < metaObject->propertyCount(); ++i) {
		const auto property = metaObject->property(i);
		if (!property.isWritable())
			continue;
		// is set -> update property
		const auto pName = QString::fromUtf8(property.name()).replace(QLatin1Char('_'), QLatin1Char('-'));

		auto isSet = false;
		{
			LogBlocker blocker;
			isSet = parser.isSet(pName);
		}
		if(isSet) {
			switch (property.userType()) {
			case QMetaType::Bool:
				property.write(instance, true);
				break;
			case QMetaType::QStringList:
			case QMetaType::QVariantList:
				property.write(instance, parser.values(pName));
				break;
			case QMetaType::QByteArrayList: {
				const auto pValues = parser.values(pName);
				QByteArrayList baList;
				baList.reserve(pValues.size());
				for (const auto &arg : pValues)
					baList.append(arg.toUtf8());
				property.write(instance, QVariant::fromValue(baList));
				break;
			}
			default:
				property.write(instance, parser.value(pName));
				break;
			}
		}
	}
}

int QCliEvaluator::callMetaMethod(QObject *instance, const QMetaMethod &method, QVariantList arguments) const
{
	Q_ASSERT_X(arguments.size() <= 10, Q_FUNC_INFO, "Trying to invoke methode with more then 10 arguments!");

	// prepare arguments
	std::array<QGenericArgument, 10> args;
	for (auto i = 0; i < arguments.size(); ++i) {
		auto &val = arguments[i];
		if (!val.convert(method.parameterType(i))) {
			QCliParser::showParserMessage(tr("Invalid positional argument at position %L1 "
											 "- unable to convert input to %2\n")
										  .arg(i)
										  .arg(QString::fromUtf8(QMetaType::typeName(method.parameterType(i)))));
			return EXIT_FAILURE;
		}
		args[static_cast<size_t>(i)] = QGenericArgument{val.typeName(), val.constData()};
	}

	int retVal = EXIT_FAILURE;
	if (method.invoke(instance, Qt::DirectConnection,
					  Q_RETURN_ARG(int, retVal),
					  args[0], args[1], args[2], args[3], args[4],
					  args[5], args[6], args[7], args[8], args[9]))
		return retVal;
	else {
		qCCritical(cliEval) << "Failed to call method" << method.methodSignature()
							<< "on instance" << instance;
		return EXIT_FAILURE;
	}
}



QCliEvaluator::LogBlocker::LogBlocker()
{
	logFilterEnabled = true;
	if (dCat)
		dCat->setEnabled(QtWarningMsg, false);
}

QCliEvaluator::LogBlocker::~LogBlocker()
{
	logFilterEnabled = false;
	if (dCat)
		dCat->setEnabled(QtWarningMsg, true);
}
