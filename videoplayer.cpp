#include "videoplayer.h"
#include <astra/streams/Image.hpp>

#define VIDEO_FPS 24
#define DMIN parameters[0]
#define DMAX parameters[1]
#define BLUR_BAND ((DMAX-DMIN)/10)

#define OFFSET_X parameters[2]
#define OFFSET_Y parameters[3]



videoplayer::videoplayer(QWidget *parent, QString PATH,std::vector<int>parameters):QLabel(parent),PATH(PATH)
{

    videoThread = new QThread;
    worker = new videoWorker(NULL,PATH,parameters);
    worker->moveToThread(videoThread);
    connect(worker,SIGNAL(frameReady(QPixmap)),this,SLOT(getNuFrame(QPixmap)));
    connect(videoThread,SIGNAL(started()),worker,SLOT(init()),Qt::QueuedConnection);
    connect(this,SIGNAL(startShortVideo()),worker,SLOT(startShortVideo()));
    videoThread->start();

}


void videoplayer::getNuFrame(QPixmap frame)
{
    //qDebug()<<"frame";
    setPixmap(frame.scaledToWidth(width()));
}



videoWorker::videoWorker(QWidget *parent, QString PATH, std::vector<int> parameters):QObject(parent),PATH(PATH),parameters(parameters)
{
    qDebug()<<"DISTANCE_MIN: "<<parameters[0];
    qDebug()<<"DISTANCE_MAX: "<<parameters[1];
    qDebug()<<"OFFSET_X: "<<parameters[2];
    qDebug()<<"OFFSET_Y: "<<parameters[3];

}


void videoWorker::init()
{

    float ratio = 0.75;

    int w = (int)896*ratio;
    int h =(int)672*ratio;

    image_size = Size(w,h);


    astra::initialize();
    reader = streamSet.create_reader();
    reader.stream<astra::DepthStream>().start();
    processTimer = new QTimer();
    processTimer->setInterval(1000/VIDEO_FPS);
    connect(processTimer,SIGNAL(timeout()),this,SLOT(processNextFrame()));
    video = new VideoCapture();
    video->release();
    loadMutex.unlock();
    loadLoopVideo();
    processTimer->start();
}





void videoWorker::loadLoopVideo()
{
    showShape = false;
    shortVideo = false;
    loadMutex.lock();
    video->release();
    QString videoName = PATH+"background.mp4";
    qDebug()<<videoName;
    video->open(videoName.toStdString().c_str());
    videoFrameCount = 0;
    loadMutex.unlock();
}

void videoWorker::startShortVideo()
{

    if(shortVideo)
        return;

    showShape = false;
    shortVideo = true;
    loadMutex.lock();
    video->release();

    QString videoName = PATH+"video.mp4";
    video->open(videoName.toStdString().c_str());
    videoFrameCount = 0;
    loadMutex.unlock();
}

void videoWorker::processNextFrame()
{
    Mat videoFrame;
    uint videoFrameCount2;

begin:
    loadMutex.lock();
    if(video->isOpened())
    {
        (*video) >> videoFrame;
        videoFrameCount2 = video->get(CAP_PROP_POS_FRAMES);

        if(videoFrameCount2==videoFrameCount) //auto loop
        {
            if(shortVideo)
            {
                loadMutex.unlock();
                loadLoopVideo();
                goto begin;
            }

            video->set(CAP_PROP_POS_FRAMES,0);
            (*video) >> videoFrame;
            videoFrameCount2 = video->get(CAP_PROP_POS_FRAMES);
        }

        videoFrameCount = videoFrameCount2;

        if((videoFrameCount>220)&&(!showShape))
            showShape = true;

        //   qDebug()<<videoFrameCount;
        loadMutex.unlock();

        //add the orbbec frame
        astra::Frame frame = reader.get_latest_frame(5000);
        const auto depthFrame = frame.get<astra::DepthFrame>();

        int h = depthFrame.height();
        int w = depthFrame.width();


        Mat image(h,w, CV_32F);

        for (int y = 0; y < depthFrame.height(); y++)
        {
            for (int x = 0; x < depthFrame.width(); x++)
            {
                uint d = depthFrame.data()[x+y*depthFrame.width()];

                if((d<DMIN) || (d>DMAX))
                    image.at<float>(y, x)= 0;
                else if((d-DMIN)<BLUR_BAND)
                {
                    float f = (float)(d-DMIN)/BLUR_BAND;
                    image.at<float>(y, x)= f;
                }
                else if(abs((int)DMAX-d)<BLUR_BAND)
                {
                    float f = (float)abs((int)DMAX-d)/BLUR_BAND;
                    image.at<float>(y, x)= f;
                }
                else
                    image.at<float>(y, x)= 1;
            }
        }


        flip(image, image, +1);

        resize(image,image,image_size);//resize image

        medianBlur ( image,image, 5 );


        //B G R
        cv::Vec3b green = cv::Vec3b(0x33,0xFF,0x30);
        cv::Vec3b blue = cv::Vec3b(0xD5,0x9A,0x19);

        cv::Vec3b red = cv::Vec3b(0x27,0x15,0xCD);
        cv::Vec3b empty = cv::Vec3b(0,0,0);
        cv::Vec3b white = cv::Vec3b(255,255,255);
        int offset = 25;

        int X,Y;

        h = image.rows;
        w = image.cols;
        if(((showShape)&&(shortVideo)||false))
        {
            for(int x=0;x<image.cols;x++)
                for(int y=0;y<image.rows;y++)
                {

                    X = x+OFFSET_X;
                    Y = y+OFFSET_Y;

                    if((X<0)||(Y<0)||(X>=videoFrame.cols)||(Y>=videoFrame.rows))
                    {
                        //  qDebug()<<"bug";
                        break;
                    }

                    float a;
                    if((x<w)&&(y<h))
                    {
                        a = image.at<float>(Point(x,y));
                        if(a > 0)
                        {
                            videoFrame.at<Vec3b>(Point(X,Y)) += red*a;
                        }
                    }

                    if((x+offset<w)&&(y+offset<h))
                    {

                        a = image.at<float>(Point(x+offset,y+offset));
                        if(a > 0)
                        {
                            videoFrame.at<Vec3b>(Point(X,Y)) += green*a;
                        }
                    }

                    if((x-offset>=0)&&(y-offset>=0))
                    {

                        a = image.at<float>(Point(x-offset,y-offset));
                        if(a > 0)
                        {
                            videoFrame.at<Vec3b>(Point(X,Y)) += blue*a;
                        }
                    }




                }


        }






        cvtColor(videoFrame,videoFrame, COLOR_BGR2RGB);

        emit frameReady(QPixmap::fromImage(QImage(videoFrame.data, videoFrame.cols, videoFrame.rows, videoFrame.step, QImage::Format_RGB888)));

    }


    loadMutex.unlock();//just in case


}














/*
 *
 *

    VideoCapture cap(1); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        exit(1);

    Mat rgb;
    int count = 0;

    do {
        cap >> rgb; // get a new frame from camera
        astra::Frame frame = reader.get_latest_frame();
        const auto depthFrame = frame.get<astra::DepthFrame>();
        // Mat RGB(colorFrame.height(),colorFrame.width(), CV_8UC3);

        int h = depthFrame.height();
        int w = depthFrame.width();


        Mat image(h,w, CV_8UC1);
        Mat large(h*2,w*2, CV_8UC3);


        for (int y = 0; y < depthFrame.height(); y++)
        {
            for (int x = 0; x < depthFrame.width(); x++)
            {
                uint d = depthFrame.data()[x+y*depthFrame.width()];

                if((d>DMIN) && (d<DMAX) )
                    image.at<uchar>(y, x)= 0;
                else
                    image.at<uchar>(y, x)= 255;




            }
        }

        int dilation_size = 2;

        Mat element = getStructuringElement( MORPH_ELLIPSE,Size( 2*dilation_size + 1, 2*dilation_size+1 ),Point( dilation_size, dilation_size ) );


        //cv::erode(image,image,element);
        // medianBlur(image, image,3);


        Mat result(h,w, CV_8UC3);

        cv::Vec3b green = cv::Vec3b(0,255,0);
        cv::Vec3b blue = cv::Vec3b(255,0,100);
        cv::Vec3b red = cv::Vec3b(0,0,255);
        cv::Vec3b empty = cv::Vec3b(0,0,0);
        cv::Vec3b white = cv::Vec3b(255,255,255);
        int offset = 20;
        bool test = false;


        for(int x=0;x<result.cols;x++)
            for(int y=0;y<result.rows;y++)
            {

                test =false;

                result.at<Vec3b>(Point(x,y)) = empty;

                if((image.at<uchar>(Point(x,y))) == 0)
                {
                    test = true;
                    result.at<Vec3b>(Point(x,y)) += red;
                }

                if((x+offset<w)&&(y+offset<h)&&(image.at<uchar>(Point(x+offset,y+offset)) == 0))
                {
                    test = true;
                    result.at<Vec3b>(Point(x,y)) += green;
                }
                if((x+2*offset<w)&&(y+2*offset<h)&&(image.at<uchar>(Point(x+2*offset,y+2*offset)) == 0))
                {
                    test = true;
                    result.at<Vec3b>(Point(x,y)) += blue;
                }

                //  if(!test)
                //   result.at<Vec3b>(Point(x,y)) = white;


            }


        cv::resize(result,large, large.size(), 0, 0);

        imshow("a",large);

        waitKey(10);

        count++;
    } while (true);






























class DepthFrameListener : public astra::FrameListener
{
public:
    DepthFrameListener(const astra::CoordinateMapper& coordinateMapper)
        : coordinateMapper_(coordinateMapper)
    {


    }



    void on_frame_ready(astra::StreamReader& reader,
                        astra::Frame& frame) override
    {
        const astra::PointFrame pointFrame = frame.get<astra::PointFrame>();

        const int width = pointFrame.width();
        const int height = pointFrame.height();


        qDebug()<<"frame";
        if (isPaused_) { return; }

        copy_depth_data(frame);




    }

    void copy_depth_data(astra::Frame& frame)
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

        if (depthFrame.is_valid())
        {
            const int width = depthFrame.width();
            const int height = depthFrame.height();
            if (!depthData_ || width != depthWidth_ || height != depthHeight_)
            {
                depthWidth_ = width;
                depthHeight_ = height;

                // texture is RGBA
                const int byteLength = depthWidth_ * depthHeight_ * sizeof(uint16_t);

                depthData_ = DepthPtr(new int16_t[byteLength]);
            }

            depthFrame.copy_to(&depthData_[0]);
        }
    }



private:

    const astra::CoordinateMapper& coordinateMapper_;

    int displayWidth_{0};
    int displayHeight_{0};

    using BufferPtr = std::unique_ptr<uint8_t[]>;
    BufferPtr displayBuffer_{nullptr};

    int depthWidth_{0};
    int depthHeight_{0};

    using DepthPtr = std::unique_ptr<int16_t[]>;
    DepthPtr depthData_{nullptr};

    float mouseNormX_{0};
    float mouseNormY_{0};
    bool isPaused_{false};
    bool isMouseOverlayEnabled_{true};
};








#define DMIN 00
#define DMAX 2000

void createAlphaImage(const cv::Mat& mat, cv::Mat_<cv::Vec4b>& dst)
{
    std::vector<cv::Mat> matChannels;
    cv::split(mat, matChannels);

    // create alpha channel
    cv::Mat alpha = matChannels.at(0) + matChannels.at(1) + matChannels.at(2);
    matChannels.push_back(alpha);

    cv::merge(matChannels, dst);
}


    */
