#ifndef serialWatcher_H
#define serialWatcher_H

#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include "qdebug.h"
#include <QSerialPort>
#include "qtimer.h"

class serialWatcher
      : public QObject
{
    Q_OBJECT



public:
    explicit serialWatcher(QObject *parent = nullptr);
    ~serialWatcher();
private:
    QSerialPort *port;


private slots:
    void readData();


signals:
    void clicked();



};

#endif // serialWatcher_H
