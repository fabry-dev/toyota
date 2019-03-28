#include "serialwatcher.h"

#include <fcntl.h>
#include <unistd.h>



serialWatcher::serialWatcher(QObject *parent):QObject(parent)
{



    port = new QSerialPort;
    port->setPortName("/dev/ttyACM0");


    if(port->open(QIODevice::ReadWrite))
        qDebug()<<"serial created";
    else
        qDebug()<<"serial error";


    port->setBaudRate(QSerialPort::Baud9600)

            && port->setDataBits(QSerialPort::Data8)

            && port->setParity(QSerialPort::NoParity)

            && port->setStopBits(QSerialPort::OneStop)

            && port->setFlowControl(QSerialPort::NoFlowControl);




    connect(port, SIGNAL(readyRead()), this, SLOT(readData()));

}





void serialWatcher::readData()
{

    const QByteArray data = port->readAll();
    uchar b = data.at(0);

    if(b==10)
        emit clicked();

}




serialWatcher::~serialWatcher()
{
    qDebug()<<"closed";

}


