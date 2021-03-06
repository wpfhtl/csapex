/// HEADER
#include <csapex/model/connectable.h>

/// COMPONENT
#include <csapex/model/connection.h>
#include <csapex/msg/message.h>
#include <csapex/msg/any_message.h>

/// SYSTEM
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace csapex;

const std::string Connectable::MIME_CREATE_CONNECTION = "csapex/connectable/create_connection";
const std::string Connectable::MIME_MOVE_CONNECTIONS = "csapex/connectable/move_connections";

//bool Connectable::allow_processing = true;


Connectable::Connectable(const UUID& uuid)
    : Unique(uuid),
      count_(0), seq_no_(-1), enabled_(true)
{
    init();
}

void Connectable::notifyMessageProcessed()
{
    messageProcessed(this);

    for(ConnectionPtr& c : connections_) {
        c->setTokenProcessed();
    }
}

void Connectable::reset()
{
    messageProcessed(this);
}

void Connectable::stop()
{
    notifyMessageProcessed();
}

void Connectable::init()
{
    setType(connection_types::makeEmpty<connection_types::AnyMessage>());
}


Connectable::~Connectable()
{
    for(ConnectionPtr& c : connections_) {
        c->detach(this);
    }
}

void Connectable::errorEvent(bool error, const std::string& msg, ErrorLevel level)
{
    connectableError(error,msg,static_cast<int>(level));
}

void Connectable::validateConnections()
{

}

void Connectable::disable()
{
    if(enabled_) {
        enabled_ = false;
        enabled_changed((bool) enabled_);
    }
}

void Connectable::enable()
{
    if(!enabled_) {
        enabled_ = true;
        enabled_changed((bool) enabled_);
    }
}

void Connectable::setEnabled(bool enabled)
{
    if(enabled) {
        enable();
    } else {
        disable();
    }
}

bool Connectable::isEnabled() const
{
    return enabled_;
}

bool Connectable::canConnectTo(Connectable* other_side, bool) const
{
    if(other_side == this) {
        return false;
    }

    bool in_out = (canOutput() && other_side->canInput()) || (canInput() && other_side->canOutput());
    bool compability = getType()->canConnectTo(other_side->getType().get());

    return in_out && compability;
}


bool Connectable::shouldCreate(bool left, bool)
{
    bool full_input = isInput() && isConnected();
    return left && !full_input;
}

bool Connectable::shouldMove(bool left, bool right)
{
    bool full_input = isInput() && isConnected();
    return (right && isConnected()) || (left && full_input);
}

std::string Connectable::getLabel() const
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    return label_;
}

void Connectable::setLabel(const std::string &label)
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    if(label != label_) {
        label_ = label;
        labelChanged(label_);
    }
}

void Connectable::setType(TokenData::ConstPtr type)
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    bool compatible = type_ && type && type_->canConnectTo(type.get()) && type->canConnectTo(type_.get());

    bool is_any = std::dynamic_pointer_cast<connection_types::AnyMessage const>(type_) != nullptr;
    bool will_be_any = std::dynamic_pointer_cast<connection_types::AnyMessage const>(type) != nullptr;

    if(!compatible || (is_any != will_be_any)) {
        type_ = type;
        validateConnections();
        lock.unlock();

        typeChanged();
    }
}

TokenData::ConstPtr Connectable::getType() const
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    return type_;
}

int Connectable::getCount() const
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    return count_;
}

int Connectable::sequenceNumber() const
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);
    return seq_no_;
}

void Connectable::setSequenceNumber(int seq_no)
{
    std::unique_lock<std::recursive_mutex> lock(sync_mutex);

    seq_no_ = seq_no;
}

void Connectable::addConnection(ConnectionPtr connection)
{
    connections_.push_back(connection);
    connection->sink_enabled_changed.connect(connectionEnabled);

    connection_added(connection);

    connection_added_to(this);
}

void Connectable::fadeConnection(ConnectionPtr connection)
{
    for(auto it = connections_.begin(); it != connections_.end(); ) {
        if(*it == connection) {
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }

    connection_faded(connection);
}
std::vector<ConnectionPtr> Connectable::getConnections() const
{
    return connections_;
}

bool Connectable::hasActiveConnection() const
{
    for(const ConnectionPtr& c : connections_) {
        if(c->isActive()) {
            return true;
        }
    }

    return false;
}

bool Connectable::isConnected() const
{
    return !connections_.empty();
}
