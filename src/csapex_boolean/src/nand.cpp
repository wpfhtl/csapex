/// HEADER
#include "nand.h"

/// COMPONENT
#include <csapex_boolean/boolean_message.h>

/// PROJECT
#include <csapex/model/connector_out.h>
#include <csapex/model/connector_in.h>

/// SYSTEM
#include <csapex/utility/register_apex_plugin.h>

CSAPEX_REGISTER_CLASS(csapex::boolean::NAND, csapex::Node)

using namespace csapex;
using namespace csapex::boolean;
using namespace csapex::connection_types;

NAND::NAND()
{
    addTag(Tag::get("Boolean"));
}

void NAND::setup()
{
    setSynchronizedInputs(true);

    in_a = addInput<BooleanMessage>("A");
    in_b = addInput<BooleanMessage>("B");

    out = addOutput<BooleanMessage>("A nand B");
}

void NAND::process()
{
    BooleanMessage::Ptr a = in_a->getMessage<BooleanMessage>();
    BooleanMessage::Ptr b = in_b->getMessage<BooleanMessage>();

    BooleanMessage::Ptr msg(new BooleanMessage);
    msg->value = !(a->value && b->value);
    out->publish(msg);
}
