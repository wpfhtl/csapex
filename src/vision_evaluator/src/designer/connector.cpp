/// HEADER
#include "connector.h"

/// COMPONENT
#include "design_board.h"
#include "box.h"
#include "box_manager.h"
#include "command_add_connection.h"

/// SYSTEM
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <QDragEnterEvent>

using namespace vision_evaluator;

const QString Connector::MIME = "vision_evaluator/connector";

Connector::Connector(Box *parent, int sub_id)
    : QRadioButton(parent), parent_widget(parent), box_(parent), designer(NULL)
{
    findParents();
    setFocusPolicy(Qt::NoFocus);
    setAcceptDrops(true);

    std::stringstream ss;
    ss << box_->UUID() << "_" << sub_id;

    uuid_ = ss.str();
}

Connector::~Connector()
{
}

std::string Connector::UUID()
{
    return uuid_;
}

void Connector::setUUID(const std::string &uuid)
{
    uuid_ = uuid;
}

bool Connector::tryConnect(QObject* other_side)
{
    Connector* c = dynamic_cast<Connector*>(other_side);
    if(c) {
        return tryConnect(c);
    }
    return false;
}

void Connector::removeConnection(QObject* other_side)
{
    Connector* c = dynamic_cast<Connector*>(other_side);
    if(c) {
        removeConnection(c);
    }
}

void Connector::findParents()
{
    QWidget* tmp = this;
    while(tmp != NULL) {
        if(dynamic_cast<vision_evaluator::Box*>(tmp)) {
            box_ = dynamic_cast<vision_evaluator::Box*>(tmp);
        } else if(dynamic_cast<vision_evaluator::DesignBoard*>(tmp)) {
            designer = dynamic_cast<vision_evaluator::DesignBoard*>(tmp);
        }
        tmp = tmp->parentWidget();
    }
}

bool Connector::hitButton(const QPoint&) const
{
    return false;
}

bool Connector::canConnectTo(Connector* other_side)
{
    return (isOutput() && other_side->isInput()) || (isInput() && other_side->isOutput());
}

void Connector::dragEnterEvent(QDragEnterEvent* e)
{
    if(e->mimeData()->text() == Connector::MIME) {
        Connector* from = dynamic_cast<Connector*>(e->mimeData()->parent());
        if(from->canConnectTo(this) && canConnectTo(from)) {
            e->acceptProposedAction();
        }
    }
}

void Connector::dragMoveEvent(QDragMoveEvent* e)
{
    if(e->mimeData()->text() == Connector::MIME) {
        Connector* from = dynamic_cast<Connector*>(e->mimeData()->parent());
        overlay_->drawTemporaryLine(QLine(from->centerPoint(), centerPoint()));
    }
}

void Connector::dropEvent(QDropEvent* e)
{
    if(e->mimeData()->text() == Connector::MIME) {
        Connector* from = dynamic_cast<Connector*>(e->mimeData()->parent());

        if(from && from != this) {
            Command::Ptr cmd(new command::AddConnection(overlay_, this, from, designer));
            BoxManager::instance().execute(cmd);
        }
    }
}

void Connector::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setText(Connector::MIME);
        mimeData->setParent(this);
        drag->setMimeData(mimeData);

        drag->exec();

        overlay_->deleteTemporaryLine();
    }
}

void Connector::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::MiddleButton) {
        removeAllConnections();
    }
}

QPoint Connector::topLeft()
{
    if(box_ == NULL) {
        findParents();
    }

    return box_->geometry().topLeft() + pos();
}

QPoint Connector::centerPoint()
{
    return topLeft() + 0.5 * (geometry().bottomRight() - geometry().topLeft());
}

void Connector::paintEvent(QPaintEvent* e)
{
    setAutoExclusive(false);
    setChecked(isConnected());

    QRadioButton::paintEvent(e);
}

vision_evaluator::Box* Connector::box()
{
    return box_;
}