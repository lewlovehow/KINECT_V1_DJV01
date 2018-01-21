#include "maximilian.h"
#include "threeBandEqualizer.h"

class Effect {
public:
    enum EFFECTTYPE {
        DELAYEFFECT,
        FILTEREFFECT,
        DYNAMICEFFECT,
        DISTORTIONEFFECT,
        FLANGEREFFECT,
        CHORUSEFFECT,
        SVFEFFECT,
        EQ3BANDEFFECT
    };

    enum POWER {
        ON,
        OFF
    };

    Effect() {
        power = OFF;
    };
    ~Effect() {};

    virtual double getEffectAudio(double input) = 0;

    void setGain( double value )     { gain = value; }
    double getGain()                 { return gain; }
    //double* getGainLocation()        { return (&gain); }
    EFFECTTYPE getType()             { return type; }
    void setPower(POWER _power)      { power = _power; }
    POWER getPower()                 { return power; }

protected:
    EFFECTTYPE type;
    POWER power;
    double gain;
};
class delayEffect : public Effect {
public:
    enum MODE {
        NORMAL_DELAY,
        POSITION_DELAY
    };
    ////useable default value
    delayEffect() {
        mode = NORMAL_DELAY;
        type = DELAYEFFECT;
        size = 0;
        feedback = 0.5;
        position = 0;
        gain = 1.0;
    };
    delayEffect(double _gain, int _size, double _feedback, int _position) {
        mode = NORMAL_DELAY;
        type = DELAYEFFECT;
        size = _size;
        feedback = _feedback;
        position = _position;
        gain = _gain;
    };
    delayEffect(MODE _mode) {
        mode = _mode;
        type = DELAYEFFECT;
        size = 10000;
        feedback = 0.5;
        position = 0;
        gain = 1.0;
    };
    delayEffect(MODE _mode, double _gain, int _size, double _feedback, int _position) {
        mode = _mode;
        type = DELAYEFFECT;
        size = _size;
        feedback = _feedback;
        position = _position;
        gain = _gain;
    };
    ~delayEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case NORMAL_DELAY:
                return input + delay.dl(input, size, feedback)*gain;
                break;
            case POSITION_DELAY:
                return input + delay.dl(input, size, feedback, position)*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }

    void setSize(int value) {
        switch(mode) {
            case NORMAL_DELAY:
                size = value;
                break;
            case POSITION_DELAY:
                size = value;
                break;
            default:
                break;
        }
    }
    int getSize() {
        switch(mode) {
            case NORMAL_DELAY:
                return size;
                break;
            case POSITION_DELAY:
                return size;
                break;
            default:
                return 0;
                break;
        }
    }

    void setFeedback( double value ) {
        switch(mode) {
            case NORMAL_DELAY:
                feedback = value;
                break;
            case POSITION_DELAY:
                feedback = value;
                break;
            default:
                break;
        }
    }

    double getFeedback() {
        switch(mode) {
            case NORMAL_DELAY:
                return feedback;
                break;
            case POSITION_DELAY:
                return feedback;
                break;
            default:
                return 0.0;
                break;
        }
    }

    void setPosition(int value) {
        switch(mode) {
            case POSITION_DELAY:
                position = value;
                break;
            default:
                break;
        }
    }

    int getPosition() {
        switch(mode) {
            case POSITION_DELAY:
                return position;
                break;
            default:
                return 0;
                break;
        }
    }

    MODE getMode() {
        return mode;
    }

private:
    maxiDelayline delay;
    MODE mode;
    int size;
    int position;
    double feedback;
};

class filterEffect : public Effect {
public:
    enum MODE {
        LOW_RESONANCE,
        HIGH_RESONANCE,
        BAND_PASS,
        LOW_PASS,
        HIGH_PASS,
        NONE
    };

    filterEffect() {
        mode = NONE;
        type = FILTEREFFECT;
        gain = 1.0;
        cutoff = 0.5;
        resonance = 0.2;
    };
    filterEffect(MODE _mode) {
        mode = _mode;
        type = FILTEREFFECT;
        gain = 1.0;
        cutoff = 0.5;
        resonance = 0.2;
    };
    filterEffect(MODE _mode, double _gain, double _cutoff, double _resonance) {
        mode = _mode;
        type = FILTEREFFECT;
        gain = _gain;
        cutoff = _cutoff;
        resonance = _resonance;
    };
    ~filterEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case LOW_RESONANCE:
                return filter.lores(input, cutoff, resonance)*gain;
                break;
            case HIGH_RESONANCE:
                return filter.hires(input, cutoff, resonance)*gain; 
                break;
            case BAND_PASS:
                return filter.bandpass(input, cutoff, resonance)*gain;
                break;
            case LOW_PASS:
                return filter.lopass(input, cutoff)*gain;
                break;
            case HIGH_PASS:
                return filter.hipass(input, cutoff)*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }
    void setCutoff( double value ) {
        switch(mode) {
            case LOW_RESONANCE:
                cutoff = value;
                break;
            case HIGH_RESONANCE:
                cutoff = value;
                break;
            case BAND_PASS:
                cutoff = value;
                break;
            case LOW_PASS:
                cutoff = value;
                break;
            case HIGH_PASS:
                cutoff = value;
                break;
            default:
                break;
        }
    }
    double getCutoff() {
        switch(mode) {
            case LOW_RESONANCE:
                return cutoff;
                break;
            case HIGH_RESONANCE:
                return cutoff;
                break;
            case BAND_PASS:
                return cutoff;
                break;
            case LOW_PASS:
                return cutoff;
                break;
            case HIGH_PASS:
                return cutoff;
                break;
            default:
                return 0.0;
                break;
        }
    }

    void setResonance( double value ) {
        switch(mode) {
            case LOW_RESONANCE:
                resonance = value;
                break;
            case HIGH_RESONANCE:
                resonance = value;
                break;
            case BAND_PASS:
                resonance = value;
                break;
            case LOW_PASS:
                break;
            case HIGH_PASS:
                break;
            default:
                break;
        }
    }

    double getResonance() {
        switch(mode) {
            case LOW_RESONANCE:
                return resonance;
                break;
            case HIGH_RESONANCE:
                return resonance;
                break;
            case BAND_PASS:
                return resonance;
                break;
            case LOW_PASS:
                return 0.0;
                break;
            case HIGH_PASS:
                return 0.0;
                break;
            default:
                return 0.0;
                break;
        }
    }

    MODE getMode() {
        return mode;
    }
private:
    maxiFilter filter;
    MODE mode;
    double cutoff;
    double resonance;
};

class dynamicEffect : public Effect {
public:
    enum MODE {
        GATE,
        TYPE1_COMPRESSOR,
        TYPE2_COMPRESSOR,
        NONE
    };

    dynamicEffect() {
        mode = NONE;
        type = DYNAMICEFFECT;
        gain = 1.0;
        threshold = 0.9;
        holdtime = 1;
        attack = 1;
        release = 0.9995;
        ratio = 1.0;
    };
    dynamicEffect(MODE _mode, double _gain = 1.0, double _threshold = 0.9, long _holdtime = 1, double _attack = 1, double _release = 0.9995, double _ratio = 1.0) {
        mode = _mode;
        type = DYNAMICEFFECT;
        gain = 1.0;
        threshold = 0.9;
        holdtime = 1;
        attack = 1;
        release = 0.9995;
        ratio = 1.0;
    };

    ~dynamicEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case GATE:
                return dynamic.gate( input, threshold, holdtime, attack, release )*gain;
                break;
            case TYPE1_COMPRESSOR:
                return dynamic.compressor( input, ratio, threshold, attack, release )*gain;
                break;
            case TYPE2_COMPRESSOR:
                return dynamic.compress( input )*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }

    //value: millisecond
    //GATE, TYPE1_COMPRESSOR: set directly, TYPE2_COMPRESSOR: useing maxiDyn::setThreshold(double thresholdI)
    void setThreshold( double value ) {
        switch(mode) {
            case GATE:
                threshold = value;
                break;
            case TYPE1_COMPRESSOR:
                threshold = value;
                break;
            case TYPE2_COMPRESSOR:
                dynamic.setThreshold( value );
                break;
            default:
                break;
        }
    };
    double getThreshold() {
        switch(mode) {
            case GATE:
                return threshold;
                break;
            case TYPE1_COMPRESSOR:
                return threshold;
                break;
            case TYPE2_COMPRESSOR:
                return dynamic.threshold;
                break;
            default:
                return 0.0;
                break;
        }
    }
    //value: millisecond, this is for GATE
    void setHoldtime( long value ) {
        switch(mode) {
            case GATE:
                holdtime = value;
                break;
            default:
                break;
        }
    }
    long getHoldtime() {
        switch(mode) {
            case GATE:
                return holdtime;
                break;
            default:
                return 0.0;
                break;
        }
    }
    //value: millisecond
    //GATE, TYPE1_COMPRESSOR: set directly, TYPE2_COMPRESSOR: useing maxiDyn::setAttack(double attackMS)
    void setAttack( double value ) {
        switch(mode) {
            case GATE:
                attack = value;
                break;
            case TYPE1_COMPRESSOR:
                attack = value;
                break;
            case TYPE2_COMPRESSOR:
                dynamic.setAttack( value );
                break;
            default:
                break;
        }
    }
    double getAttack() {
        switch(mode) {
            case GATE:
                return attack;
                break;
            case TYPE1_COMPRESSOR:
                return attack;
                break;
            case TYPE2_COMPRESSOR:
                return dynamic.attack;
                break;
            default:
                return 0.0;
                break;
        }
    }
    //value: millisecond
    //GATE, TYPE1_COMPRESSOR: set directly, TYPE2_COMPRESSOR: useing maxiDyn::setRelease(double attackMS)
    void setRelease( double value ) {
        switch(mode) {
            case GATE:
                release = value;
                break;
            case TYPE1_COMPRESSOR:
                release = value;
                break;
            case TYPE2_COMPRESSOR:
                dynamic.setRelease( value );
                break;
            default:
                break;
        }
    }
    double getRelease() {
        switch(mode) {
            case GATE:
                return release;
                break;
            case TYPE1_COMPRESSOR:
                return release;
                break;
            case TYPE2_COMPRESSOR:
                return dynamic.release;
                break;
            default:
                return 0.0;
                break;
        }
    }
    //GATE, TYPE1_COMPRESSOR: set directly, TYPE2_COMPRESSOR: useing maxiDyn::setRatio(double ratioF)
    void setRatio( double value ) {
        switch(mode) {
            case TYPE1_COMPRESSOR:
                ratio = value;
                break;
            case TYPE2_COMPRESSOR:
                dynamic.setRatio( value );
                break;
            default:
                break;
        }    
    }
    double getRatio() {
        switch(mode) {
            case TYPE1_COMPRESSOR:
                return ratio;
                break;
            case TYPE2_COMPRESSOR:
                return dynamic.ratio;
                break;
            default:
                return 0.0;
                break;
        }    
    }
    MODE getMode() {
        return mode;
    }
private:
    maxiDyn dynamic;
    MODE mode;
    //gate, compressor varible
    double threshold;
    //gate varible
    long holdtime;
    //gate, compressor varible
    double attack;
    //gate, compressor varible
    double release;
    //compressor varible
    double ratio;
};

/*atan distortion, see http://www.musicdsp.org/showArchiveComment.php?ArchiveID=104
shape from 1 (soft clipping) to infinity (hard clipping)*/
class distortionEffect : public Effect {
public:
    enum MODE {
        ATAN_DIST_DISTORTION,
        FAST_ATAN_DIST_DISTORTION
    };

    distortionEffect(double _gain = 1.0, double _shape = 5.0) {
        mode = ATAN_DIST_DISTORTION;
        type = DISTORTIONEFFECT;
        gain = _gain;
        shape = _shape;
    };

    distortionEffect(MODE _mode, double _gain = 0.1, double _shape = 75.0) {
        mode = _mode;
        type = DISTORTIONEFFECT;
        gain = _gain;
        shape = _shape;
    };

    ~distortionEffect() {};
    
    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case ATAN_DIST_DISTORTION:
                return distortion.atanDist(input, shape)*gain;
                break;
            case FAST_ATAN_DIST_DISTORTION:
                return distortion.fastAtanDist(input, shape)*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }

    void setShape( double value ) { shape = value; }
    double getShape() { return shape; }

    MODE getMode() {
        return mode;
    }
private:
    MODE mode;
    maxiDistortion distortion;
    double shape;
};

class flangerEffect : public Effect {
public:
    enum MODE {
        NORMAL_FLANGE
    };

    flangerEffect() {
        mode = NORMAL_FLANGE;
        type = FLANGEREFFECT;
        gain = 1.0;
        delay = 10;
        feedback = 0.5;
        speed = 1.2;
        depth = 3.0;
    };

    flangerEffect(MODE _mode, double _gain = 1.85, unsigned int _delay = 20000, double _feedback = 0.7, double _speed = 7, double _depth = 0.9) {
        mode = _mode;
        type = FLANGEREFFECT;
        gain = _gain;
        delay = _delay;
        feedback = _feedback;
        speed = _speed;
        depth = _depth;
    };

    ~flangerEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case NORMAL_FLANGE:
                return /*input +*/ flanger.flange(input, delay, feedback, speed, depth)*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }

    void setDelay( unsigned int value ) {
        switch(mode) {
            case NORMAL_FLANGE:
                delay = value;
                break;
            default:
                break;
        }
    }
    unsigned int getDelay() {
        return delay;
    }
    void setFeedback( double value ) {
        switch(mode) {
            case NORMAL_FLANGE:
                feedback = value;
                break;
            default:
                break;
        }
    }
    double getFeedback() {
        return feedback;
    }
    void setSpeed( double value ) {
        switch(mode) {
            case NORMAL_FLANGE:
                speed = value;
                break;
            default:
                break;
        }
    }
    double getSpeed() {
        return speed;
    }
    void setDepth( double value ) {
        switch(mode) {
            case NORMAL_FLANGE:
                depth = value;
                break;
            default:
                break;
        }
    }
    double getDetpth() {
        return depth;
    }
    MODE getMode() {
        return mode;
    }
private:
    maxiFlanger flanger;
    MODE mode;
    unsigned int delay;
    double feedback;
    double speed;
    double depth;
};

class chorusEffect : public Effect {
public:
    enum MODE {
        NORMAL_CHORUS
    };
    chorusEffect() {
        mode = NORMAL_CHORUS;
        type = CHORUSEFFECT;
        gain = 1.0;
        delay = 10;
        feedback = 0.5;
        speed = 1.2;
        depth = 3.0;
    };
    chorusEffect(MODE _mode = NORMAL_CHORUS, double _gain = 1.85, unsigned int _delay = 2000, double _feedback = 0.7, double _speed = 7, double _depth = 0.9) {
        mode = _mode;
        type = CHORUSEFFECT;
        gain = _gain;
        delay = _delay;
        feedback = _feedback;
        speed = _speed;
        depth = _depth;
    };
    ~chorusEffect() {};
    

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case NORMAL_CHORUS:
                return /*input +*/ chorus.chorus(input, delay, feedback, speed, depth)*gain;
                break;
            default:
                return 0.0; 
                break;
        }
        else return input;
    }
    void setDelay( unsigned int value ) {
        switch(mode) {
            case NORMAL_CHORUS:
                delay = value;
                break;
            default:
                break;
        }
    }
    unsigned int getDelay() {
        return delay;
    }
    void setFeedback( double value ) {
        switch(mode) {
            case NORMAL_CHORUS:
                feedback = value;
                break;
            default:
                break;
        }
    }
    double getFeedback() {
        return feedback;
    }
    void setSpeed( double value ) {
        switch(mode) {
            case NORMAL_CHORUS:
                speed = value;
                break;
            default:
                break;
        }
    }
    double getSpeed() {
        return speed;
    }
    void setDepth( double value ) {
        switch(mode) {
            case NORMAL_CHORUS:
                depth = value;
                break;
            default:
                break;
        }
    }
    double getDetpth() {
        return depth;
    }
    MODE getMode() {
        return mode;
    }
private:
    MODE mode;
    maxiChorus chorus;
    unsigned int delay;
    double feedback;
    double speed;
    double depth;
};

class SVFEffect : public Effect {
public:
    enum MODE {
        NORMAL_SVF
    };

    SVFEffect() {
        mode = NORMAL_SVF;
        type = SVFEFFECT;
        gain = 1.0;
        lpmix = 0.0;
        bpmix = 1.0;
        hpmix = 0.0;
        notchmix = 0.0;
    };
    SVFEffect(MODE _mode = NORMAL_SVF, double _gain = 1.0, double _lpmix = 0.0, double _bpmix = 1.0, double _hpmix = 0.0, double _notchmix = 0.0) {
        mode = _mode;
        type = SVFEFFECT;
        gain = _gain;
        lpmix = _lpmix;
        bpmix = _bpmix;
        hpmix = _hpmix;
        notchmix = _notchmix;
    };
    ~SVFEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case NORMAL_SVF:
                return svf.play(input, lpmix, bpmix, hpmix, notchmix)*gain;
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }
    void setCutoff( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                svf.setCutoff(value);
                break;
            default:
                break;
        }
    }
    void setResonance( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                svf.setResonance(value);
                break;
            default:
                break;
        }
    }

    void setLpmix( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                lpmix = value;
                break;
            default:
                break;
        }
    }
    double getLpmix() {
        switch(mode) {
            case NORMAL_SVF:
                return lpmix;
                break;
            default:
                return 0.0;
                break;
        }
    }
    void setBpmix( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                bpmix = value;
                break;
            default:
                break;
        }
    }
    double getBpmix() {
        switch(mode) {
            case NORMAL_SVF:
                return bpmix;
                break;
            default:
                return 0.0;
                break;
        }
    }
    void setHpmix( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                hpmix = value;
                break;
            default:
                break;
        }
    }
    double getHpmix() {
        switch(mode) {
            case NORMAL_SVF:
                return hpmix;
                break;
            default:
                return 0.0;
                break;
        }
    }
    void setNotchmix( double value ) {
        switch(mode) {
            case NORMAL_SVF:
                notchmix = value;
                break;
            default:
                break;
        }
    }
    double getNotchmix() {
        switch(mode) {
            case NORMAL_SVF:
                return notchmix;
                break;
            default:
                return 0.0;
                break;
        }
    }
    MODE getMode() {
        return mode;
    }
private:
/*
 State Variable Filter
 
 algorithm from  http://www.cytomic.com/files/dsp/SvfLinearTrapOptimised.pdf
 usage:
 either set the parameters separately as required (to save CPU)
 
 filter.setCutoff(param1);
 filter.setResonance(param2);
 
 w = filter.play(w, 0.0, 1.0, 0.0, 0.0);
 
 or set everything together at once
 
 w = filter.setCutoff(param1).setResonance(param2).play(w, 0.0, 1.0, 0.0, 0.0);
 
 */
    maxiSVF svf;
    MODE mode;
    double lpmix;
    double bpmix;
    double hpmix;
    double notchmix;
};

class eq3BandEffect : public Effect {
public:
    enum MODE {
        NORMAL_EQ3BAND
    };
    eq3BandEffect(MODE _mode = NORMAL_EQ3BAND) {
        mode = _mode;
        type = EQ3BANDEFFECT;
        gain = 1.0;
        eq.init_3band_state();
        lg = 1.0; // low gain
        mg = 1.0; // mid gain
        hg = 1.0; // high gain
    };
    //eq3BandEffect(MODE _mode = NORMAL_EQ3BAND, double _gain = 1.0, double _lg = 1.0, double _mg = 1.0, double _hg = 1.0, int _lowfreq = 880, int _highfreq = 5000, int _mixfreq = 44100) {
    //    mode = _mode;
    //    type = EQ3BANDEFFECT;
    //    gain = _gain;
    //    lg = _lg; // low gain
    //    mg = _mg; // mid gain
    //    hg = _hg; // high gain
    //    lowfreq = _lowfreq;
    //    highfreq = _highfreq;
    //    mixfreq = _mixfreq;
    //    eq.setFreq(lowfreq, highfreq, mixfreq);
    //};
    ~eq3BandEffect() {};

    double getEffectAudio(double input) {
        if( power == ON )
        switch(mode) {
            case NORMAL_EQ3BAND:
                return eq.do_3band(input);
                break;
            default:
                return 0.0;
                break;
        }
        else return input;
    }
    void  setEqStateGain(double _lg, double _mg, double _hg) {
        switch(mode) {
            case NORMAL_EQ3BAND:
                eq.setEqStateGain(_lg, _mg, _hg);
                lg = _lg; // low gain
                mg = _mg; // mid gain
                hg = _hg; // high gain
                break;
            default:
                break;
        }
    }
    void  setLowGain(double _lg) {
        switch (mode) {
        case NORMAL_EQ3BAND:
            eq.setEqStateGain(_lg, mg, hg);
            lg = _lg; // low gain
            break;
        default:
            break;
        }
    }
    void  setMidGain(double _mg) {
        switch (mode) {
        case NORMAL_EQ3BAND:
            eq.setEqStateGain(lg, _mg, hg);
            mg = _mg; // mid gain
            break;
        default:
            break;
        }
    }
    void  setHighGain(double _hg) {
        switch (mode) {
        case NORMAL_EQ3BAND:
            eq.setEqStateGain(lg, mg, _hg);
            hg = _hg; // high gain
            break;
        default:
            break;
        }
    }
    double getLowGain() {
        switch(mode) {
            case NORMAL_EQ3BAND:
                return lg;
                break;
            default:
                return 0.0;
                break;
        }
    }

    double getMidGain() {
        switch(mode) {
            case NORMAL_EQ3BAND:
                return mg;
                break;
            default:
                return 0.0;
                break;
        }
    }

    double getHighGain() {
        switch(mode) {
            case NORMAL_EQ3BAND:
                return hg;
                break;
            default:
                return 0.0;
                break;
        }
    }

    void setFreq( int _lowfreq, int _highfreq, int _mixfreq ) {
        lowfreq = _lowfreq;
        highfreq = _highfreq;
        mixfreq = _mixfreq;
        eq.setFreq( lowfreq, highfreq, mixfreq );
    }
    MODE getMode() {
        return mode;
    }
private:
    eq3Band eq;
    MODE mode;
    int lowfreq;
    int highfreq;
    int mixfreq;
    double lg; // low gain
    double mg; // mid gain
    double hg; // high gain
};