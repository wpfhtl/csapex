/// HEADER
#include <csapex/command/add_connector.h>

/// COMPONENT
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/model/connection_type.h>
#include <csapex/model/graph.h>
#include <csapex/model/node.h>
#include <csapex/command/dispatcher.h>
#include <csapex/msg/message_factory.h>
#include <csapex/utility/assert.h>
#include <csapex/model/node_worker.h>

using namespace csapex;
using namespace command;

AddConnector::AddConnector(const UUID &box_uuid, const std::string& label, const std::string& type, bool input, const UUID &uuid)
    : type(type), label(label), input(input), c(NULL), b_uuid(box_uuid), c_uuid(uuid)
{

}

std::string AddConnector::getType() const
{
    return "AddConnector";
}

std::string AddConnector::getDescription() const
{
    return std::string("added a connector with UUID ") + c_uuid.getFullName() + " to " + b_uuid.getFullName();
}


bool AddConnector::doExecute()
{
    Node* node = graph_->findNode(b_uuid);
    apex_assert_hard(node);

    if(input) {
        UUID uuid = c_uuid.empty() ? Connectable::makeUUID(node->getUUID(), Connectable::TYPE_IN, node->countInputs()) : c_uuid;
        Input* in = new Input(graph_->getSettings(), uuid);
        c = in;
        node->getNodeWorker()->registerInput(in);
    } else {
        UUID uuid = c_uuid.empty() ? Connectable::makeUUID(node->getUUID(), Connectable::TYPE_OUT, node->countOutputs()) : c_uuid;
        Output* out = new Output(graph_->getSettings(), uuid);
        c = out;
        node->getNodeWorker()->registerOutput(out);
    }

    c->setType(MessageFactory::createMessage(type));
    c->setLabel(label);
    c_uuid = c->getUUID();

    return true;
}

bool AddConnector::doUndo()
{
    Node* node = graph_->findNode(b_uuid);
    apex_assert_hard(node);

    if(input) {
        node->removeInput(node->getInput(c_uuid));
    } else {
        node->removeOutput(node->getOutput(c_uuid));
    }
    return false;
}

bool AddConnector::doRedo()
{
    return doExecute();
}
