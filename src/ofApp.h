#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxOsc.h"

// send host (aka ip address)
#define HOST "localhost"

/// send port
#define PORT 14500

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	ofVideoGrabber grabber;
	ofxFaceTracker2 tracker;

	bool transformB;
	float lineLength;
	float globalEyeDistance, originY, originX;


	ofTrueTypeFont font;
	ofxOscSender sender;
	ofBuffer imgAsBuffer;
	ofImage img;

	//for TD
	float smoothedfaceX, smoothedfaceY, smoothedfaceW, smoothedfaceH, smoothedfacePitch, smoothedfaceYaw, smoothedfaceRoll;
	int trackerSize;

	double pitch, roll, yaw;
	bool isDebug;
	int myTimer;

	//for calculation
	float noiseOffset, noiseAngleOffset;
	float boxX, boxY, boxW, boxH;

	bool isSendable;
	//biggest face width
	//vector<float> faceWs;
	//float biggerstFace;


};