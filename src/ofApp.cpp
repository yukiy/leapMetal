#include "ofApp.h"

using namespace ofxCv;

//-------------------------------------------------------------------------------------------

void ofApp::setup() {
    //---init variable
    camSw = true;
    
    //---setting display
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();

    //---load music
    music.loadSound("lovegun.mp3");
    
    //---camera start
    cam.initGrabber(640,480);
    
    //---tracker start
    tracker.setup();
    imgTracker.setup();
    
    //---allocate memories for images
    faceImage.allocate(640, 480, OF_IMAGE_COLOR);
    faceImage.loadImage("normal.jpg");

    //----load overlay image
    makeImage.loadImage("kiss.jpg");

    
    //---leap start
    leap.open();
    
    //---init leap 3D draw
    leapCam.setOrientation(ofPoint(-30, 180, 0));
    leapCam.setDistance(3);
    leapCam.setGlobalPosition(-70, 420, -220);
    leapCamPos = leapCam.getGlobalPosition();
    light1.setPosition(200, 300, 50);
    light2.setPosition(-200, -200, 50);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

}


//-------------------------------------------------------------------------------------------

void ofApp::update() {
    ofSoundUpdate();

    leapUpdate();

    cam.update();
    
    if(cam.isFrameNew()) {
        imgTracker.update(toCv(faceImage));
        tracker.update(toCv(cam));
    }
}


//-------------------------------------------------------------------------------------------

void ofApp::leapUpdate(){

    fingersFound.clear();

#ifndef _USE_V2
    simpleHands = leap.getSimpleHands();
    
    if( leap.isFrameNew() && simpleHands.size() ){
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        for(int i = 0; i < simpleHands.size(); i++){
            
            for(int j = 0; j < simpleHands[i].fingers.size(); j++){
                int id = simpleHands[i].fingers[j].id;
                
                ofPolyline & polyline = fingerTrails[id];
                ofPoint pt = simpleHands[i].fingers[j].pos;
                
                //if the distance between the last point and the current point is too big - lets clear the line
                //this stops us connecting to an old drawing
                if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
                    polyline.clear();
                }
                
                //add our point to our trail
                polyline.addVertex(pt);
                
                //store fingers seen this frame for drawing
                fingersFound.push_back(id);
            }
        }
    }
#else

    simpleHands = leap.getSimpleHands_v2();
    
    if( leap.isFrameNew() && simpleHands.size() ){
        
//        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
//        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
//        leap.setMappingZ(-150, 150, -200, 200);
        
        for(int i = 0; i < simpleHands.size(); i++){
            
            for(int j = 0; j < simpleHands[i].fingers.size(); j++){
                int id = simpleHands[i].fingers[j].id;
                
                ofPolyline & polyline = fingerTrails[id];
                ofPoint pt = simpleHands[i].fingers[j].tipPosition;
                
                //if the distance between the last point and the current point is too big - lets clear the line
                //this stops us connecting to an old drawing
                if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
                    polyline.clear();
                }
                
                //add our point to our trail
                polyline.addVertex(pt);
                
                //store fingers seen this frame for drawing
                fingersFound.push_back(id);
            }
        }
    }
#endif

    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();	
}



//-------------------------------------------------------------------------------------------

void ofApp::draw() {
    leapDetectHandShape();


    if(camSw){
        ofSetColor(255);
        cam.draw(0,0, 640, 480);
        leapDrawWorld();

        if(tracker.getFound() && imgTracker.getFound() && isMetalized) {
            if(!music.getIsPlaying()){
                music.play();
            }
            maskDraw();
        }else{
            music.stop();
        }
    }else {
        ofBackground(0, 0, 0);
        leapDrawWorld();
    }


    //---draw data
    int left = 20;
    ofSetColor(0);
    ofDrawBitmapString("Frame rate: " + ofToString((int) ofGetFrameRate()), left, 20);
    ofDrawBitmapString("Leap connected? " + ofToString(leap.isConnected()), left, 40);
    ofDrawBitmapString("hand detected? " + ofToString(simpleHands.size()>0), left, 60);
    ofDrawBitmapString("sign detected? " + ofToString(isMetalized), left, 80);
    ofDrawBitmapString("face detected? " + ofToString(tracker.getFound()), left, 100);
}



//-------------------------------------------------------------------------------------------
void ofApp::maskDraw(){
    ofSetOrientation(OF_ORIENTATION_DEFAULT, true);
//    ofSetupScreenOrtho(640, 480, OF_ORIENTATION_DEFAULT, true, -1000, 1000);
    ofSetupScreenOrtho(640, 480, -1000, 1000);
    
    //---get mesh from my face
    ofMesh objectMesh = tracker.getObjectMesh();
    
    //---get mesh of template face
    ofMesh imgMesh = imgTracker.getObjectMesh();
    
    //---apply template mesh to makemesh
    ofMesh makeMesh;
    makeMesh.append(imgMesh);
    
    //---adapt vertieces from camera to image
    for (int i = 0; i < objectMesh.getNumVertices(); i++) {
        makeMesh.setVertex(i, objectMesh.getVertex(i));
    }
    
    //---scale image to fit to the camera
    ofVec2f positon = tracker.getPosition();
    float scale = tracker.getScale();
    ofVec3f orientation = tracker.getOrientation();
    
    ofPushMatrix();
    
    ofTranslate(positon.x, positon.y);
    ofScale(scale, scale, scale);
    ofRotateX(orientation.x * 45.0f);
    ofRotateY(orientation.y * 45.0f);
    ofRotateZ(orientation.z * 45.0f);
    
    ofSetColor(255, 255, 255, 127);
    makeImage.getTextureReference().bind();
    makeMesh.draw();
    makeImage.getTextureReference().unbind();
    
    ofPopMatrix();
}


//-------------------------------------------------------------------------------------------

void ofApp::leapDrawWorld(){
    leapCam.begin();

    ofEnableLighting();
    light1.enable();
    light2.enable();
    material.begin();
    material.setShininess(0.6);
    light2.disable();
    
    for(int i = 0; i < simpleHands.size(); i++){
        simpleHands[i].debugDraw();

        //---draw x pos of the finger
        for(int j = 0; j < simpleHands[i].fingers.size(); j++){
            ofSetColor(255,0,0);
            ofDrawBitmapString("0", simpleHands[i].fingers.at(0).tipPosition);
            ofDrawBitmapString(ofToString(round(thumbToIndex)), simpleHands[i].fingers.at(1).tipPosition);
            ofDrawBitmapString(ofToString(round(thumbToMiddle)), simpleHands[i].fingers.at(2).tipPosition);
            ofDrawBitmapString(ofToString(round(thumbToRing)), simpleHands[i].fingers.at(3).tipPosition);
            ofDrawBitmapString(ofToString(round(thumbToLittle)), simpleHands[i].fingers.at(4).tipPosition);
        }
    }
    
    material.end();
    leapCam.end();

    ofSetColor(255,255,255);
    ofDrawBitmapString("LeapCamPos: " + ofToString(leapCamPos.x) +", "+ ofToString(leapCamPos.y) +", "+ ofToString(leapCamPos.z), 20, ofGetHeight()-20);
}


//-------------------------------------------------------------------------------------------

void ofApp::leapDetectHandShape(){

    for(int i = 0; i < simpleHands.size(); i++){
        
        ofPoint thumbPos = simpleHands[i].fingers.at(0).tipPosition;
        ofPoint indexPos = simpleHands[i].fingers.at(1).tipPosition;
        ofPoint middlePos = simpleHands[i].fingers.at(2).tipPosition;
        ofPoint ringPos = simpleHands[i].fingers.at(3).tipPosition;
        ofPoint littlePos = simpleHands[i].fingers.at(4).tipPosition;
        
        int rangeR = 30; //---collision detection size
        int range = rangeR * rangeR;
        thumbToIndex = getDistance(thumbPos, indexPos);
        thumbToMiddle = getDistance(thumbPos, middlePos);
        thumbToRing = getDistance(thumbPos, ringPos);
        thumbToLittle = getDistance(thumbPos, littlePos);
        
        if(thumbToIndex > range
           && thumbToMiddle < range
           && thumbToRing < range
           && thumbToLittle > range
           ){
            isMetalized = true;
            ofDrawBitmapString("DEATH!", 0,0);
        }else{
            isMetalized = false;
        };
    }

}


//-------------------------------------------------------------------------------------------

int ofApp::getDistance(ofPoint point1, ofPoint point2){
    int dist;
    int distX = point1.x - point2.x;
    int distY = point1.y - point2.y;
    int distZ = point1.z - point2.z;
    return dist = distX*distX + distY*distY + distZ*distZ;
}

//-------------------------------------------------------------------------------------------

void ofApp::keyPressed(int key) {
    if(key == 'r') {
        tracker.reset();
        imgTracker.reset();
    }

    if(key == 'c'){
        if(camSw){
            camSw = false;
        }else{
            camSw = true;
        }
    }
    

    if(key == 'm'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.y++;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    } else if(key == 'i'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.y--;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    }

    if(key == 'l'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.x++;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    } else if(key == 'j'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.x--;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    }

    if(key == 'o'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.z++;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    } else if(key == 'k'){
        leapCamPos = leapCam.getGlobalPosition();
        leapCamPos.z--;
        leapCam.setGlobalPosition(leapCamPos.x, leapCamPos.y, leapCamPos.z);
    }
    
    if(key == 'q'){
        ofVec3f scale = leapCam.getScale();
        scale += 0.5;
        leapCam.setScale(scale);
    }else if(key == 'a'){
        ofVec3f scale = leapCam.getScale();
        scale -= 0.5;
        leapCam.setScale(scale);
    }
    
}


void ofApp::mousePressed(int x, int y, int button){

}
