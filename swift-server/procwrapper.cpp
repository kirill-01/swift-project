#include "procwrapper.h"


ProcWrapper::ProcWrapper(const QString &command, const QStringList &args, QObject *parent) : QObject(parent), cmd(command), arg(args), was_started(false), restarts_count(0) {
    process_name = command.split("/").last();
    QTimer::singleShot( 2000, this, &ProcWrapper::start );
}


QString ProcWrapper::getStatus() {
    if ( !_proc.isNull() && _proc->isReadable() ) {
        return _proc->readAllStandardOutput();
    } else {
        return "Not running";
    }
}

void ProcWrapper::stop() {
    blockSignals(true);
    _proc->kill();
}

void ProcWrapper::restart() {
    blockSignals(true);
    _proc->kill();
    blockSignals(false);
    QTimer::singleShot( 5000, this, &ProcWrapper::start );
}

void ProcWrapper::onProcFinished(const quint32 &exitcode) {
    if ( exitcode == 0 ) {
        qInfo() << "Process is normal exited";
    } else {
        qInfo() << "Process exited becouse error";   
        if ( restarts_count >= 150 ) {
            qWarning() << "Maximum restarts count reached -> trying to kill all processes";
        } else {
            qInfo() << process_name << "Proccess waiting for restart ";
            QTimer::singleShot( 5000, this, &ProcWrapper::start );
            restarts_count++;
        }
     }
}

void ProcWrapper::start() {
    _proc.reset( new QProcess() );
    connect( _proc.get(), &QProcess::stateChanged, this, &ProcWrapper::onStateChanged );
    connect( this, &ProcWrapper::procFinished, this, &ProcWrapper::onProcFinished );
    _proc->setProcessChannelMode( QProcess::ForwardedChannels );
    _proc->setProgram( cmd );
    _proc->setArguments( arg );
    _proc->start();
}

void ProcWrapper::onStateChanged(QProcess::ProcessState state) {
    if ( state == QProcess::Running ) {
        was_started = true;
    } else if ( state == QProcess::NotRunning ) {
        if ( was_started ) {
            was_started = false;
            qInfo() << process_name << "Proccess finished with exit code " << _proc->exitCode();
            emit procFinished( _proc->exitCode() );
        }
    }
}
