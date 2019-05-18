#ifndef QCLIGENERATOR_META_H
#define QCLIGENERATOR_META_H

#include <QObject>

#define Q_CLI_ORCHESTRATOR_OBJECT() Q_CLASSINFO("__q_cli_orchestrator", "")
#define Q_CLI_CONTEXT_OBJECT(path) Q_CLASSINFO("__q_cli_context", #path)
#define Q_CLI_PREFIX(prefix), Q_CLASSINFO("__q_cli_prefix", #prefix "_")  // defaults to "qCli_"

#endif // QCLIGENERATOR_META_H
