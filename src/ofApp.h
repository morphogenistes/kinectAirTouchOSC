#pragma once

#include "ofMain.h"
//#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "Button.h"
#include "Kinect.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 9999

class ofApp : public ofBaseApp{
private:
    Kinect k;
    Button saveBtn, reloadBtn;
    ofxXmlSettings config;
    ofxOscSender sender;
    
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;

    void keyPressed(int key) override;
    //void keyReleased(int key) override;
    //void mouseMoved(int x, int y ) override;
    void mousePressed(int x, int y, int button) override;
    void mouseDragged(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    //void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    //void mouseEntered(int x, int y) override;
    //void mouseExited(int x, int y) override;
    void windowResized(int w, int h) override;
    void dragEvent(ofDragInfo dragInfo) override;
    void gotMessage(ofMessage msg) override;
};
