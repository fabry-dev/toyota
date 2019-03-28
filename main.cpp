#include "interfacewindow.h"
#include <QApplication>
#include "qfile.h"
#include "qdebug.h"
#include "qscreen.h"
#include "qpushbutton.h"
#define PATH_DEFAULT "/home/fred/Dropbox/Taf/Cassiopee/toyota/files/"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString PATH;
    QStringList params = a.arguments();

    if(params.size()>1)
        PATH = params[1];
    else
        PATH=PATH_DEFAULT;

    bool HIDE_CURSOR=false;
    bool DEBUG=false;
    int distance_min,distance_max,offset_x,offset_y;


    QFile file(PATH+"config.cfg");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"no config file";
    }
    else
    {
        QTextStream in(&file);

        QString  line;
        QString paramName,paramValue;
        QStringList params;

        while(!in.atEnd()) {
            line = in.readLine();
            line = (line.split("#"))[0];
            params = line.split("=");
            if(params.size()>=2)
            {
                paramName = params[0];
                paramValue = params[1];

                if (paramName.mid(0,6)=="CURSOR")
                    HIDE_CURSOR = (paramValue=="NO");
                else if (paramName=="DEBUG")
                    DEBUG = (paramValue=="YES");
                else if (paramName=="DISTANCE_MIN")
                    distance_min = paramValue.toInt();
                else if (paramName=="DISTANCE_MAX")
                    distance_max = paramValue.toInt();
                else if (paramName=="OFFSET_X")
                    offset_x = paramValue.toInt();
                else if (paramName=="OFFSET_Y")
                    offset_y = paramValue.toInt();
                else
                    qDebug()<<paramName<<" - "<<paramValue;




            }
        }
        file.close();
    }

    if (HIDE_CURSOR)
    {
        QCursor cursor(Qt::BlankCursor);
        a.setOverrideCursor(cursor);
        a.changeOverrideCursor(cursor);
    }



    int desiredScreen = 1;



    qDebug()<<"screen count "<<a.screens().size();

    if(desiredScreen>a.screens().size()-1)
        desiredScreen = a.screens().size()-1;

    QScreen* screen1 = a.screens().at(desiredScreen);


    std::vector<int> parameters;
    parameters.push_back(distance_min);
    parameters.push_back(distance_max);
    parameters.push_back(offset_x);
    parameters.push_back(offset_y);


    interfaceWindow *iw = new interfaceWindow(NULL,PATH,parameters);
    iw->setGeometry(screen1->geometry().x(),screen1->geometry().y(),1344,672);
    //iw->showFullScreen();
    iw->show();



    if(DEBUG)
    {
        QPushButton *pb0 = new QPushButton(iw);
        pb0->move(0,0);
        pb0->resize(200,200);
        pb0->setText("close app");
        a.connect(pb0,SIGNAL(clicked(bool)),&a,SLOT(closeAllWindows()));
        pb0->show();
        pb0->raise();
    }


    return a.exec();
}
