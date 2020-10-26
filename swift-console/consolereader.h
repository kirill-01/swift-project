#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include <QThread>
#include "../swift-corelib/wampclient.h"

class ConsoleReader : public QThread
{
    Q_OBJECT
signals:
    void KeyPressed(char ch);
    void callRpcCommand( const QString& cmd, const QVariantList& list = QVariantList() );
public:
   ConsoleReader();
   ~ConsoleReader();
   void run();
public slots:
   void onWampSession( Wamp::Session * session );
};

#endif  /* CONSOLEREADER_H */
