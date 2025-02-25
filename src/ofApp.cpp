#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    k.init();
    saveBtn = {{420,10,120,25}, "save"};
    reloadBtn = {{420,40,120,25}, "reset"};
    
    // populate config in  case the file doesn't exist
    config.getValue("angle", 0);
    config.setValue("distance", 120);
    config.setValue("areaX", 0.);
    config.setValue("areaY", 0.);
    config.setValue("areaW", 64.);
    config.setValue("areaH", 48.);
    
    config.load("config.xml");
    k.setAngle(config.getValue("angle", 0));
    k.setDistance(config.getValue("distance", 120));
    k.setActiveZone({
        static_cast<float>(config.getValue("areaX", 0.)),
        static_cast<float>(config.getValue("areaY", 0.)),
        static_cast<float>(config.getValue("areaW", 64.)),
        static_cast<float>(config.getValue("areaH", 48.))
    });
    
    sender.setup(HOST, PORT);
}

//--------------------------------------------------------------
void ofApp::update(){
    k.update();
    std::vector<ofVec2f>& cursors = k.getCursors();
    
    ofxOscMessage m;
    m.setAddress("/blobs");

    if (cursors.size() > 0) {
        for (auto i = 0; i < cursors.size(); ++i) {
            m.addFloatArg(cursors[i].x);
            m.addFloatArg(cursors[i].y);
        }
    }

    sender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::draw(){
    k.draw();
    saveBtn.draw();
    reloadBtn.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    k.keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int btn){
    k.mousePressed(x, y, btn);
    saveBtn.mousePressed(x, y, btn);
    reloadBtn.mousePressed(x, y, btn);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int btn){
    k.mouseDragged(x, y, btn);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int btn){
    k.mouseReleased(x, y, btn);
    if (saveBtn.mouseReleased(x, y, btn)) {
        ofLogNotice() << "clicked save";
        ofRectangle area = k.getActiveZone();
        config.setValue("angle", k.getAngle());
        config.setValue("distance", k.getDistance());
        config.setValue("areaX", area.x);
        config.setValue("areaY", area.y);
        config.setValue("areaW", area.width);
        config.setValue("areaH", area.height);
        config.save("config.xml");
    }
    if (reloadBtn.mouseReleased(x, y, btn)) {
        ofLogNotice() << "clicked reset";
        config.load("config.xml");
        k.setAngle(config.getValue("angle", 0));
        k.setDistance(config.getValue("distance", 0));
        k.setActiveZone({
            static_cast<float>(config.getValue("areaX", 0.)),
            static_cast<float>(config.getValue("areaY", 0.)),
            static_cast<float>(config.getValue("areaW", 64.)),
            static_cast<float>(config.getValue("areaH", 48.))
        });
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
