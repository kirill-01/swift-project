#include "consolereader.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

static struct termios oldSettings;
static struct termios newSettings;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &oldSettings); /* grab old terminal i/o settings */
  newSettings = oldSettings; /* make new settings same as old settings */
  newSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
  newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &oldSettings);
}

/* Read 1 character without echo */
char getch(void)
{
  return getchar();
}

ConsoleReader::ConsoleReader()
{
  initTermios(0);
}

ConsoleReader::~ConsoleReader()
{
  resetTermios();
}

void ConsoleReader::run()
{
    forever
    {
        char key = getchar();
        int kcode = key;
        if ( kcode == 91 ) {
            key = getchar();
            key = getchar();
        }
        emit KeyPressed(key);
    }
}

void ConsoleReader::onWampSession(Wamp::Session *session) {
    connect( this, &ConsoleReader::callRpcCommand, [=]( const QString& cmd, const QVariantList& list ){
        session->call(cmd, list);
    });
}
