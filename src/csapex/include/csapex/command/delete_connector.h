#ifndef COMMAND_DELETE_CONNECTOR_H
#define COMMAND_DELETE_CONNECTOR_H

/// COMPONENT
#include "command.h"
#include <csapex/utility/uuid.h>

namespace csapex
{

namespace command
{

struct DeleteConnector : public Command
{
    DeleteConnector(Connectable *_c);

protected:
    bool doExecute();
    bool doUndo();
    bool doRedo();

    bool refresh();

    virtual std::string getType() const;
    virtual std::string getDescription() const;

private:
    bool       in;
    Connectable* c;

    Command::Ptr    delete_connections;

    UUID c_uuid;

};
}
}
#endif // COMMAND_DELETE_CONNECTOR_H
