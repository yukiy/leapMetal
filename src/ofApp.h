#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxLeapMotion.h"

#define _USE_V2

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
    void mousePressed(int x, int y, int button);

    bool camSw;
    bool isMetalized;
    ofSoundPlayer music;
    
    ofImage resizeImage(ofImage img);
    void maskDraw();
    
    ofVideoGrabber cam;
	ofxFaceTracker tracker;
	ofxFaceTracker imgTracker;
	ofImage faceImage;
    ofImage makeImage;

    void leapUpdate();
    void leapDraw();
    void leapDrawWorld();
    void leapDetectHandShape();
    int getDistance(ofPoint point1, ofPoint point2);
    
    ofxLeapMotion leap;
    ofVec3f leapCamPos;
    ofEasyCam leapCam;
    ofLight light1;
    ofLight light2;
    ofMaterial material;

    vector <int> fingersFound;
    map <int, ofPolyline> fingerTrails;
    int thumbToIndex;
    int thumbToMiddle;
    int thumbToRing;
    int thumbToLittle;
    
    
#ifndef _USE_V2
    vector <ofxLeapMotionSimpleHand> simpleHands;
#else
    vector <ofxLeapMotionSimpleHand_v2> simpleHands;
#endif

    
};


