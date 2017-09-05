#include "qcliparser.h"
#include <QDebug>
#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED) && !defined(Q_OS_WINRT)
#  include <qt_windows.h>
#endif

//base on QCommandLineParser code:

extern void Q_CORE_EXPORT qt_call_post_routines();

#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED) && !defined(Q_OS_WINRT)
// Return whether to use a message box. Use handles if a console can be obtained
// or we are run with redirected handles (for example, by QProcess).
static inline bool displayMessageBox()
{
	if (GetConsoleWindow())
		return false;
	STARTUPINFO startupInfo;
	startupInfo.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&startupInfo);
	return !(startupInfo.dwFlags & STARTF_USESTDHANDLES);
}
#endif // Q_OS_WIN && !QT_BOOTSTRAPPED && !Q_OS_WIN && !Q_OS_WINRT

static void showParserMessage(const QString &message)
{
#if defined(Q_OS_WINRT)
	if (type == UsageMessage)
		qInfo(qPrintable(message));
	else
		qCritical(qPrintable(message));
	return;
#elif defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED)
	if (displayMessageBox()) {
		const UINT flags = MB_OK | MB_TOPMOST | MB_SETFOREGROUND | MB_ICONERROR;
		QString title;
		if (QCoreApplication::instance())
			title = QCoreApplication::instance()->property("applicationDisplayName").toString();
		if (title.isEmpty())
			title = QCoreApplication::applicationName();
		MessageBoxW(0, reinterpret_cast<const wchar_t *>(message.utf16()),
					reinterpret_cast<const wchar_t *>(title.utf16()), flags);
		return;
	}
#endif // Q_OS_WIN && !QT_BOOTSTRAPPED
	fputs(qPrintable(message), stderr);
}



QCliParser::QCliParser() :
	QCommandLineParser(),
	QCliContext(),
	_contextChain(),
	_errorText(),
	_readContextIndex(-1)
{}

void QCliParser::process(const QStringList &arguments)
{
	if(parse(arguments)) {
		if(QCommandLineParser::isSet(QStringLiteral("help")))
			showHelp(EXIT_SUCCESS);
		if(QCommandLineParser::isSet(QStringLiteral("version")))
			showVersion();
	} else {
		showParserMessage(errorText() + QLatin1Char('\n'));
		qt_call_post_routines();
		::exit(EXIT_FAILURE);
	}
}

void QCliParser::process(const QCoreApplication &app)
{
	Q_UNUSED(app)
	process(QCoreApplication::arguments());
}

bool QCliParser::parse(const QStringList &arguments)
{
#ifndef QT_NO_DEBUG
	if(_readContextIndex != -1) {
		qWarning() << "Parser is currently in context scope. "
					  "Parsing different arguments then before can lead to undefined behaviour!";
	}
#endif
	_errorText.clear();
	try {
		parseContext(this, arguments);
		return true;
	} catch (QString &string) {
		if(_contextChain.isEmpty())
			_errorText = string;
		else {
			_errorText = tr("%1\nCommand-Context: %2")
						 .arg(string)
						 .arg(_contextChain.join(QStringLiteral(" -> ")));
		}
		return false;
	}
}

bool QCliParser::enterContext(const QString &name)
{
	auto nIndex = _readContextIndex + 1;
	if(nIndex < 0 ||
	   nIndex >= _contextChain.size())
		return false;

	if(_contextChain[nIndex] == name) {
		_readContextIndex++;
		return true;
	} else
		return false;
}

QString QCliParser::currentContext() const
{
	if(_readContextIndex < 0 ||
	   _readContextIndex >= _contextChain.size())
		return {};
	else
		return _contextChain[_readContextIndex];
}

bool QCliParser::leaveContext()
{
	if(_readContextIndex >= 0) {
		_readContextIndex--;
		return true;
	} else
		return false;
}

QStringList QCliParser::contextChain() const
{
	return _contextChain;
}

QString QCliParser::errorText() const
{
	return _errorText;
}

void QCliParser::addPositionalArgument(const QString &name, const QString &description, const QString &syntax)
{
	Q_UNREACHABLE();
	Q_UNUSED(name)
	Q_UNUSED(description)
	Q_UNUSED(syntax)
}

void QCliParser::clearPositionalArguments()
{
	Q_UNREACHABLE();
}

void QCliParser::parseContext(QCliContext *context, QStringList arguments)
{
	Q_ASSERT_X(!context->_nodes.isEmpty(),
			   Q_FUNC_INFO,
			   qPrintable(QStringLiteral("A QCliContext must have at least 1 node. At chain: %1")
						  .arg(_contextChain.join(QStringLiteral("->")))));

	// reset args + add options
	QCommandLineParser::clearPositionalArguments();
	QCommandLineParser::addOptions(context->_options);

	//create positional args
	auto commands = context->_nodes.keys();
	auto firstName = commands.first();
	auto pFirstName = firstName;
	if(firstName == context->_defaultNode)
		pFirstName = tr("%1 (default)").arg(firstName);
	QCommandLineParser::addPositionalArgument(pFirstName,
											  context->_nodes.value(firstName).first,
											  (context->_defaultNode.isNull() ? QStringLiteral("%1%2%3") : QStringLiteral("%1%2[%3]"))
											  .arg(_contextChain.join(QLatin1Char(' ')))
											  .arg(_contextChain.isEmpty() ? QString() : QStringLiteral(" "))
											  .arg(commands.join(QLatin1Char('|'))));
	for(auto i = 1; i < commands.size(); i++) {
		auto name = commands[i];
		auto pName = name;
		if(name == context->_defaultNode)
			pName = tr("%1 (default)").arg(name);
		QCommandLineParser::addPositionalArgument(pName, context->_nodes.value(name).first, QStringLiteral(" \b"));
	}

	// parse . if no errors and version -> done
	if(QCommandLineParser::parse(arguments) &&
	   QCommandLineParser::isSet(QStringLiteral("version")))
		return;
	//ignore errors, only treated on leafs

	//determine the selected command
	auto pArgs = QCommandLineParser::positionalArguments();
	auto cIndex = -1;
	if(!pArgs.isEmpty()) {
		cIndex = commands.indexOf(pArgs.first());
		if(cIndex == -1)
			throw tr("Unknown command \"%1\"").arg(pArgs.first());
	} else {
		if(QCommandLineParser::isSet(QStringLiteral("help")))
			return;
		cIndex = commands.indexOf(context->_defaultNode);
		if(cIndex == -1)
			throw tr("A command must be specified");
	}

	// get the next node and it's type
	auto nextContext = commands[cIndex];
	_contextChain.append(nextContext);
	arguments.removeOne(nextContext); //remove the command from the args list, as it is already processed
	auto nextNode = context->_nodes.value(nextContext).second;

	if(auto contextNode = nextNode.dynamicCast<QCliContext>())
		parseContext(contextNode.data(), arguments);
	else if(auto leafNode = nextNode.dynamicCast<QCliLeaf>())
		parseLeaf(leafNode.data(), arguments);
	else
		throw tr("Unknown QCliNode type. Must be QCliContext or QCliLeaf");
}

void QCliParser::parseLeaf(QCliLeaf *leaf, const QStringList &arguments)
{
	// reset args + add options
	QCommandLineParser::clearPositionalArguments();
	QCommandLineParser::addOptions(leaf->_options);

	if(leaf->_arguments.isEmpty())
		QCommandLineParser::addPositionalArgument(QStringLiteral(" "), QStringLiteral(" "), _contextChain.join(QLatin1Char(' ')));
	else {
		auto first = leaf->_arguments.first();
		QCommandLineParser::addPositionalArgument(std::get<0>(first),
												  std::get<1>(first),
												  QStringLiteral("%1 %2")
												  .arg(_contextChain.join(QLatin1Char(' ')))
												  .arg(std::get<2>(first)));
		for(auto i = 1; i < leaf->_arguments.size(); i++) {
			const auto &pArg = leaf->_arguments[i];
			QCommandLineParser::addPositionalArgument(std::get<0>(pArg), std::get<1>(pArg), std::get<2>(pArg));
		}
	}

	//parse completly now, must be valid!
	if(!QCommandLineParser::parse(arguments))
		throw QCommandLineParser::errorText();
}
