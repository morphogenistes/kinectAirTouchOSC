//
//  Button.h
//  globefluxcontrol
//
//  Created by joseph larralde on 24/02/2025.
//

#ifndef Button_h
#define Button_h

class Button {
private:
    bool pressed;
    ofRectangle area;
    std::string text;
    ofColor onColor, offColor, textColor;
    
public:
    Button(ofRectangle where = {0,0,0,0}, std::string what = "") :
    pressed(false), area(where), text(what) {
        onColor = {0,128,255};
        offColor = {0,64,255};
        textColor = {255,255,255};
    }
    
    ~Button() {}
    
    void draw() {
        ofFill();
        ofSetColor(pressed ? onColor : offColor);
        ofDrawRectRounded(area, 10);
        ofSetColor(textColor);
        ofDrawBitmapString(text, area.x + 10, area.y + area.height * 0.5);
    }
    
    void mousePressed(int x, int y, int btn) {
        if (area.inside(x, y)) {
            pressed = true;
        }
    }
    
    void mouseDragged(int x, int y, int btn) {
        
    }
    
    bool mouseReleased(int x, int y, int btn) {
        bool clicked = area.inside(x, y) && pressed;
        pressed = false;
        return clicked;
    }
};

#endif /* Button_h */
