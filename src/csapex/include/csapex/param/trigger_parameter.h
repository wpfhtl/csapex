#ifndef TRIGGER_PARAM_H
#define TRIGGER_PARAM_H

/// COMPONENT
#include <csapex/param/parameter.h>

namespace csapex {
namespace param {

class TriggerParameter : public Parameter
{
    friend class ParameterFactory;

public:
    typedef std::shared_ptr<TriggerParameter> Ptr;

public:
    TriggerParameter();
    explicit TriggerParameter(const std::string& name, const ParameterDescription& description);
    virtual ~TriggerParameter();

    virtual int ID() const override { return 0x007; }
    virtual std::string TYPE() const override { return "trigger"; }

    void trigger();
    virtual bool hasState() const override;

    virtual const std::type_info &type() const override;
    virtual std::string toStringImpl() const override;

    void doSetValueFrom(const Parameter& other) override;
    void doClone(const Parameter& other) override;

    void doSerialize(YAML::Node& e) const override;
    void doDeserialize(const YAML::Node& n) override;

protected:
    virtual boost::any get_unsafe() const override;
    virtual bool set_unsafe(const boost::any& v) override;

private:
};

}
}
#endif // TRIGGER_PARAM_H
