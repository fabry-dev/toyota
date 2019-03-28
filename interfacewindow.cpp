#include "interfacewindow.h"

interfaceWindow::interfaceWindow(QWidget *parent,QString(PATH),std::vector<int>parameters) : QWidget(parent),PATH(PATH)
{

    sw = new serialWatcher(this);

    vp = new videoplayer(this,PATH,parameters);
    vp->resize(size());
    vp->show();

    connect(sw,SIGNAL(clicked()),vp,SIGNAL(startShortVideo()));

}
void interfaceWindow::resizeEvent(QResizeEvent* event)
{
    vp->resize(size());
    qDebug()<<size();
}
