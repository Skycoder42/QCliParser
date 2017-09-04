#include <QCoreApplication>
#include <QDebug>
#include <qcliparser.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("qcliparser-demo"));
	QCoreApplication::setApplicationVersion(QStringLiteral("4.2.0"));

	QCliParser parser;
	parser.addVersionOption();
	parser.addHelpOption();
	parser.setApplicationDescription(QStringLiteral("An application to demonstrate the capabilities of QCliParser"));

	parser.addOption({
						 {QStringLiteral("e"), QStringLiteral("exit-code")},
						 QStringLiteral("The the exit <code> to be used if the program runs successfully. "
						 "The default code is 0"),
						 QStringLiteral("code"),
						 QStringLiteral("0")
					 });

	auto printNode = parser.addContextNode(QStringLiteral("print"), QStringLiteral("Print various things to the console"));
	printNode->addOption({
							 {QStringLiteral("c"), QStringLiteral("colored")},
							 QStringLiteral("Prints the graphics colored, instead of black and white")
						 });
	auto printTreeNode = printNode->addLeafNode(QStringLiteral("tree"), QStringLiteral("print a tree"));
	printTreeNode->addOption({
								 QStringLiteral("size"),
								 QStringLiteral("Choose the <size> (in pixels) the tree should be high"),
								 QStringLiteral("size"),
								 QStringLiteral("42")
							 });
	printTreeNode->addOption({
								 QStringLiteral("season"),
								 QStringLiteral("Choose the <season> (spring/summer/fall/winter) to show the tree for"),
								 QStringLiteral("season"),
								 QStringLiteral("summer")
							 });
	auto printNumbersNode = printNode->addLeafNode(QStringLiteral("sum"), QStringLiteral("print the sum of numbers"));
	printNumbersNode->addPositionalArgument(QStringLiteral("numbers"),
											QStringLiteral("The numbers to be summed up"),
											QStringLiteral("number..."));

	auto messageNode = parser.addContextNode(QStringLiteral("message"), QStringLiteral("print some kind of message"));
	messageNode->addOption({
							   QStringLiteral("scream"),
							   QStringLiteral("Scream the message, instead of beeing friendly")
						   });
	messageNode->addOption({
							   {QStringLiteral("f"), QStringLiteral("failure")},
							   QStringLiteral("Print the message as failure (to stderr) instead of "
							   "a normal message (stdout)")
						   });

	auto messageEchoNode = messageNode->addLeafNode(QStringLiteral("echo"), QStringLiteral("Echo additional arguments"));
	messageEchoNode->addPositionalArgument(QStringLiteral("message"),
										   QStringLiteral("The message to be printed (can contain spaces)"),
										   QStringLiteral("[message]"));

	messageNode->addLeafNode(QStringLiteral("random"), QStringLiteral("Print some random stuff"));
	messageNode->addLeafNode(QStringLiteral("help"), QStringLiteral("print the full help information, not just the selective one"));

	parser.process(a);

	qInfo() << "Context:" << parser.contextChain();
	qInfo() << "Options:" << parser.optionNames();
	qInfo() << "Arguments:" << parser.positionalArguments();
	return parser.value(QStringLiteral("e")).toInt();
}
