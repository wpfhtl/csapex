/// HEADER
#include <csapex/message_provider.h>

using namespace csapex;

const MessageProvider::Ptr MessageProvider::NullPtr;

ConnectionType::ConstPtr MessageProvider::getType() const
{
    return type_;
}

void MessageProvider::setType(ConnectionType::Ptr type)
{
    type_ = type;
}

std::string MessageProvider::getName() const
{
    return name_;
}

void MessageProvider::setName(const std::string& name)
{
    name_ = name;
}
