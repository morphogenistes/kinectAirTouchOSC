//
//  Kinect.h
//  globefluxcontrol
//
//  Created by joseph larralde on 15/02/2025.
//

#ifndef Kinect_h
#define Kinect_h

#include "ofxOpenCv.h"
#include "ofxKinect.h"

class Kinect {
private:
    const float deg2rad = 3.1415926535 / 180.0;
    const float halfPi = 0.5 * 3.1415926535;
    
    int angle;
    float radAngle;
    
    int horizontalDistance;
    int planeDistance;
    ofVec4f planeCoefs;
    float planeNormalNorm; // optimize local distance to plane
    
    bool computeLocalDepth;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvContourFinder contourFinder;
    ofRectangle activeZone;
    ofxKinect k;
    std::vector<ofVec2f> cursors;

    // gui interaction
    ofRectangle kRect;
    ofRectangle normRect;
    ofRectangle depthPreview;
    ofRectangle blobsPreview;
    
    bool drawing;
    ofVec2f drawOrigin;
    ofVec2f drawEnd;
    ofRectangle activeZonePreview;
    
public:
    Kinect() :
        computeLocalDepth(true),
        depthPreview(10,10,400,300),
        blobsPreview(10,320,400,300)
    {}
    
    ~Kinect() {}
    
    void init() {
        k.init();
        k.open();
        
        if (k.isConnected()) {
            ofLogNotice() << "sensor-emitter dist: " << k.getSensorEmitterDistance() << "cm";
            ofLogNotice() << "sensor-camera dist:  " << k.getSensorCameraDistance() << "cm";
            ofLogNotice() << "zero plane pixel size: " << k.getZeroPlanePixelSize() << "mm";
            ofLogNotice() << "zero plane dist: " << k.getZeroPlaneDistance() << "mm";
        }
        
        angle = 0;
        setAngle(angle);
        
        setDistance(800);
        updatePlane();
        
        kRect = { 0, 0, static_cast<float>(k.width), static_cast<float>(k.height) };
        grayImage.allocate(k.width, k.height);
    }
    
    void close() {
        setAngle(0); // zero the tilt on exit
        k.close();
    }
    
    void update() {
        k.update();
        
        if (k.isFrameNew()) {
            grayImage.setFromPixels(k.getDepthPixels());
            ofPixels& pix = grayImage.getPixels();
            // ofLogNotice() << pix.size();
            
            for (auto x = 0; x < k.width; ++x) {
                for (auto y = 0; y < k.height; ++y) {
                    int dist = k.getDistanceAt(x, y);
                    if(dist > 0) {
                        ofVec3f d = k.getWorldCoordinateAt(x, y);
                        float dot = ofVec4f(d.x, d.y, d.z, 1.0).dot(planeCoefs);
                        if (dot > 0) {
                            pix[x + y * k.width] = 0;
                        } else {
                            if (computeLocalDepth) {
                                // see https://fr.wikipedia.org/wiki/Distance_d%27un_point_%C3%A0_un_plan
                                float depth = abs(dot) / planeNormalNorm;
                                float v = ofMap(depth, 0, horizontalDistance / 2, 0, 255);
                                pix[x + y * k.width] = ofClamp(v, 0, 255);
                            } else {
                                pix[x + y * k.width] = 255;
                            }
                        }
                    }
                }
            }
            
            // update the cv images
            grayImage.flagImageChanged();
            int dilatations = 3;
            for (auto i = 0; i < dilatations; ++i) { grayImage.dilate(); }
            for (auto i = 0; i < dilatations; ++i) { grayImage.erode(); }
            
            // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
            // also, find holes is set to true so we will get interior contours as well....
            contourFinder.findContours(grayImage, 900, (k.width * k.height) / 2, 2, false);
            
            cursors.clear();
            // and now map the blobs' centroids in the activeZone to [[-1,1],[-1,1]]
            for (auto i = 0; i < contourFinder.blobs.size(); ++i) {
                auto& b = contourFinder.blobs.at(i);
                ofVec2f pt = {
                    ofClamp(b.centroid.x, activeZone.getLeft(), activeZone.getRight()),
                    ofClamp(b.centroid.y, activeZone.getTop(), activeZone.getBottom())
                };
                pt.x = ofMap(pt.x, activeZone.getLeft(), activeZone.getRight(), -1, 1);
                pt.y = ofMap(pt.y, activeZone.getTop(), activeZone.getBottom(), -1, 1); // or 1, -1 ?
                cursors.push_back(pt);
            }
        }
    }
    
    void draw() {
        k.drawDepth(depthPreview);
        grayImage.draw(blobsPreview);
        contourFinder.draw(blobsPreview);

        ofSetLineWidth(1);
        ofNoFill();
        ofSetColor(255,255,0);
        ofDrawRectangle(activeZonePreview);

        if (drawing) {
            ofSetColor(255,127,0);
            ofDrawRectangle(drawOrigin.x, drawOrigin.y, drawEnd.x - drawOrigin.x, drawEnd.y - drawOrigin.y);
        }

        for (auto i = 0; i < contourFinder.blobs.size(); ++i) {
            auto& b = contourFinder.blobs.at(i);
            ofVec2f pt = scaleRectangleCoords(b.centroid, kRect, blobsPreview);
            pt.x = ofClamp(pt.x, activeZonePreview.getLeft(), activeZonePreview.getRight());
            pt.y = ofClamp(pt.y, activeZonePreview.getTop(), activeZonePreview.getBottom());
            ofSetColor(255,0,0);
            ofFill();
            ofDrawCircle(pt.x, pt.y, 10);
            ofSetColor(255,240,240);
            ofNoFill();
            ofDrawCircle(pt.x, pt.y, 10);
        }
        
        ofSetColor(255, 255, 255);
        stringstream reportStream;
        reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl;
        reportStream << "press LEFT and RIGHT to change the distance threshold: " << horizontalDistance << " mm" << endl;
        reportStream << "blobs: ";
        for (auto i = 0; i < contourFinder.nBlobs; ++i) {
            auto& b = contourFinder.blobs.at(i);
            reportStream << "\t" << i << "\t" << b.centroid.x << "\t" << b.centroid.y << endl;
        }
        ofDrawBitmapString(reportStream.str(), 20, 652);
    }
    
    std::vector<ofVec2f>& getCursors() { return cursors; }
    auto& getBlobs() { return contourFinder.blobs; }
    
    int getAngle() { return angle; }
    
    void setAngle(int a) {
        angle = a;
        updatePlane();
        k.setCameraTiltAngle(angle);
    }
    
    int getDistance() { return horizontalDistance; }
    
    void setDistance(int d) {
        horizontalDistance = d;
        updatePlane();
    }
    
    ofRectangle getActiveZone() { return activeZone; }
    
    void setActiveZone(ofRectangle area) {
        activeZone = area;
        ofVec2f tl = scaleRectangleCoords({
            activeZone.getLeft(),
            activeZone.getTop()
        }, kRect, blobsPreview);
        ofVec2f br = scaleRectangleCoords({
            activeZone.getRight(),
            activeZone.getBottom()
        }, kRect, blobsPreview);
        activeZonePreview = { tl.x, tl.y, br.x - tl.x, br.y - tl.y };
    }
    
    ofVec2f scaleRectangleCoords(ofVec2f pt, ofRectangle& src, ofRectangle& dst) {
        ofVec2f res;
        res.x = ofClamp(pt.x, src.getLeft(), src.getRight());
        res.y = ofClamp(pt.y, src.getTop(), src.getBottom());
        res.x = ofMap(res.x, src.getLeft(), src.getRight(), dst.getLeft(), dst.getRight());
        res.y = ofMap(res.y, src.getTop(), src.getBottom(), dst.getTop(), dst.getBottom());
        return res;
    }
    
    void updatePlane() {
        radAngle = -angle * deg2rad;
        planeDistance = static_cast<int>(static_cast<float>(horizontalDistance) / cos(radAngle));
        // we already have points (0,0,0) and (1,0,0) because it's our rotation axis
        // now we need the third point on the plane which is (0,1,0), rotated around x by radAngle radians
        // x is still 0, we get y and z with resp. sin(radAngle) = cos(π/2 - radAngle) and cos(radAngle) = sin(π/2 - radAngle)
        ofVec3f rotatedY(0,cos(radAngle),sin(radAngle));
        // now from those 3 points we need to define the equation of the plane
        // first we compute the cross product to get the normal to the plane
        ofVec3f normal = ofVec3f(1,0,0).getCrossed(rotatedY);
        // we also know a point of the plane : (0,0,planeDistance) so the plane equation is
        // normal.x * x + normal.y * y + normal.z * (z - planeDistance) = 0
        // => normal.x * x + normal.y * y + normal.z * z - normal.z * planeDistance = 0
        planeCoefs.set(normal.x, normal.y, normal.z, -normal.z * planeDistance);
        planeNormalNorm = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    }
    
    void keyPressed(int key) {
        switch (key) {
            case OF_KEY_UP:
                angle++;
                if (angle > 30) angle = 30;
                setAngle(angle);
                break;
                
            case OF_KEY_DOWN:
                angle--;
                if (angle < -30) angle = -30;
                setAngle(angle);
                break;
                
            case OF_KEY_LEFT:
                horizontalDistance -= 10;
                if (horizontalDistance < 0) horizontalDistance = 0;
                setDistance(horizontalDistance);
                break;
                
            case OF_KEY_RIGHT:
                horizontalDistance += 10;
                if (horizontalDistance > 2000) horizontalDistance = 2000;
                setDistance(horizontalDistance);
                break;
                
            default:
                break;
        }
    }
    
    void mousePressed(int x, int y, int btn) {
        if (blobsPreview.inside(x, y)) {
            drawing = true;
            drawOrigin.x = x;
            drawOrigin.y = y;
            drawEnd = drawOrigin;
        }
    }
    
    void mouseDragged(int x, int y, int btn) {
        if (drawing) {
            drawEnd.x = ofClamp(x, blobsPreview.getLeft(), blobsPreview.getRight());
            drawEnd.y = ofClamp(y, blobsPreview.getTop(), blobsPreview.getBottom());
        }
    }
    
    void mouseReleased(int x, int y, int btn) {
        if (drawing) {
            drawing = false;
            if (drawOrigin.distance(drawEnd) > 10) {
                activeZonePreview = {
                    drawOrigin.x,
                    drawOrigin.y,
                    drawEnd.x - drawOrigin.x,
                    drawEnd.y - drawOrigin.y
                };
                activeZonePreview.standardize();
                ofVec2f tl = scaleRectangleCoords({
                    activeZonePreview.getLeft(),
                    activeZonePreview.getTop()
                }, blobsPreview, kRect);
                ofVec2f br = scaleRectangleCoords({
                    activeZonePreview.getRight(),
                    activeZonePreview.getBottom()
                }, blobsPreview, kRect);
                activeZone = { tl.x, tl.y, br.x - tl.x, br.y - tl.y };
            }
        }
    }
};

#endif /* Kinect_h */
