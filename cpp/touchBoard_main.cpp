#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <typeinfo>
#include <thread>
#include <time.h>
#include <math.h>
#include <windows.h>

// Audio process lib Header
#include <playerUnit.h>
// Frame process lib header
#include <button.h>

// NiTE.h Header
#include <NiTE.h>

// OpenNI Header
#include <OpenNI.h>

// OpenCV Header
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#define DEBUG

//namespace: std, cv, openni, std::chrono
using namespace std;
using namespace cv;
using namespace openni;
using namespace std::chrono;


//typedef std::ratio<1l, 1000000000000l> pico;
typedef duration<long long, pico> picosecond;
//typedef std::ratio<1l, 1000000l> micro;
typedef duration<long long, micro> microsecond;

// set const varible
const unsigned int XRES = 640;
const unsigned int YRES = 480;
const unsigned int BIN_THRESH_OFFSET = 5;
const unsigned int ROI_OFFSET = 70;
const unsigned int MEDIAN_BLUR_K = 5;
const float        DEPTH_SCALE_FACTOR = 255.0 / 4096.0;
const double       GRASPING_THRESH = 0.9;

//偵測膚色前的轉換中介Matrix
Mat intermideate;
Mat skinColor;

// set colors
const Scalar COLOR_BLUE = Scalar(240, 40, 0);
const Scalar COLOR_DARK_GREEN = Scalar(0, 128, 0);
const Scalar COLOR_LIGHT_GREEN = Scalar(0, 255, 0);
const Scalar COLOR_YELLOW = Scalar(0, 128, 200);
const Scalar COLOR_RED = Scalar(0, 0, 255);
const Scalar COLOR_D75B66 = Scalar(102, 91, 215);
const Scalar COLOR_23345C = Scalar(92, 52, 35);     //rgb(35, 52, 92)
const Scalar COLOR_F1BA48 = Scalar(72, 186, 241);   //rgb(241, 186, 72)
const Scalar COLOR_BD8A44 = Scalar(68, 138, 189);   //rgb(189, 138, 68)
const Scalar COLOR_BFAFA0 = Scalar(160, 175, 191);  //rgb(191, 175, 160)

//將感興趣的區域取平均數，將能按到的深度設定成background，到時候和frontground相差就是判斷案鈕的感興趣深度。
void average(vector<Mat1s>& frames, Mat1s& mean) {
    Mat1d acc(mean.size());
    Mat1d frame(mean.size());
    acc = 0.0;
    for (unsigned int i = 0; i<frames.size(); i++) {
        frames[i].convertTo(frame, CV_64FC1);
        acc = acc + frame;
    }
    acc = acc / frames.size();
    acc.convertTo(mean, CV_16SC1);
}

double clockToMilliseconds(clock_t ticks) {
    // units/(units/time) => time (seconds) * 1000 = milliseconds
    return (ticks / (double)CLOCKS_PER_SEC)*1000.0;
}

void second_cout(VinylSample* _in) {
    //while (1) cout<< _in->getPosition()/44100.0<<endl;
}

int main(int argc, char** argv)
{
    //speed up stdio
    std::ios::sync_with_stdio(false);
    cin.tie(NULL);

    static int cueArray[10] = {-1, -1 , -1 , -1, -1, -1 , -1 , -1 , -1 , -1};
    playerUnit player;
    static VinylSample* music1 = new VinylSample("D:/workspace/VS_projects/touchBoard_KinectV1/touchBoard_KinectV1/audio/Message.wav");

    stereoEffects eqEffect1;
    EFFECTGENERATOR(eq3BandEffect, eq1, eq3BandEffect::MODE::NORMAL_EQ3BAND);
    COMBINEEFFECT(eqEffect1, eq1);
    music1->addEffect(&eqEffect1);

    stereoEffects delayEffect1;
    EFFECTGENERATOR(delayEffect, delay1, delayEffect::MODE::NORMAL_DELAY);
    COMBINEEFFECT(delayEffect1, delay1);
    music1->addEffect(&delayEffect1);

    SETEFFECT_STATICPTR(delay1, setPower(Effect::POWER::OFF));
    SETEFFECT_STATICPTR(eq1, setPower(Effect::POWER::ON));
    SETEFFECT_STATICPTR(delay1, setFeedback(0.8));

    music1->setState(AudioSample::STATE::PAUSING);
    player.addSample(music1);

    //thread ti(second_cout, music1);
    const unsigned int nBackgroundTrain = 50;
    const unsigned short touchDepthMin = 5;
    const unsigned short touchDepthMax = 15;
    const unsigned int touchMinArea = 10;

    const double debugFrameMaxDepth = 4000; // maximal distance (in millimeters) for 8 bit debug depth frame quantization
    const char* main_windowName = "GUI interface";
#ifdef DEBUG
    const char* debug_windowName = "DEBUG window";
#endif
    const Scalar debugColor0(0, 0, 128);
    const Scalar debugColor1(84, 140, 29);
    const Scalar debugColor2(255, 255, 255);

    int xMin = 114;
    int xMax = 471;
    int yMin = 175;
    int yMax = 440;

    Mat1s depthShow_16bit(YRES, XRES);        // 16 bit depth (in millimeters)
    Mat1b depthShow_8bit(YRES, XRES);         // 8 bit depth
    Mat   colorShow(YRES, XRES, CV_8UC3);
    Mat   colorShow_hand(YRES, XRES, CV_8UC3);
#ifdef DEBUG
    Mat3b debug(YRES, XRES);                  // debug visualization
#endif
    Mat1s foreground(XRES, YRES);
    //Mat1b foreground8(XRES, YRES);

    Mat1b touchMask(XRES, YRES);              // touch mask

    Mat1s background(YRES, XRES);
    vector<Mat1s> buffer(nBackgroundTrain);

    //Initial OpenNI
    if (OpenNI::initialize() != STATUS_OK) {
        cerr << "OpenNI Initial Error: " << OpenNI::getExtendedError() << endl;
        return -1;
    }
    Device mDevice;
    if (mDevice.open(ANY_DEVICE) != STATUS_OK) {
        cerr << "Can't Open Device: " << OpenNI::getExtendedError() << endl;
        return -1;
    }
    //Create depth stream
    VideoStream mDepthStream;
    if (mDevice.hasSensor(SENSOR_DEPTH)) {
        if (mDepthStream.create(mDevice, SENSOR_DEPTH) == STATUS_OK) {
            //set video mode
            VideoMode mMode;
            mMode.setResolution(640, 480);
            mMode.setFps(30);
            mMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
            if (mDepthStream.setVideoMode(mMode) != STATUS_OK) {
                cout << "Can't apply VideoMode: " << OpenNI::getExtendedError() << endl;
            }
        }
        else {
            cerr << "Can't create depth stream on device: " << OpenNI::getExtendedError() << endl;
            return -1;
        }
    }
    else {
        cerr << "ERROR: This device does not have depth sensor" << endl;
        return -1;
    }
    //Create color stream
    VideoStream mColorStream;
    if (mDevice.hasSensor(SENSOR_COLOR)) {
        if (mColorStream.create(mDevice, SENSOR_COLOR) == STATUS_OK) {
            //set video mode
            VideoMode mMode;
            mMode.setResolution(640, 480);
            mMode.setFps(30);
            mMode.setPixelFormat(PIXEL_FORMAT_RGB888);
            if (mColorStream.setVideoMode(mMode) != STATUS_OK) {
                cout << "Can't apply VideoMode: " << OpenNI::getExtendedError() << endl;
            }
            //image registration 
            //設定自動影像校準技術(深度與彩圖整合)
            //http://www.terasoft.com.tw/support/techpdf/Automating%20Image%20Registration%20with%20MATLAB.pdf
            if (mDevice.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
                mDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
            } else {
                cerr << "Can't set ImageRegistration Mode."<< endl;
            }
        }
        else {
            cerr << "Can't create color stream on device: " << OpenNI::getExtendedError() << endl;
            return -1;
        }
    }

    VideoFrameRef  mDepthFrame;
    VideoFrameRef  mColorFrame;
    mDepthStream.start();
    mColorStream.start();
    player.startAudio();

    //-------------Add Button-----------------
    vector<Buttons*> vButtons;
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(200, 225), 30.0, 30.0, 
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
            switch (state)
            {
            case Buttons::PRESSED:
                cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
                break;

            case Buttons::UNPRESSED:
            {
                cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
            }
            break;
            }
        }, 
        [](Buttons::BUTTON_STATE state) {cout << "RectButton1" << endl; }));

    vButtons.push_back(new RectButton("RectButton testing", cv::Point(200, 255), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cout << "RectButton2" << endl; }));

    vButtons.push_back(new RectButton("RectButton testing", cv::Point(200, 285), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cout << "RectButton3" << endl; }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(200, 315), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cout << "RectButton4" << endl; }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(305, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {
        music1->setCuetoPosition(cueArray[1]);
    }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(275, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {
        music1->setCuetoPosition(cueArray[0]);
    }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(245, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {
        if (cueArray[0] == -1) {
            cueArray[0] = music1->addCue();
        }
        else {
            music1->updateCuePosition(cueArray[0]);
        }
    }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(335, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cueArray[1] = music1->addCue();}));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(365, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cout << "RectButton9" << endl; }));
    vButtons.push_back(new RectButton("RectButton testing", cv::Point(395, 400), 30.0, 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_D75B66, 3);
            if (delay1_1->getPower() == Effect::POWER::OFF) {
                SETEFFECT_STATICPTR(delay1, setPower(Effect::POWER::ON));
            }
            else {
                SETEFFECT_STATICPTR(delay1, setPower(Effect::POWER::OFF));
            }
            break;
        case Buttons::UNPRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_23345C, 3);
            break;
        }
    },
        [](Buttons::BUTTON_STATE state) {cout << "RectButton10" << endl; }));

    vButtons.push_back(new ScrollBarButton("ScrollBarButton testing", cv::Point(150, 300), 180.0 , 30.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width,int offset_y, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BD8A44, 3);  //SCROLL BAR
            //  1/2*height - 1/12*height = 5/12*height 不然小方塊會超過範圍
            cv::rectangle(target, Rect(point.x, point.y + 5*height / 12  + offset_y, width, height / 6), COLOR_BD8A44, 3); //BUTTON  the rate is 6 
            //std::cout << 50 - (offset_y / height) * 100 << "%" << endl;
            break;

        case Buttons::UNPRESSED:
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BFAFA0, 3);  //SCROLL BAR
            cv::rectangle(target, Rect(point.x,point.y+ 5*height/12 + offset_y, width, height/6), COLOR_BFAFA0, 3);  //BUTTON  the rate is 6 
            break;
        }
    },
        [](Buttons::BUTTON_STATE state, double delta) {
        if (state == Buttons::BUTTON_STATE::PRESSED) {
            //cout << "Original" << delta << endl;
            delta -= 50.0;
            delta /= 100.0;
            music1->setSpeed(1.0 + delta);
            //cout << 1.0 + delta << endl;
        }
    }));
    vButtons.push_back(new ScrollBarButton("ScrollBarButton testing", cv::Point(410, 215), 60.0, 15.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, int offset_y, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BD8A44, 3);  //SCROLL BAR
                                                                                                  //  1/2*height - 1/12*height = 5/12*height 不然小方塊會歪
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BD8A44, 3); //BUTTON  the rate is 6 
            //std::cout << 50 - (offset_y / height) * 100 << "%";
        }
        break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BFAFA0, 3);  //SCROLL BAR
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BFAFA0, 3);  //BUTTON  the rate is 6 
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state, double delta) {
        delta -= 50.0;
        delta /= 50.0;
        SETEFFECT_STATICPTR(eq1, setLowGain(1.0+delta));
        //cout << 1.0 + delta << endl;
    }));
    vButtons.push_back(new ScrollBarButton("ScrollBarButton testing", cv::Point(435, 215), 60.0, 15.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, int offset_y, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BD8A44, 3);  //SCROLL BAR
            //  1/2*height - 1/12*height = 5/12*height 不然小方塊會歪
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BD8A44, 3); //BUTTON  the rate is 6 
        }
        break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BFAFA0, 3);  //SCROLL BAR
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BFAFA0, 3);  //BUTTON  the rate is 6 
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state, double delta) {
        delta -= 50.0;
        delta /= 50.0;
        SETEFFECT_STATICPTR(eq1, setMidGain(1.0 + delta));
    }));
    vButtons.push_back(new ScrollBarButton("ScrollBarButton testing", cv::Point(460, 215), 60.0, 15.0,
        [](Buttons::BUTTON_STATE state, cv::Point point, float height, float width, int offset_y, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BD8A44, 3);  //SCROLL BAR
            // 1/2*height - 1/12*height = 5/12*height 不然小方塊會歪
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BD8A44, 3); //BUTTON  the rate is 6 
            //std::cout << 50 - (offset_y / height) * 100 << "%";
        }
        break;

        case Buttons::UNPRESSED:
        {
            cv::rectangle(target, Rect(point, Size(width, height)), COLOR_BFAFA0, 3);  //SCROLL BAR
            cv::rectangle(target, Rect(point.x, point.y + 5 * height / 12 + offset_y, width, height / 6), COLOR_BFAFA0, 3);  //BUTTON  the rate is 6 
        }
        break;
        }
    },
        [](Buttons::BUTTON_STATE state, double delta) {
        delta -= 50.0;
        delta /= 50.0;
        SETEFFECT_STATICPTR(eq1, setHighGain(1.0 + delta));
    }));

    IplImage *src = 0;                                                  //來源影像指標
    IplImage *srctodo = 0;                                              //目標影像指標
    CvSize srctodo_cvsize;                                              //目標影像尺寸

    src = cvLoadImage("D:/workspace/VS_projects/touchBoard_KinectV1/touchBoard_KinectV1/images/vinyl/vinyl_1.jpg", 1);                   //載入影像
    srctodo_cvsize.width = 150;                                         //目標影像的寬為源影像寬的scale倍
    srctodo_cvsize.height = 150;                                        //目標影像的高為源影像高的scale倍
    srctodo = cvCreateImage(srctodo_cvsize, src->depth, src->nChannels);//創立目標影像
    cvResize(src, srctodo, CV_INTER_LINEAR);                            //縮放來源影像到目標影像

    VinylButton vinylButton1("VinylButton testing", cv::Point(320, 300), srctodo_cvsize.width / 2, *srctodo,
        [](Buttons::BUTTON_STATE state, cv::Point point, float radius, IplImage& _src, cv::Mat& target) {
        //switch (state)
        //{
        //case Buttons::PRESSED:
        //    //cv::circle(target, point, radius, cv::Scalar(0, 255, 255), CV_FILLED);
        //    break;
        //case Buttons::UNPRESSED:
        //    //cv::circle(target, point, radius, cv::Scalar(255, 255, 0), CV_FILLED);
        //    break;
        //}
        Mat dst(&(_src), 0);
        Mat imgROI = target(Rect(point.x - radius, point.y - radius, radius * 2, radius * 2));  //指定插入的大小和位置
        addWeighted(imgROI, 0, dst, 1, 0, imgROI);
    },
        [](Buttons::BUTTON_STATE state, double sinDelta, double cosDelta) {
        static Buttons::BUTTON_STATE pre_Bstate;
        static AudioSample::STATE pre_Astate;
        if (state == Buttons::BUTTON_STATE::UNPRESSED) {
            if (pre_Bstate != state) {
                //pre_Bstate = state;
                //music1->setState(pre_Astate);

                

                //music1->exitScratch();
                //music1->clrScratchBuffer();
                //vinylButton1Ptr->setSpinPosition(pos);
            }
            music1->setVinylPressed(false);
        }
        else {
            if (pre_Bstate != state) {
                //pre_Bstate = state;
                //pre_Astate = music1->getState();
                //music1->setScratchBuffer(sinDelta, cosDelta);
                //music1->enterScratch();
                //music1->setState(AudioSample::STATE::SCRATCHING);
                //music1->queuingScratchPosition(-sinDelta, cosDelta);
                
            }
            music1->setVinylPressed(true);
            //music1->setScratchBuffer(sinDelta, cosDelta);
            //music1->queuingScratchPosition(-sinDelta, cosDelta);
        }
    });

    vButtons.push_back(&vinylButton1);
    vinylButton1.setAudioPositionPtr(music1->getPositionPtr());
    music1->setVinylPositionPtr(vinylButton1.getVinylPositionPtr());
    //music1->setTriFunPtr(vinylButton1.getSinPtr(), vinylButton1.getCosPtr());
    static VinylButton* vinylButton1Ptr = &vinylButton1;


    vButtons.push_back(new CircleButton("CircleButton testing", cv::Point(440, 380), 10.0F,
        [](Buttons::BUTTON_STATE state, cv::Point point, float radius, cv::Mat& target) {
        switch (state)
        {
        case Buttons::PRESSED:
            cv::circle(target, point, radius, COLOR_F1BA48, CV_FILLED);
            break;

        case Buttons::UNPRESSED:
            cv::circle(target, point, radius, COLOR_BD8A44, CV_FILLED);
            break;
        }
    },
        [](Buttons::BUTTON_STATE state) {
        if (music1->getState() == AudioSample::STATE::PAUSING) {
            music1->setState(AudioSample::STATE::PLAYING);
        }
        else {
            music1->setState(AudioSample::STATE::PAUSING);
            //vinylButton1Ptr->setSpinPosition(-1.0);
        }
        //
    }));
    //-------------END----Add Button-----------------

    //-------Create a Window with controlbar-------
    cv::namedWindow(main_windowName, WND_PROP_FULLSCREEN);
#ifdef DEBUG
    cv::createTrackbar("xMin", main_windowName, &xMin, 640);
    cv::createTrackbar("xMax", main_windowName, &xMax, 640);
    cv::createTrackbar("yMin", main_windowName, &yMin, 480);
    cv::createTrackbar("yMax", main_windowName, &yMax, 480);
#endif // DEBUG
    cvResizeWindow(main_windowName, 1280, 960);
#ifdef DEBUG
    cv::namedWindow(debug_windowName);
#endif

    HWND m_hwnd = (HWND)cvGetWindowHandle(main_windowName);
    //HWND hRawWnd = ::GetParent(hWnd);

    //-------Train the background-------
    if (mDepthStream.isValid()) {
        if (mDepthStream.readFrame(&mDepthFrame) == STATUS_OK) {
            for (unsigned int i = 0; i < nBackgroundTrain; i++) {
                depthShow_16bit.data = (uchar*)mDepthFrame.getData();
                buffer[i] = depthShow_16bit;
            }
            average(buffer, background);
            cv::flip(background, background, 1);
#ifdef DEBUG
            cv::imshow("Debug_background", background);
#endif
        }
    }
    //-------End Trainning-------

    int keyboardKeynum = 0;
    int frameCount = 0;
    int64 durationTime = 0;
    //clock_t deltaTime = 0;
    //unsigned int framesCounter = 0;
    //double  frameRate = 30;
    //double  averageFrameTimeMilliseconds = 30.0;

    while (/*keyboardKeynum != 27 && keyboardKeynum != 'q' &&*/IsWindowVisible(m_hwnd))
    {
        ////// Start time
        ////if (test == 0) start = clock();
        //clock_t beginFrame = clock();
        frameCount++;
        const auto t1 = high_resolution_clock::now();

        //get depth frame
        if (mDepthStream.isValid()) {
            if (mDepthStream.readFrame(&mDepthFrame) == STATUS_OK) {
                depthShow_16bit.data = (uchar*)mDepthFrame.getData();
                cv::flip(depthShow_16bit, depthShow_16bit, 1);
            }
        }
        //check if color stream is available
        if (mColorStream.isValid()) {
            //get color frame
            if (mColorStream.readFrame(&mColorFrame) == STATUS_OK) {
                //convert data to OpenCV format
                const cv::Mat colorRaw(
                    mColorFrame.getHeight(), mColorFrame.getWidth(),
                    CV_8UC3, (void*)mColorFrame.getData());
                //convert form RGB to BGR
                cv::cvtColor(colorRaw, colorShow, CV_RGB2BGR);
                //水平翻轉
                cv::flip(colorShow, colorShow, 1);
            }
        }

        cv::cvtColor(colorShow, intermideate, CV_BGR2YCrCb);
        cv::inRange(intermideate, Scalar(0, 137, 77), Scalar(256, 177, 127), skinColor);
        cv::erode(skinColor, skinColor, Mat(), Point(-1, -1), 1);
        //cv::dilate(skinColor, skinColor, Mat(), Point(-1, -1), 1);

        //--------計算感興趣區域的contour--------
        foreground = background - depthShow_16bit;
        touchMask = (foreground > touchDepthMin) & (foreground < touchDepthMax);
        Rect roi(xMin, yMin, xMax - xMin, yMax - yMin);
        Mat colorRoi = skinColor(roi);
        Mat touchRoi = touchMask(roi);
        touchRoi = touchRoi & colorRoi;
  
        //將每一個button自己感興趣的區域，傳入button確認是否達到按下的狀態
        for (auto itButton = vButtons.begin(); itButton != vButtons.end(); ++itButton)
            (*itButton)->CheckHand(touchMask(Rect(cv::Point((*itButton)->getPoint().x-(int)(*itButton)->getWidth()/2,
                                                        (*itButton)->getPoint().y-(int)(*itButton)->getHeight()/2),
                                                  Size((int)(*itButton)->getWidth(), (int)(*itButton)->getHeight()))));
        //Debug--在感興趣區域裡，如果面積大於域值，就判斷是一個感興趣點，在這裡只提供Debug作為觀察使用，button並無參考此座標
#ifdef DEBUG
        vector< vector<Point2i> > contoursANYTHING;
        vector<Point2f> touchPoints;
        cv::findContours(touchRoi, contoursANYTHING, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point2i(xMin, yMin));
        for (unsigned int i = 0; i<contoursANYTHING.size(); i++) {
            Mat contourMat(contoursANYTHING[i]);
            // find touch points by area thresholding
            if (contourArea(contourMat) > touchMinArea && contourArea(contourMat) < 100) {
                Scalar center = mean(contourMat);
                Point2i touchPoint(center[0], center[1]);
                touchPoints.push_back(touchPoint);
            }
        }
        depthShow_16bit.convertTo(depthShow_8bit, CV_8U, 255 / debugFrameMaxDepth); // render depth to debug frame
        cv::cvtColor(depthShow_8bit, debug, CV_GRAY2BGR);
        debug.setTo(debugColor0, touchMask);                                        // touch mask
        cv::rectangle(colorShow, roi, debugColor1,2);                               // surface boundaries
        cv::rectangle(debug, roi, debugColor1, 2);                                  // surface boundaries
        for (unsigned int i = 0; i<touchPoints.size(); i++) {                       // touch points
            circle(colorShow, touchPoints[i], 5, debugColor2, CV_FILLED);
            circle(debug, touchPoints[i], 5, debugColor2, CV_FILLED);
        }
#endif
        //--------計算感興趣區域的contour--------
        colorShow_hand = colorShow.clone();

        //將所有的button畫出來
        for (auto itButton = vButtons.begin(); itButton != vButtons.end(); ++itButton)
            (*itButton)->draw(colorShow);
        colorShow = 0.75*colorShow + 0.25*colorShow_hand;

        //waitKey裡的值必 > 0，否則無法使用
        keyboardKeynum = waitKey(1);
        //double seconds;
        ////cout << test << endl;
        //if (++test >= testFrameNUM) {
        //    end = clock();
        //    seconds = difftime(end, start);
        //    seconds /= 1000.0;
        //    fps = (double)testFrameNUM / seconds;
        //    test = 0;
        //    //cout << seconds << endl;
        //}
        const auto t2 = high_resolution_clock::now();
        if (frameCount == 10) {
            frameCount = 0;
            durationTime = duration_cast<picosecond>(t2 - t1).count();
        }
        double timeprecise = durationTime / 1000000000000.0;

        putText(colorShow, string("DJV-01"), Point(250, 170), 0, 1, Scalar(102, 91, 215), 3);
        putText(colorShow,"SPF: "+to_string(timeprecise)+" s", Point(30, 50), 0, 1, Scalar(255, 255, 255), 2);
        //putText(colorShow, to_string(fps), Point(30, 50), 0, 1, Scalar(102, 91, 215), 3);
        cv::imshow(main_windowName, colorShow);
#ifdef DEBUG
        cv::imshow(debug_windowName, debug);
#endif


        //clock_t endFrame = clock();
        //deltaTime += endFrame - beginFrame;
        //framesCounter++;
        //if (clockToMilliseconds(deltaTime)>1000.0) { //every second
        //    frameRate = (double)framesCounter * 0.5 + frameRate * 0.5; //more stable
        //    framesCounter = 0;
        //    deltaTime -= CLOCKS_PER_SEC;
        //    averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
        //    //std::cout << "FrameTime was:" << averageFrameTimeMilliseconds << std::endl;
        //}
    }
    cvDestroyAllWindows();
    player.closeAudio();
    mDepthStream.destroy();
    mColorStream.destroy();
    mDevice.close();
    OpenNI::shutdown();
    return 0;
}

