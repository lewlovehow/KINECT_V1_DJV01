/*
*  AudioSample.h
*  Created by SF234 on 23/08/2017.
*  Copyright. All rights reserved.
*
*/
#include <maximilian.h>
#include "effect.h"
#include <math.h>
#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include "BPMDetect.h"
using namespace std;
#define COMBINE1(X,Y) X##Y  // helper macro
#define COMBINE(X,Y) COMBINE1(X,Y)
#define FINALMACRO
/*
EX.
#include "playerUnit.h"

playerUnit player;
VinylSample music1("Taipei.wav"); //load music
stereoEffects effects1;
player.startAudio();

EFFECTGENERATOR(filterEffect, highpass, filterEffect::MODE::HIGH_PASS);
COMBINEEFFECT(effects1, highpass);
music1.addEffect(&effects1);
player.addSample(&music1);
music1.setState(AudioSample::STATE::PLAYING);
*/
#ifdef TESTMACRO
#define EFFECTGENERATOR(TYPE, NAME, PARA) static TYPE* COMBINE1(NAME, _1) = new TYPE(PARA); static TYPE* COMBINE1(NAME, _2) = new TYPE(PARA);
#define COMBINEEFFECT(TARGET, EFFECT); TARGET.effects[0] = COMBINE(EFFECT, _1); TARGET.effects[1] = COMBINE(EFFECT, _2);
#define SETEFFECT_STATICPTR(NAME, TODO); COMBINE(NAME, _1)->TODO; COMBINE(NAME, _2)->TODO;

#define SETSTEREOEFFECT_PTR(TARGET, TODO) TARGET->effects[0]->TODO; TARGET->effects[1]->TODO;
#define SETSTEREOEFFECT(TARGET, TODO) TARGET.effects[0]->TODO; TARGET.effects[1]->TODO;
//#define GENERATORHELPER(TYPE, NAME, PARA) TYPE NAME(PARA);
//#define EFFECTGENERATOR(TYPE, NAME, PARA) GENERATORHELPER(TYPE, COMBINE1(NAME, _1), PARA) GENERATORHELPER(TYPE, COMBINE1(NAME, _2), PARA)
//#define COMBINEEFFECT(TARGET, EFFECT); TARGET.effects[0] = &COMBINE(EFFECT, _1); TARGET.effects[1] = &COMBINE(EFFECT, _2);
//#define EFFECT_STATICPTRGENERATOR(NAME, TYPE, SOURCE); static TYPE* COMBINE(NAME, _1) = &COMBINE(SOURCE, _1); static TYPE* COMBINE(NAME, _2) = &COMBINE(SOURCE, _2);
//#define SETEFFECT_STATICPTR(NAME, TODO); COMBINE(NAME, _1)->TODO; COMBINE(NAME, _2)->TODO;
//#define SETSTEREOEFFECT_PTR(TARGET, TODO) TARGET->effects[0]->TODO; TARGET->effects[1]->TODO;
//#define SETSTEREOEFFECT(TARGET, TODO) TARGET.effects[0]->TODO; TARGET.effects[1]->TODO;
#endif
#ifdef FINALMACRO
#define EFFECTGENERATOR(TYPE, NAME, PARA) static TYPE* COMBINE1(NAME, _1) = new TYPE(PARA); static TYPE* COMBINE1(NAME, _2) = new TYPE(PARA);
#define COMBINEEFFECT(TARGET, EFFECT); TARGET.effects[0] = COMBINE(EFFECT, _1); TARGET.effects[1] = COMBINE(EFFECT, _2);
#define SETEFFECT_STATICPTR(NAME, TODO); COMBINE(NAME, _1)->TODO; COMBINE(NAME, _2)->TODO;
#endif

mutex g_num_mutex;

struct stereoEffects {
    Effect* effects[2];
};

class AudioSample {
public:
    enum STATE {
        PLAYING,
        PAUSING,
        SCRATCHING,
        LOOPING
    };

    enum AUDIOTYPE {
        VINYLSAMPLE
    };

    AudioSample() {
        vol = 1.0;
    };
    ~AudioSample() {};

    virtual double* getSample() = 0;
    virtual bool   STATEsupport(STATE _state) = 0;
    AUDIOTYPE getAudioType() { return type; }
    void setVolume(double _vol) {
        if (_vol > 10.0) _vol = 10.0;
        if (_vol < 0.0) _vol = 0.0;
        vol = _vol;
    }
    double getVolume() { return vol; }
protected:
    AUDIOTYPE type;
    double vol = 1.0;
};

class VinylSample : public AudioSample {
public:
    class ScratchPosition
    {
    public:
        ScratchPosition(double _sin, double _cos, double _position, double _length, double _freq = 30.0) {
            length_ScratchPosition = _length;
            if (_position >= length_ScratchPosition) _position = length_ScratchPosition - 1;
            if (_position<0) _position = 0;
            cur_position = _position;
            scratchingFreq = _freq;
            setScratch(_sin, _cos);
        };
        ~ScratchPosition() {};
        double cur_position;
        double next_position;
        double scratchingFreq;
        bool   isForward;
        double length_ScratchPosition;

    private:
        void setScratch(double _sin, double _cos) {
            double _arctan = atan(_sin / _cos);
            double deg_1 = 1 * M_PI / 180.0;
            double scale = _arctan / deg_1;
            next_position = cur_position + scale * Deg_1Pos_Ratio;
            if (next_position >= length_ScratchPosition) next_position = length_ScratchPosition - 1;
            if (next_position<0) next_position = 0;

            if (scale > 0) {
                isForward = true;
                scratchingFreq = 1.0*scratchingFreq;
            }
            else {
                isForward = false;
                scratchingFreq = -1.0*scratchingFreq;
            }
        }
        const double Deg_1Pos_Ratio = 245.0;

    };

    struct Buffer_Scratch
    {
        double position;
        double sinTheta;
        double cosTheta;
    };

    enum PLAYMODE {
        PLAY_ONCE,
        PLAY_LOOP
    };

    void scratchChecking() {
        STATE prestate;
        //bool isScratching = false;    //This variable is declared at the private
        while (isLoop) {
            if (scratchVector.size() >= 3 && state != SCRATCHING) {
                prestate = state;
                state = SCRATCHING;
                isScratching = true;
            }
            else {
                int i = 0;
            }
            if (scratchVector.size() == 1 && isScratching) {
                state = prestate;
                isScratching = false;
            }
        }
    }

    void scratchRouting() {
        double MeanPosition = position; //divide three :))
        while (isLoop) {
            if (isVinylPressed) {
                if (scratchVector.size()>2)
                {
                    this_thread::sleep_for(chrono::milliseconds(50));
                    MeanPosition += ((*vinylPositionPtr) - MeanPosition) / 2;
                    scratchVector.push_back(MeanPosition);
                }
                else if (scratchVector.size() == 2)
                {
                    scratchVector.push_back(*vinylPositionPtr);
                }
                /* if (scratchVector.size()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                scratchVector.push_back(*vinylPositionPtr);
                }*/ else {
                    scratchVector.push_back(position);
                }
            }
            else
            {
                MeanPosition = position;
            }
        }
    }

    //Sample = 取樣
    VinylSample() {
        state = PAUSING;
        playMode = PLAY_ONCE;
        type = VINYLSAMPLE;
        speed = 1.0;
        isLoop = true;
        isScratching = false;
        checkingScractchVectorThread = new thread(&VinylSample::scratchChecking, this);
        scratchRoutingThread = new thread(&VinylSample::scratchRouting, this);
        loop_lock = 0;
        //static thread CheckingThread(scratchChecking, &state, &scratchVector, &isLoop);
        //scratchThread = new thread (scratchRouting, vinylPositionPtr, &scratchVector, &isLoop, &isVinylPressed);
        //buffer_scratch[0].position = -1.0;
        //buffer_scratch[1].position = -1.0;
        //buffer_scratch[2].position = -1.0;
        
    };

    VinylSample(string _filePath) {
        loadAudio(_filePath);
        state = PAUSING;
        playMode = PLAY_ONCE;
        type = VINYLSAMPLE;
        isLoop = true;
        isScratching = false;
        loop_lock = 0;

        checkingScractchVectorThread = new thread(&VinylSample::scratchChecking, this);
        scratchRoutingThread = new thread(&VinylSample::scratchRouting, this);
        speed = 1.0;
        //buffer_scratch[0].position = -1.0;
        //buffer_scratch[1].position = -1.0;
        //buffer_scratch[2].position = -1.0;

    };

    ~VinylSample() {
        isLoop = false;
        checkingScractchVectorThread->join();
        scratchRoutingThread->join();
    };

    float bpmDetect(maxiSample *audio) {
        soundtouch::BPMDetect bpm(1, 44100);
        soundtouch::SAMPLETYPE sampleBuffer[6720];
        int counter = -1;
        long n = audio->length;
        float *tempWave;
        
        tempWave = (float*)malloc(n * sizeof(float));
        for (int i = 0; i < n; i++) {
            tempWave[i] = (float)audio->temp[i] / 32767.0f;
        }
        while (counter < n - 1) {
            int num = 0;

            for (int i = 0; i < 6720; i++) {
                if (counter >= n) {
                    break;
                }
                else {
                    num++;
                    counter++;
                    sampleBuffer[i] = tempWave[counter];
                }
            }
            bpm.inputSamples(sampleBuffer, num);
        }
        float bpmValue = bpm.getBpm();
        //cout << "bpm: " << bpmValue << endl;
        //printf("Detected BPM rate %.1f\n\n", bpmValue);
        return bpmValue;
    }

    void loadAudio(string _filePath) {
        filePath = _filePath;
        audioLeftChannel.load(filePath, 0);
        audioRightChannel.load(filePath, 1);
        float bpmLeft = bpmDetect(&audioLeftChannel);
        float bpmRight = bpmDetect(&audioRightChannel);
        if (bpmLeft - bpmRight > 0.001) {
            int dicition;
            cout << "Pick a BPM from LeftChannel or RightChannel " << '\n'
                << "0.LeftChannel:" << bpmLeft << "\n 1.RightChannel:" << bpmRight << '\n';
            cin >> dicition;
            bpm = (dicition == 0) ? bpmLeft : bpmRight;
        }
        else {
            bpm = bpmLeft;
        }
        spb = (1.0 / bpm) * 44100.0 * 60.0;
        length = audioLeftChannel.length;
        position = 0.0;
        audioLeftChannel.reset();
        audioRightChannel.reset();
    }

    void setState(STATE _state) {
        state = _state;
    }

    STATE getState() {
        return state;
    };

    double* getSample() {
        g_num_mutex.lock();
        setRawSample();
        effecting();
        g_num_mutex.unlock();
        return effectedSample;
    }

    void setPlayMode(PLAYMODE _mode) {
        playMode = _mode;
    }

    void setRawSample() {
        switch (state) {
        case PLAYING:
            if (scratchVector.size())
            {
                scratchVector.clear();
            }
            rawSample[0] = audioLeftChannel.play(speed);
            rawSample[1] = audioRightChannel.play(speed);
            position = getPositionHelper();
            if (position == 0) {
                if (playMode == PLAY_ONCE) {
                    state = PAUSING;
                }
            }
            break;
        case PAUSING:
            if (scratchVector.size())
            {
                scratchVector.clear();
            }
            rawSample[0] = 0.0;
            rawSample[1] = 0.0;
            position = getPositionHelper();
            break;
        case SCRATCHING:
            if (scratchVector.size() > 1) {
                if (scratchVector[1] >= scratchVector[0]) {
                    rawSample[0] = audioLeftChannel.play(20.0, scratchVector[0], scratchVector[1]);
                    rawSample[1] = audioRightChannel.play(20.0, scratchVector[0], scratchVector[1]);
                    position = getPositionHelper();
                    if (position <= scratchVector[0] || position >= scratchVector[1]) {
                        if (scratchVector.size())  scratchVector.erase(scratchVector.begin());
                    }
                }
                else {
                    rawSample[0] = audioLeftChannel.play(-20.0, scratchVector[1], scratchVector[0]);
                    rawSample[1] = audioRightChannel.play(-20.0, scratchVector[1], scratchVector[0]);
                    position = getPositionHelper();
                    if (position >= scratchVector[0] || position <= scratchVector[1]) {
                        if (scratchVector.size())  scratchVector.erase(scratchVector.begin());
                    }
                }
            }
            else {
                rawSample[0] = 0.0;
                rawSample[1] = 0.0;
            }
            //if(!positionQueue.empty()) {
            //    scratchingLock = true;
            //    ScratchPosition tmp = positionQueue.front();
            //    if(tmp.isForward) {
            //        rawSample[0] = audioLeftChannel.play4(tmp.scratchingFreq, tmp.cur_position, tmp.next_position);
            //        rawSample[1] = audioRightChannel.play4(tmp.scratchingFreq, tmp.cur_position, tmp.next_position);
            //        position = getPositionHelper();
            //        if( position <= tmp.cur_position || position >= tmp.next_position ) positionQueue.pop();
            //    } else {
            //        rawSample[0] = audioLeftChannel.play4(tmp.scratchingFreq, tmp.next_position, tmp.cur_position);
            //        rawSample[1] = audioRightChannel.play4(tmp.scratchingFreq, tmp.next_position, tmp.cur_position);
            //        position = getPositionHelper();
            //        if( position <= tmp.next_position || position >= tmp.cur_position ) positionQueue.pop();
            //    }
            //} else {
            //    scratchingLock = false;
            //}
            ////clockTime++;
            ////double scratchSample[2];
            ////
            ////scratchSample[0] = 0.0;
            ////scratchSample[1] = 0.0;
            //////initial state
            ////if (firstTime) {
            ////    firstTime = false;
            ////    sin_Buffer.push_back(*sinPtr);
            ////    cos_Buffer.push_back(*cosPtr);
            ////    pos_Buffer.push_back(position);
            ////    clockTime = 0;
            ////}
            ////else {
            ////    if (clockTime > 22050) {
            ////        sin_Buffer.push_back(*sinPtr);
            ////        cos_Buffer.push_back(*cosPtr);
            ////        if (sin_Buffer.size() > (unsigned int)2 && cos_Buffer.size() > (unsigned int)2) {
            ////            double sinAlpha = sin_Buffer[1] * cos_Buffer[0] - cos_Buffer[1] * sin_Buffer[0];
            ////            double cosAlpha = cos_Buffer[1] * cos_Buffer[0] + sin_Buffer[1] * sin_Buffer[0];
            ////            double _atan = atan((double)(-sinAlpha / cosAlpha));
            ////            double tmpPos = pos_Buffer[0] + _atan * (180.0/(double)M_PI)*245.0;
            ////            if(tmpPos>0)pos_Buffer.push_back(tmpPos);
            ////            else pos_Buffer.push_back(0.0);
            ////            if (sin_Buffer.size()) sin_Buffer.erase(sin_Buffer.begin());
            ////            if (cos_Buffer.size()) cos_Buffer.erase(cos_Buffer.begin());
            ////            if (pos_Buffer.size() > (unsigned int)2) {
            ////                scratchPosition[0] = pos_Buffer[0];
            ////                scratchPosition[1] = pos_Buffer[1];
            ////                //double _freq = 22050.0 / (pos_Buffer[1] - pos_Buffer[0]);
            ////                //if (pos_Buffer[1]  >= pos_Buffer[0]) {
            ////                //    scratchSample[0] = audioLeftChannel.play4(_freq, pos_Buffer[0], pos_Buffer[1]);
            ////                //    scratchSample[1] = audioRightChannel.play4(_freq, pos_Buffer[0], pos_Buffer[1]);
            ////                //    position = getPositionHelper();
            ////                //} else {
            ////                //    scratchSample[0] = audioLeftChannel.play4(_freq, pos_Buffer[1], pos_Buffer[0]);
            ////                //    scratchSample[1] = audioRightChannel.play4(_freq, pos_Buffer[1], pos_Buffer[0]);
            ////                //    position = getPositionHelper();
            ////                //}
            ////                if(pos_Buffer.size())pos_Buffer.erase(pos_Buffer.begin());
            ////            }
            ////        }
            ////    }
            ////}
            ////if (scratchPosition[0] >= 0 && scratchPosition[1] >= 0) {
            ////    cout << "enter" << endl;
            ////    //double _freq = 22050.0 / (scratchPosition[1] - scratchPosition[0]);
            ////    if (scratchPosition[1]  >= scratchPosition[0]) {
            ////        rawSample[0] = audioLeftChannel.play4(2.0, scratchPosition[0], scratchPosition[1]);
            ////        rawSample[1] = audioRightChannel.play4(2.0, scratchPosition[0], scratchPosition[1]);
            ////        position = getPositionHelper();
            ////    } else {
            ////        rawSample[0] = audioLeftChannel.play4(-2.0, scratchPosition[1], scratchPosition[0]);
            ////        rawSample[1] = audioRightChannel.play4(-2.0, scratchPosition[1], scratchPosition[0]);
            ////        position = getPositionHelper();
            ////    }
            ////}
            ////else {
            ////    rawSample[0] = 0.0;
            ////    rawSample[1] = 0.0;
            ////}
            //if (!scratchingLock()) {
            //    buffer_scratch[0] = buffer_scratch[1];
            //    buffer_scratch[1] = buffer_scratch[2];
            //    clockTime = clock();
            //}
            //clock_t cur_clock = clock();
            //double deltaTime = clockToMilliseconds(cur_clock - clockTime);
            //if (deltaTime > 40.0 && scratchingLock()) {
            //    clockTime = clock();
            //    if (buffer_scratch[0].position >= 0.0 && buffer_scratch[1].position >= 0.0) {
            //        //double _freq = 441.0 / (buffer_scratch[1].position - buffer_scratch[0].position);
            //        if (buffer_scratch[1].position > buffer_scratch[0].position) {
            //            rawSample[0] = audioLeftChannel.play4(25.0, buffer_scratch[0].position, buffer_scratch[1].position);
            //            rawSample[1] = audioRightChannel.play4(25.0, buffer_scratch[0].position, buffer_scratch[1].position);
            //            position = getPositionHelper();
            //            if (position <= buffer_scratch[1].position || position >= buffer_scratch[0].position) {
            //                buffer_scratch[0] = buffer_scratch[1];
            //                buffer_scratch[1] = buffer_scratch[2];
            //            }
            //        }
            //        else if (buffer_scratch[1].position < buffer_scratch[0].position) {
            //            rawSample[0] = audioLeftChannel.play4(-25.0, buffer_scratch[1].position, buffer_scratch[0].position);
            //            rawSample[1] = audioRightChannel.play4(-25.0, buffer_scratch[1].position, buffer_scratch[0].position);
            //            position = getPositionHelper();
            //            if (position <= buffer_scratch[0].position || position >= buffer_scratch[1].position) {
            //                buffer_scratch[0] = buffer_scratch[1];
            //                buffer_scratch[1] = buffer_scratch[2];
            //            }
            //        } else {
            //            rawSample[0] = 0.0;
            //            rawSample[1] = 0.0;
            //            buffer_scratch[0] = buffer_scratch[1];
            //            buffer_scratch[1] = buffer_scratch[2];
            //        }
            //    } else if (buffer_scratch[0].position >= 0.0 && buffer_scratch[1].position < 0.0) {
            //        rawSample[0] = 0.0;
            //        rawSample[1] = 0.0;
            //        buffer_scratch[0] = buffer_scratch[1];
            //        buffer_scratch[1] = buffer_scratch[2];
            //    }
            //    else if (buffer_scratch[0].position < 0.0 && buffer_scratch[1].position >= 0.0) {
            //        rawSample[0] = 0.0;
            //        rawSample[1] = 0.0;
            //        buffer_scratch[0] = buffer_scratch[1];
            //        buffer_scratch[1] = buffer_scratch[2];
            //        clockTime = clock();
            //    }
            //}
            //else {
            //    rawSample[0] = 0.0;
            //    rawSample[1] = 0.0;
            //}
            break;
       case LOOPING:
            if (scratchVector.size())
            {
                scratchVector.clear();
            }
            float pre_position = getPositionHelper(); 
            rawSample[0] = audioLeftChannel.play(speed);
            rawSample[1] = audioRightChannel.play(speed);
            position = getPositionHelper();
            float diff_position = position - pre_position;
            if (position == 0) {
                if (playMode == PLAY_ONCE) {
                    state = PAUSING;
                }
            }
            samplecounter += diff_position;
            if (position >= loop_end_pos) {
                audioLeftChannel.position = loop_start_pos;
                audioRightChannel.position = loop_start_pos;
            }
            
            break;
        }
    }

    //void enterScratch() {
    //    firstTime = true;
    //    scratchPosition[0] = -1.0;
    //    scratchPosition[1] = -1.0;
    //}

    //void exitScratch() {
    //    cout << "clr" << endl;
    //    if(sin_Buffer.size())sin_Buffer.clear();
    //    if(cos_Buffer.size())cos_Buffer.clear();
    //    if(pos_Buffer.size())pos_Buffer.clear();
    //}

    inline double clockToMilliseconds(clock_t ticks) {
        // units/(units/time) => time (seconds) * 1000 = milliseconds
        return (ticks / (double)CLOCKS_PER_SEC)*1000.0;
    }

    //void queuingScratchPosition(double _sin, double _cos, double _freq = 30.0 ) {
    //    g_num_mutex.lock();
    //    state = SCRATCHING;
    //    double tmpPosition = positionQueue.empty()? position : positionQueue.front().cur_position;
    //    ScratchPosition tmp(_sin, _cos, tmpPosition, length, _freq);
    //    positionQueue.push(tmp);
    //    g_num_mutex.unlock();
    //}

    inline void effecting() {
        effectedSample[0] = rawSample[0];
        effectedSample[1] = rawSample[1];
        for (int i = 0; i < effectList.size(); i++) {
            // effectedSample[0] = effectList[i]->getEffectAudio(effectedSample[0]);
            // effectedSample[1] = effectList[i]->getEffectAudio(effectedSample[1]);
            effectedSample[0] = effectList[i]->effects[0]->getEffectAudio(effectedSample[0]);
            effectedSample[1] = effectList[i]->effects[1]->getEffectAudio(effectedSample[1]);
        }
    }
    void setSpeed(double _speed) {
        speed = _speed;
    }

    void loop_enlock(int lock_password) {
        if (loop_lock == 0) {
            samplecounter = 0;
            loop_start_pos = getPositionHelper();
            loop_lock = lock_password;
        }
    }

    void setMusicLoop(float beats, int lock_password) {
        if (loop_lock == lock_password) {
            loop_end_pos = loop_start_pos + beats*spb;
            if (getState() != LOOPING) {
                loop_pre_state = getState();
                setState(LOOPING);
            }
        }
    }

    void freeMusicLoop(int lock_password) {
        if (loop_lock == lock_password) {
            if (getState() == LOOPING)
            {
                setState(loop_pre_state);
                if (loop_pre_state == PAUSING) {
                    audioLeftChannel.position = loop_start_pos;
                    audioRightChannel.position = loop_start_pos;
                }
                else {
                    audioLeftChannel.position = loop_start_pos + samplecounter;
                    audioRightChannel.position = loop_start_pos + samplecounter;
                }
            }
            loop_lock = 0;
        }
    }
    

    //return the id of effect, this id is distinct to all other id and will give you once,
    //so you need to keep it. 
    //The id will never change, even deletiing
    //any effect object from effectList.
    int addEffect(stereoEffects* _stereoEffects) {
        mapEffectList[++max_effect_id] = (int)effectList.size();
        //cout << effectList.size() << endl;
        effectList.push_back(_stereoEffects);
        return max_effect_id;
    }

    inline Effect::EFFECTTYPE getEffectType(int effect_id, int _index = 0) {
        //return effectList[mapEffectList[effect_id]]->getType();
        return effectList[mapEffectList[effect_id]]->effects[_index]->getType();
    }

    bool checkStereoEffectType(int effect_id) {
        return getEffectType(effect_id, 0) == getEffectType(effect_id, 1); //
    }

    void deleteEffect(int effect_id) {
        effectList.erase(effectList.begin() + mapEffectList[effect_id]);
        mapEffectList.erase(effect_id);
        for (int i = effect_id + 1; i <= max_effect_id; i++) if (mapEffectList.find(i) != mapEffectList.end()) mapEffectList[i] -= 1;//list位置前移
    }

    void clearEffect() { //清除全部
        effectList.clear();
        mapEffectList.clear();
        max_effect_id = -1;
    }
    
    //return the id of cue, this id is distinct to all other id and will give you once,
    //so you need to keep it. 
    //The id will never change, even deletiing
    //any cue object from cueList.
    int addCue() {
        mapCueList[++max_cue_id] = (int)cueList.size();
        cueList.push_back(getPositionHelper());
        return max_cue_id;
    }
    int addCue(double _cur_Position) {
        mapCueList[++max_cue_id] = (int)cueList.size();
        cueList.push_back(_cur_Position);
        return max_cue_id;
    }

    double getCuePosition(int _cue_id) {
        return cueList[mapCueList[_cue_id]];
    }

    /*if th function return false, meaning that can't find the cue object*/
    bool setCuetoPosition(int _cue_id) {
        if (mapCueList.find(_cue_id) != mapCueList.end()) {
            position = cueList[mapCueList[_cue_id]];
            audioLeftChannel.position = position;
            audioRightChannel.position = position;
            return true;
        }
        else {
            return false;
        }
    }

    /*if th function return false, meaning that can't find the cue object*/
    bool updateCuePosition(int _cue_id, double _cur_Position) {
        if (mapCueList.find(_cue_id) != mapCueList.end()) {
            cueList[mapCueList[_cue_id]] = _cur_Position;
            return true;
        }
        else {
            return false;
        } 
    }
    bool updateCuePosition(int _cue_id) {
        if (mapCueList.find(_cue_id) != mapCueList.end()) {
            cueList[mapCueList[_cue_id]] = getPositionHelper();
            return true;
        }
        else {
            return false;
        }
    }
    /*if th function return false, meaning that can't find the cue object*/
    bool deleteCue(int _cue_id) {
        if (mapCueList.find(_cue_id) != mapCueList.end()) {
            cueList.erase(cueList.begin() + mapCueList[_cue_id]);
            mapCueList.erase(_cue_id);
            for (int i = _cue_id + 1; i <= max_cue_id; i++) if (mapCueList.find(i) != mapCueList.end()) mapCueList[i] -= 1;
            return true;
        }
        else {
            return false;
        }
    }

    void clearCue() {
        cueList.clear();
        mapCueList.clear();
        max_cue_id = -1;
    }

    bool STATEsupport(STATE _state) {
        if (_state == PLAYING ||
            _state == PAUSING ||
            _state == SCRATCHING) return true;
        else return false;
    }

    double getPosition() {
        return position;
    }

    //void clrScratchBuffer() {
    //    buffer_scratch[2].position = -1.0;
    //}

    //void setScratchBuffer(double _sin, double _cos) {
    //    if (buffer_scratch[2].position < 0) {
    //        cout << position << endl;
    //        buffer_scratch[2].position = position;
    //        buffer_scratch[2].sinTheta = _sin;
    //        buffer_scratch[2].cosTheta = _cos;
    //        buffer_scratch[3] = buffer_scratch[2];
    //    }
    //    else {
    //        double sinAlpha = _sin * buffer_scratch[3].cosTheta - _cos * buffer_scratch[3].sinTheta;
    //        double cosAlpha = _cos * buffer_scratch[3].cosTheta + _sin * buffer_scratch[3].sinTheta;
    //        double _atan = atan((double)(sinAlpha / cosAlpha));
    //        buffer_scratch[2].position += _atan * ((double)M_PI / 180.0);
    //        buffer_scratch[2].sinTheta = _sin;
    //        buffer_scratch[2].cosTheta = _cos;
    //    }
    //}

    //void setTriFunPtr(double* _sinPtr, double* _cosPtr) {
    //    sinPtr = _sinPtr;
    //    cosPtr = _cosPtr;
    //}

    //double getSin() {
    //    return *sinPtr;
    //}

    //double getCos() {
    //    return *cosPtr;
    //}

    double *getPositionPtr() {
        return &position;
    }

    void setVinylPositionPtr(double *_vinylPositionPtr) {
        vinylPositionPtr = _vinylPositionPtr;
    }

    void setVinylPressed(bool _ispressed) {
        isVinylPressed = _ispressed;
    }

private:
    maxiSample audioLeftChannel, audioRightChannel;

    int max_cue_id = -1;
    vector<double> cueList;
    std::map<int, int> mapCueList;

    Buffer_Scratch buffer_scratch[4];
    time_t clockTime;

    //int clockTime;
    //double *sinPtr, *cosPtr;
    bool firstTime;
    vector<double> sin_Buffer;
    vector<double> cos_Buffer;
    vector<double> pos_Buffer;
    double scratchPosition[2];

    // int max_effect_id = -1;
    // vector<Effect*> effectList;
    // std::map<int, int> mapEffectList;
    int max_effect_id = -1;
    vector<stereoEffects*> effectList;
    std::map<int, int> mapEffectList;

    inline bool scratchingLock() {
        //cout << buffer_scratch[0].position << " " << buffer_scratch[1].position << endl;
        return (buffer_scratch[0].position >= 0.0) || (buffer_scratch[1].position >= 0.0);
    }
    //bool scratchingLock = false;
    bool isScratching;
    double isLoop;
    double *vinylPositionPtr;
    bool isVinylPressed;
    vector<double> scratchVector;
    thread *scratchRoutingThread;
    thread *checkingScractchVectorThread;
    queue<ScratchPosition> positionQueue;
    /*0: LeftChannel, 1: RightChannel*/
    double rawSample[2];
    /*0: LeftChannel, 1: RightChannel*/
    double effectedSample[2];
    double position;
    double speed;
    long length;
    string filePath;
    STATE state;
    PLAYMODE playMode;
    float bpm;
    float spb;
    float loop_start_pos;
    float loop_end_pos;
    int   loop_lock;
    STATE loop_pre_state;
    unsigned int samplecounter;
    inline double getPositionHelper() {
        //g_num_mutex.lock();
        if ((long)audioLeftChannel.position == (long)audioRightChannel.position) {
            return audioLeftChannel.position;
        }
        else {
            long avr = (audioRightChannel.position + audioLeftChannel.position) / (long)2;
            audioLeftChannel.position = avr;
            audioRightChannel.position = avr;
            return avr;
        }
        //g_num_mutex.unlock();
    };

};