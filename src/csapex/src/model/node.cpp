/// HEADER
#include <csapex/model/node.h>

/// COMPONENT
#include <csapex/command/meta.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/slot.h>
#include <csapex/signal/trigger.h>
#include <csapex/model/node_state.h>
#include <csapex/model/node_worker.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/assert.h>
#include <csapex/core/settings.h>

/// SYSTEM
#include <boost/foreach.hpp>

using namespace csapex;

Node::Node()
    : adebug(std::cout, ""), ainfo(std::cout, ""), awarn(std::cout, ""), aerr(std::cerr, ""),
      modifier_(NULL), settings_(NULL),
      worker_(NULL)
{
}

Node::~Node()
{
    delete modifier_;
}

void Node::initialize(const std::string& type, const UUID& uuid,
                   NodeWorker* node_worker, Settings* settings)
{
    worker_ = node_worker;
    modifier_ = new NodeModifier(node_worker),
    settings_ = settings;
    node_state_.reset(new NodeState(node_worker));

    parameter_state_->setParentUUID(uuid);
    node_state_->setLabel(uuid);

    std::string p = uuid.getFullName();
    adebug.setPrefix(p);
    ainfo.setPrefix(p);
    awarn.setPrefix(p);
    aerr.setPrefix(p);
}

void Node::doSetup()
{
    setupParameters();

    try {
        modifier_->addSlot("enable", boost::bind(&NodeWorker::setEnabled, worker_, true));
        modifier_->addSlot("disable", boost::bind(&NodeWorker::setEnabled, worker_, false));

        setup();
    } catch(std::runtime_error& e) {
        aerr << "setup failed: " << e.what() << std::endl;
    }
}

void Node::messageArrived(Input *)
{

}
void Node::setupParameters()
{

}

void Node::stateChanged()
{

}

void Node::process()
{
}

NodeState::Ptr Node::getNodeStateCopy() const
{
    apex_assert_hard(node_state_);

    NodeState::Ptr memento(new NodeState(getNodeWorker()));
    *memento = *node_state_;

    memento->setParameterState(getParameterStateClone());

    return memento;
}

NodeState::Ptr Node::getNodeState()
{
    apex_assert_hard(node_state_);

    return node_state_;
}

void Node::setNodeState(NodeState::Ptr memento)
{
    boost::shared_ptr<NodeState> m = boost::dynamic_pointer_cast<NodeState> (memento);
    apex_assert_hard(m.get());

    *node_state_ = *m;

    node_state_->setParent(getNodeWorker());
    if(m->getParameterState()) {
        setParameterState(m->getParameterState());
    }

    Q_EMIT worker_->nodeStateChanged();

    stateChanged();
}

void Node::triggerModelChanged()
{
    Q_EMIT worker_->nodeModelChanged();
}

bool Node::canTick()
{
    return true;
}

void Node::tick()
{
}

void Node::abort()
{
}

NodeWorker* Node::getNodeWorker() const
{
    return worker_;
}


void Node::errorEvent(bool error, const std::string& msg, ErrorLevel level)
{
    aerr << msg << std::endl;

    if(node_state_->isEnabled() && error && level == EL_ERROR) {
        worker_->setIOError(true);
    } else {
        worker_->setIOError(false);
    }
}
