#ifndef ADD_MSG_CONNECTION_H
#define ADD_MSG_CONNECTION_H

/// COMPONENT
#include "command.h"
#include <csapex/model/model_fwd.h>
#include <csapex/signal/signal_fwd.h>
#include <csapex/command/add_connection.h>
#include <csapex/utility/uuid.h>

namespace csapex
{
namespace command
{

class CSAPEX_COMMAND_EXPORT AddMessageConnection : public AddConnection
{
public:
    AddMessageConnection(const AUUID& graph_uuid, const UUID &from_uuid, const UUID &to_uuid, bool active);

protected:
    bool doExecute() override;

    void refresh() override;

private:
    Output* from;
    Input* to;

    bool active;
};
}
}
#endif // ADD_MSG_CONNECTION_H

