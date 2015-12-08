#ifndef COMMAND_ADD_NODE_HPP
#define COMMAND_ADD_NODE_HPP

/// COMPONENT
#include <csapex/command/command.h>
#include <csapex/utility/uuid.h>
#include <csapex/data/point.h>

namespace csapex
{

namespace command
{

class AddNode : public Command
{
public:
    AddNode(const std::string& type, Point pos, const UUID& parent_uuid_, const UUID& uuid_, NodeStatePtr state);

protected:
    bool doExecute();
    bool doUndo();
    bool doRedo();

    virtual std::string getType() const;
    virtual std::string getDescription() const;

private:
    std::string type_;
    Point pos_;

    UUID parent_uuid_;
    UUID uuid_;

    NodeStatePtr saved_state_;
};
}
}

#endif // COMMAND_ADD_NODE_HPP
