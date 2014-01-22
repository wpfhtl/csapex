#ifndef NODE_ADAPTER_H
#define NODE_ADAPTER_H

/// COMPONENT
#include <csapex/csapex_fwd.h>

/// PROJECT
#include <utils_param/parameter.h>

/// SYSTEM
#include <QLayout>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <vector>

namespace csapex
{

class NodeAdapterBridge : public QObject
{
    Q_OBJECT

public:
    NodeAdapterBridge(NodeAdapter* parent);

public Q_SLOTS:
    void modelChangedEvent();
    void rebuildEvent();

Q_SIGNALS:
    void guiChanged();
    void rebuild();

public:
    void triggerGuiChanged();
    void triggerRebuild();

private:
    NodeAdapter* parent_;
};

class NodeAdapter : public param::Parameter::access
{
    friend class NodeAdapterBridge;

public:
    typedef boost::shared_ptr<NodeAdapter> Ptr;

public:
    NodeAdapter();
    virtual ~NodeAdapter();

    void setNode(Node* node);
    Node* getNode();

    void setupUiAgain();
    void doSetupUi(QBoxLayout* layout);
    virtual void updateDynamicGui(QBoxLayout* layout);

    void modelChangedEvent();

protected:
    virtual void setupUi(QBoxLayout* layout);

    template <typename T>
    void updateParam(const std::string& name, T value);

    void updateParamSet(const std::string& name, const std::string& value);

    template <typename T>
    void updateUi(const param::Parameter* p, boost::function<void(T)> setter);

    void updateUiSet(const param::Parameter* p, boost::function<void(const std::string&)> setter);

protected:
    void guiChanged();
    void clear();

public:
    NodeAdapterBridge bridge;

private:
    QBoxLayout *layout_;
    bool is_gui_setup_;

    Node* node_;

    std::vector<QObject*> callbacks;
    std::vector<boost::signals2::connection> connections;
};

}

#endif // NODE_ADAPTER_H
