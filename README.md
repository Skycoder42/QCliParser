# QCliParser
An extension of QCommandlineParser to make context based CLIs easier

## Features
 - Similar API to QCommandlineParser
 - Works by creating command trees
	 - each tree node can have multiple sub-nodes of any kind
	 - each tree node can define additional optional arguments
	 - each tree leaf can define it's own positional arguments

## Installation
The package is provided via qdep, as `Skycoder42/QCliParser`. To use it simply:

1. Install and enable qdep (See [qdep - Installing](https://github.com/Skycoder42/qdep#installation))
2. Add the following to your pro file:
```qmake
QDEP_DEPENDS += Skycoder42/QCliParser
!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
```

## Example
Assume the following application:

```cpp
QCoreApplication a(argc, argv);
QCliParser parser;
parser.addVersionOption();
parser.addHelpOption();

auto leafNode = parser.addLeafNode("leaf", "a leaf command");
leafNode->addPositionalArgument("stuff", "some positional args");

auto ctxNode = parser.addContextNode("context", "a context command");
ctxNode->addOption({"option", "some extra option"});
auto subNode1 = ctxNode->addLeafNode("sub1", "the first sub command");
auto subNode2 = ctxNode->addLeafNode("sub2", "the second sub command");

parser.process(a);
if(parser.enterContext("leaf"))
	qDebug() << "leaf was called with args:" << parser.positionalArguments();
else if(parser.enterContext("context") {
	qDebug() << "option set:" << parser.isSet("option");
	if(parser.enterContext("sub1"))
		qDebug() << "context sub1";
	else if(parser.enterContext("sub2"))
		qDebug() << "context sub2";
}
```

Some examples of calling this with the corresponding output would be (newlines replaced by `;`):

- `leaf 1 2 3`: `leaf was called with args: ("1", "2", "3")`
- `context sub1`: `option set: false; context sub1`
- `context --option sub2`: `option set: true; context sub2`

For a full example, check the demo.
