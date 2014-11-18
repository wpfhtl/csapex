#ifndef WIDGET_CONTROLLER_H
#define WIDGET_CONTROLLER_H

/// PROJECT
#include <csapex/csapex_fwd.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QPoint>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <QLayout>

class QTreeWidget;
class QMenu;
class QAbstractItemModel;

namespace csapex
{

class WidgetController : public QObject
{
    Q_OBJECT

public:
    typedef boost::shared_ptr<WidgetController> Ptr;

public:
    WidgetController(Settings& settings, GraphPtr graph, NodeFactory* node_factory, NodeAdapterFactory* node_adapter_factory);

    void startPlacingBox(QWidget *parent, const std::string& type, NodeStatePtr state, const QPoint &offset = QPoint(0,0));

    NodeBox* getBox(const UUID& node_id);

    MovableGraphicsProxyWidget* getProxy(const UUID& node_id);

    Port* getPort(const UUID& connector_id);
    Port* getPort(const Connectable* connector_id);

    GraphPtr getGraph();
    NodeFactory* getNodeFactory();

    void setDesigner(Designer* designer);

    CommandDispatcher* getCommandDispatcher() const;
    void setCommandDispatcher(CommandDispatcher *dispatcher);

    void foreachBox(boost::function<void (NodeBox*)> f, boost::function<bool (NodeBox*)> pred);

    void setStyleSheet(const QString &str);


    void insertAvailableNodeTypes(QMenu* menu);
    void insertAvailableNodeTypes(QTreeWidget *tree);
    QAbstractItemModel *listAvailableNodeTypes();    

    void insertPort(QLayout* layout, Port* port);

public Q_SLOTS:
    void nodeAdded(NodeWorkerPtr node_worker);
    void nodeRemoved(NodeWorkerPtr node_worker);

    void connectorCreated(Connectable *connector);
    void connectorRemoved(Connectable *connector);

private:
    void createPort(Connectable* connector, NodeBox *box, QBoxLayout *layout);

    void connectorSignalAdded(Connectable *connector);
    void connectorSignalRemoved(Connectable *connector);

    void connectorMessageAdded(Connectable *connector);
    void connectorMessageRemoved(Connectable *connector);

private:
    GraphPtr graph_;
    CommandDispatcher* dispatcher_;
    Settings& settings_;
    NodeFactory* node_factory_;
    NodeAdapterFactory* node_adapter_factory_;
    Designer* designer_;

    boost::unordered_map<UUID, NodeBox*, UUID::Hasher> box_map_;
    boost::unordered_map<UUID, MovableGraphicsProxyWidget*, UUID::Hasher> proxy_map_;
    boost::unordered_map<UUID, Port*, UUID::Hasher> port_map_;

    QString style_sheet_;
};

}

#endif // WIDGET_CONTROLLER_H
