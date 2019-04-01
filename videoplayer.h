#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "qobject.h"
#include "qlabel.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qthread.h"

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <astra/astra.hpp>
/*#include <astra/astra.hpp>
#include "LitDepthVisualizer.hpp"
#include <key_handler.h>
*/


#include <chrono>
#include <iostream>
#include <iomanip>

#include <sstream>
#include "qmutex.h"

using namespace std;
using namespace cv;
class videoWorker;

class videoplayer : public QLabel
{
    Q_OBJECT
public:
    videoplayer(QWidget *parent = 0, QString PATH="", std::vector<int> parameters=std::vector<int>());
private:
    QString PATH;
    QThread *videoThread;
    videoWorker *worker;

private slots:
    void getNuFrame(QPixmap frame);

signals:
    void initWorker();
    void startShortVideo();
};




class videoWorker : public QObject
{
    Q_OBJECT
public:
    videoWorker(QWidget *parent = 0, QString PATH="", std::vector<int> parameters=std::vector<int>());

private:
    QString PATH;
    std::vector<int> parameters;
    QTimer *processTimer;
    QPixmap buf;
    cv::VideoCapture *video,*orbbec;
    uint videoFrameCount;
    QMutex loadMutex;
    bool shortVideo;
    astra::StreamReader reader;
     astra::StreamSet streamSet;
     bool showShape;
     Size image_size;


signals:
    void frameReady(QPixmap frame);

private slots:
    void startShortVideo();
    void init();
    void processNextFrame();
    void loadLoopVideo();

};


#endif // VIDEOPLAYER_H
