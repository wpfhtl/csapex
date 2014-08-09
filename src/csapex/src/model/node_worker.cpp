/// HEADER
#include <csapex/model/node_worker.h>

/// COMPONENT
#include <csapex/model/node.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/utility/timer.h>
#include <csapex/utility/thread.h>
#include <csapex/core/settings.h>
#include <csapex/model/node_state.h>

/// SYSTEM
#include <QThread>

using namespace csapex;

NodeWorker::NodeWorker(Node* node)
    : node_(node), private_thread_(NULL),
      timer_history_pos_(-1),
      thread_initialized_(false), paused_(false), stop_(false)
{
    timer_history_.resize(Settings::timer_history_length_);
    apex_assert_hard(timer_history_.size() == Settings::timer_history_length_);
    apex_assert_hard(timer_history_.capacity() == Settings::timer_history_length_);

    apex_assert_hard(node_);

    tick_timer_ = new QTimer();
    setTickFrequency(DEFAULT_FREQUENCY);

    QObject::connect(tick_timer_, SIGNAL(timeout()), this, SLOT(tick()));
}

NodeWorker::~NodeWorker()
{
    while(!node_->inputs_.empty()) {
        node_->removeInput(*node_->inputs_.begin());
    }
    while(!node_->outputs_.empty()) {
        node_->removeOutput(*node_->outputs_.begin());
    }
    while(!node_->managed_inputs_.empty()) {
        node_->removeInput(*node_->managed_inputs_.begin());
    }
    while(!node_->managed_outputs_.empty()) {
        node_->removeOutput(*node_->managed_outputs_.begin());
    }
}

bool NodeWorker::isEnabled() const
{
    return node_->node_state_->isEnabled();
}

void NodeWorker::setEnabled(bool e)
{
    node_->node_state_->setEnabled(e);

    if(!e) {
        node_->setError(false);
    }

    checkIO();
    Q_EMIT enabled(e);
}

bool NodeWorker::canReceive()
{
    bool can_receive = true;
    Q_FOREACH(Input* i, node_->inputs_) {
        if(!i->isConnected() && !i->isOptional()) {
            can_receive = false;
        } else if(i->isConnected() && !i->getSource()->isEnabled()) {
            can_receive = false;
        }
    }

    return can_receive;
}


void NodeWorker::makeThread()
{
    if(!private_thread_) {
        private_thread_ = new QThread;
        connect(private_thread_, SIGNAL(finished()), private_thread_, SLOT(deleteLater()));

        moveToThread(private_thread_);

        private_thread_->start();
    }
}

void NodeWorker::connectConnector(Connectable *c)
{
    QObject::connect(c, SIGNAL(connectionInProgress(Connectable*,Connectable*)), this, SIGNAL(connectionInProgress(Connectable*,Connectable*)));
    QObject::connect(c, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
    QObject::connect(c, SIGNAL(connectionDone()), this, SIGNAL(connectionDone()));
    QObject::connect(c, SIGNAL(connectionDone()), this, SLOT(checkIO()));
    QObject::connect(c, SIGNAL(connectionEnabled(bool)), this, SLOT(checkIO()));
    QObject::connect(c, SIGNAL(connectionRemoved()), this, SLOT(checkIO()));
}


void NodeWorker::disconnectConnector(Connectable */*c*/)
{
}


void NodeWorker::stop()
{
    QMutexLocker lock(&stop_mutex_);


    Q_FOREACH(Input* i, node_->inputs_) {
        i->free();
    }
    Q_FOREACH(Output* i, node_->outputs_) {
        i->stop();
    }
    Q_FOREACH(Input* i, node_->inputs_) {
        i->stop();
    }

    Q_FOREACH(Input* i, node_->inputs_) {
        disconnectConnector(i);
    }
    Q_FOREACH(Output* i, node_->outputs_) {
        disconnectConnector(i);
    }

    QObject::disconnect(private_thread_);
    stop_ = true;

    pause(false);

    if(private_thread_) {
        private_thread_->quit();
        //        private_thread_->wait(1000);
        //        if(private_thread_->isRunning()) {
        //            std::cout << "terminate thread" << std::endl;
        //            private_thread_->terminate();
        //        }
    }

}

void NodeWorker::pause(bool pause)
{
    QMutexLocker lock(&pause_mutex_);
    paused_ = pause;
    continue_.wakeAll();
}

void NodeWorker::killExecution()
{
    // TODO: implement
}

void NodeWorker::forwardMessage(Connectable *s)
{
    {
        pause_mutex_.lock();
        while(paused_) {
            continue_.wait(&pause_mutex_);
        }
        pause_mutex_.unlock();
    }

    QMutexLocker lock(&stop_mutex_);
    if(stop_) {
        return;
    }

    Input* source = dynamic_cast<Input*> (s);
    apex_assert_hard(source);

    if(isEnabled()) {
        Q_EMIT messagesReceived();

        forwardMessageSynchronized(source);
    }
}

void NodeWorker::addInput(Input *source)
{
    clearInput(source);

    connect(source, SIGNAL(enabled(bool)), this, SLOT(checkInputs()));
}


void NodeWorker::manageInput(Input* in)
{
    node_->managed_inputs_.push_back(in);
    connectConnector(in);
    in->moveToThread(thread());
}

void NodeWorker::manageOutput(Output* out)
{
    node_->managed_outputs_.push_back(out);
    connectConnector(out);
    out->moveToThread(thread());
}


void NodeWorker::registerInput(Input* in)
{
    node_->inputs_.push_back(in);
    in->setCommandDispatcher(node_->dispatcher_);

    in->moveToThread(thread());

    addInput(in);
    connectConnector(in);
    QObject::connect(in, SIGNAL(messageArrived(Connectable*)), this, SLOT(forwardMessage(Connectable*)));

    Q_EMIT connectorCreated(in);
}

void NodeWorker::registerOutput(Output* out)
{
    node_->outputs_.push_back(out);
    out->setCommandDispatcher(node_->dispatcher_);

    out->moveToThread(thread());

    connectConnector(out);

    Q_EMIT connectorCreated(out);
}

void NodeWorker::checkInputs()
{
    for(int i = 0; i < node_->countInputs(); ++i) {
        Input* source = node_->getInput(i);
        if(!source->isEnabled() && source->isBlocked()) {
            source->free();
            clearInput(source);
        }
    }
}

void NodeWorker::clearInput(Input *source)
{
    QMutexLocker lock(&message_mutex_);
    has_msg_[source] = false;
}

void NodeWorker::removeInput(Input *source)
{
    QMutexLocker lock(&message_mutex_);
    std::map<Input*, bool>::iterator it = has_msg_.find(source);

    if(it != has_msg_.end()) {
        has_msg_.erase(it);
    }
}

void NodeWorker::forwardMessageSynchronized(Input *source)
{
    typedef std::pair<Input*, bool> PAIR;
    bool can_process = true;

    {
        QMutexLocker lock(&message_mutex_);
        std::vector< boost::shared_ptr<QMutexLocker> > locks;

        // forbid async to change
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;
            locks.push_back(cin->lockAsync());
        }

        //apex_assert_hard(!has_msg_[source]);
        if(has_msg_[source] && !source->isAsync()) {
            std::cerr << "input @" << source->getUUID().getFullName() <<
                         ": assertion failed: !has_msg_[" << source->getUUID().getFullName() << "]" << std::endl;
            apex_assert_hard(false);
        }
        if(!source->isAsync()) {
            has_msg_[source] = true;
        }

        // check if all inputs have messages
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;
            if(!has_msg_[cin]) {
                // connector doesn't have a message
                if(cin->isAsync()) {
                    // do not wait for this input
                } else if(cin->isOptional()) {
                    if(cin->isConnected()) {
                        // c is optional and connected, so we have to wait for a message
                        return;
                    } else {
                        // c is optional and not connected, so we can proceed
                        /* do nothing */
                    }
                } else {
                    // c is mandatory, so we have to wait for a message
                    return;
                }
            }
        }

        // everything has a message here

        // check for old messages
        bool had_old_message = false;
        int highest_seq_no = -1;
        UUID highest = UUID::NONE;
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;

            if(has_msg_[cin] && !cin->isAsync()) {
                int s = cin->sequenceNumber();
                if(s > highest_seq_no) {
                    highest_seq_no = s;
                    highest = cin->getUUID();
                }
            }
        }
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;
            if(cin == source) {
                continue;
            }

            if(has_msg_[cin] && !cin->isAsync() && cin->sequenceNumber() != highest_seq_no) {
                std::cerr << "input @" << source->getUUID().getFullName() <<
                             ": dropping old message @" << cin->getUUID().getFullName() << " with #" << cin->sequenceNumber() <<
                             " < #" << highest_seq_no << " @" << highest.getFullName() << std::endl;
                has_msg_[cin] = false;
                had_old_message = true;
                cin->free();
            }
        }

        // if a message was dropped we can already return
        if(had_old_message) {
            return;
        }

        // now all sequence numbers must be equal!
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;

            if(has_msg_[cin] && !cin->isAsync()) {
                if(highest_seq_no != cin->sequenceNumber()) {
                    std::cerr << "input @" << cin->getUUID().getFullName() <<
                                 ": assertion failed: highest_seq_no (" << highest_seq_no << ") == cin->seq_no (" << cin->sequenceNumber() << ")" << std::endl;
                    apex_assert_hard(false);
                }
            }
        }

        // check if one is "NoMessage"
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;

            if(has_msg_[cin] && !cin->isAsync()) {
                if(cin->isMessage<connection_types::NoMessage>()) {
                    can_process = false;
                }
            }
        }

        // set output sequence numbers
        for(int i = 0; i < node_->countOutputs(); ++i) {
            Output* out = node_->getOutput(i);
            out->setSequenceNumber(highest_seq_no);
        }

        // reset states
        Q_FOREACH(const PAIR& pair, has_msg_) {
            Input* cin = pair.first;

            if(has_msg_[cin] && !cin->isAsync()) {
                has_msg_[cin] = false;
            }
        }
    }

    Timer::Ptr t(new Timer(node_->getUUID()));
    node_->useTimer(t.get());
    if(can_process){
        //boost::shared_ptr<QMutexLocker> lock = node_->getParamLock();
        try {
            node_->process();

        }  catch(const std::exception& e) {
            node_->setError(true, e.what());
        } catch(const std::string& s) {
            node_->setError(true, "Uncatched exception (string) exception: " + s);
        } catch(...) {
            throw;
        }

    }
    t->finish();

    // send the messages
    sendMessages();

    if(++timer_history_pos_ >= (int) Settings::timer_history_length_) {
        timer_history_pos_ = 0;
    }
    timer_history_[timer_history_pos_] = t;

    // reset all edges
    Q_FOREACH(const PAIR& pair, has_msg_) {
        Input* cin = pair.first;
        cin->free();
    }

    Q_EMIT messageProcessed();
}

void NodeWorker::sendMessages()
{
    for(int i = 0; i < node_->countOutputs(); ++i) {
        Output* out = node_->getOutput(i);
        out->sendMessages();
    }
}

void NodeWorker::setTickFrequency(double f)
{
    if(tick_timer_->isActive()) {
        tick_timer_->stop();
    }
    if(f == 0.0) {
        return;
    }
    tick_timer_->setInterval(1000. / f);
    tick_timer_->setSingleShot(false);
    tick_timer_->start();
}

void NodeWorker::tick()
{
    {
        pause_mutex_.lock();
        while(paused_) {
            continue_.wait(&pause_mutex_);
        }
        pause_mutex_.unlock();

        QMutexLocker lock(&stop_mutex_);
        if(stop_) {
            return;
        }
    }


    if(!thread_initialized_) {
        thread::set_name(node_->getUUID().getShortName().c_str());
        thread_initialized_ = true;
    }

    if(isEnabled()) {
        //boost::shared_ptr<QMutexLocker> lock = node_->getParamLock();
        node_->tick();

        // if there is a message: send!
        bool has_msg = false;
        for(int i = 0; i < node_->countOutputs(); ++i) {
            Output* out = node_->getOutput(i);
            if(out->hasMessage()) {
                has_msg = true;
            }
        }
        if(has_msg) {
            sendMessages();
        }
    }
}

void NodeWorker::checkParameters()
{
    Parameterizable::ChangedParameterList changed_params = node_->getChangedParameters();

    if(!changed_params.empty()) {
        for(Parameterizable::ChangedParameterList::iterator it = changed_params.begin(); it != changed_params.end();) {
            try {
                it->second(it->first);

            } catch(const std::exception& e) {
                it = changed_params.erase(it);
                throw;
            }

            it = changed_params.erase(it);
        }
    }
}

void NodeWorker::checkIO()
{
    if(isEnabled()) {
        enableInput(canReceive());
        enableOutput(canReceive());
    } else {
        enableInput(false);
        enableOutput(false);
    }
}


void NodeWorker::enableIO(bool enable)
{
    enableInput(canReceive() && enable);
    enableOutput(enable);
}

void NodeWorker::enableInput (bool enable)
{
    Q_FOREACH(Input* i, node_->inputs_) {
        if(enable) {
            i->enable();
        } else {
            i->disable();
        }
    }
}


void NodeWorker::enableOutput (bool enable)
{
    Q_FOREACH(Output* o, node_->outputs_) {
        if(enable) {
            o->enable();
        } else {
            o->disable();
        }
    }
}

void NodeWorker::triggerError(bool e, const std::string &what)
{
    node_->setError(e, what);
}



void NodeWorker::setIOError(bool error)
{
    Q_FOREACH(Input* i, node_->inputs_) {
        i->setErrorSilent(error);
    }
    Q_FOREACH(Output* i, node_->outputs_) {
        i->setErrorSilent(error);
    }
    enableIO(!error);
}

void NodeWorker::setMinimized(bool min)
{
    node_->node_state_->setMinimized(min);
}
