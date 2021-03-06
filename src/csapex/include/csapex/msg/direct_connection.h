#ifndef DIRECT_CONNECTION_H
#define DIRECT_CONNECTION_H

/// PROJECT
#include <csapex/model/connection.h>

namespace csapex
{

class CSAPEX_EXPORT DirectConnection : public Connection
{
public:
    static ConnectionPtr connect(Output* from, Input* to);
    static ConnectionPtr connect(Output* from, Input* to, int id);

public:
    ~DirectConnection();

    virtual void setToken(const TokenPtr& msg) override;

protected:
    DirectConnection(Output* from, Input* to);
    DirectConnection(Output* from, Input* to, int id);

};

}

#endif // DIRECT_CONNECTION_H

