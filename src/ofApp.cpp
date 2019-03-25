#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	// Initialize variables
	ofSetVerticalSync(true);
	transformB = true;
	lineLength = 100;
	globalEyeDistance = 50;
	originY = ofGetHeight() / 2;;
	originX = ofGetWidth() / 2;

	// Setup grabber
	grabber.setup(320, 240);

	//// Setup tracker
	tracker.setup(ofToDataPath("shape_predictor_68_face_landmarks.dat"));

	//osc
	//open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);
	isDebug = true;

	//initial param
	myTimer = 0;
	isSendable = false;
	noiseOffset = 15;
	noiseAngleOffset = 10 * 3.14159 / 180;
	trackerSize = 0;
	smoothedfaceX = ofGetWidth() * 0.5;
	smoothedfaceY = ofGetHeight() * 0.5;
	smoothedfaceW = ofGetWidth() * 0.3;
	smoothedfaceH = ofGetHeight() * 0.3;
	smoothedfacePitch = 0;
	smoothedfaceYaw = 0;
	smoothedfaceRoll = 0;
}

//--------------------------------------------------------------
void ofApp::update() {
	grabber.update();

	// Update tracker when there are new frames
	if (grabber.isFrameNew()) {
		tracker.update(grabber);
	}



}

//--------------------------------------------------------------
void ofApp::draw() {



	ofSetColor(255);

	ofPushMatrix();
	ofTranslate(grabber.getWidth(), 0);
	ofScale(-1, 1, 1);
	//grabber.setAnchorPoint(0, 0);
	if (isDebug) {
		grabber.draw(0, 0);
		tracker.drawDebug(0, 0);
	}

	ofPopMatrix();

	//cout << tracker.size() << endl;

	////find the biggest face
	//for (int num = 0; num < tracker.size(); ++num) {

	//}


	if (tracker.size() == 1) {
		for (auto instance : tracker.getInstances()) {
			isSendable = true;
			// initialize variables for pose decomposition
			ofVec3f scale, transition;
			ofQuaternion rotation, orientation;
			// get pose matrix
			ofMatrix4x4 p = instance.getPoseMatrix();
			// decompose the modelview
			ofMatrix4x4(p).decompose(transition, rotation, scale, orientation);

			ofPushView();
			ofPushMatrix();

			// obtain pitch, roll, yaw
			pitch = atan2(2 * (rotation.y()*rotation.z() + rotation.w()*rotation.x()), rotation.w()*rotation.w() - rotation.x()*rotation.x() - rotation.y()*rotation.y() + rotation.z()*rotation.z());
			roll = atan2(2 * (rotation.x()*rotation.y() + rotation.w()*rotation.z()), rotation.w()*rotation.w() + rotation.x()*rotation.x() - rotation.y()*rotation.y() - rotation.z()*rotation.z());
			yaw = asin(-2 * (rotation.x()*rotation.z() - rotation.w()*rotation.y()));



			if (ofRadToDeg(roll) > 0) roll = -3.14159 + roll;
			else roll = 3.14159 + roll;

			pitch = -pitch;
			roll = -roll;

			// get point of sellion - place our pose marker here
			ofPoint sellion = instance.getLandmarks().getImagePoint(27);
			// mirror image
			ofTranslate(grabber.getWidth(), 0);
			ofScale(-1, 1, 1);

			// use  as image center point
			//grabber.setAnchorPoint(sellion.x, sellion.y);

			// transpose to new origin;
			//ofTranslate(originX, originY);

			// calculate the distance between booth eyes
			ofPoint centerLeftEye = instance.getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE).getCentroid2D();
			ofPoint centerRightEye = instance.getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE).getCentroid2D();
			float localEyeDistance = centerLeftEye.distance(centerRightEye);

			// scale in relation to the global eye distance
			float scaleFactor = globalEyeDistance / localEyeDistance;
			//ofScale(scaleFactor, scaleFactor);

			ofTranslate(sellion.x, sellion.y);

			// apply rotation from pose matrix (need to flip the z-axis)
			ofScale(1, 1, -1);
			ofRotateX(ofRadToDeg(pitch));
			ofRotateY(ofRadToDeg(yaw));
			ofRotateZ(ofRadToDeg(roll));

			ofVec3f all;
			all.set(ofRadToDeg(-pitch), ofRadToDeg(yaw), ofRadToDeg(-roll));
			//cout << all << endl;


			if (isDebug) {
				// draw a rectangle to indicate when coordinate system flips (why?)
				ofSetColor(255, 255, 0, 50);
				ofDrawRectangle(0, 0, lineLength, lineLength);

				// draw our pose marker
				ofSetLineWidth(1);
				ofSetColor(255);
				ofSetColor(255, 0, 0);
				ofDrawLine(-lineLength, 0, 0, lineLength, 0, 0);
				ofSetColor(0, 255, 0);
				ofDrawLine(0, -lineLength, 0, 0, lineLength, 0);
				ofSetColor(0, 0, 255);
				ofDrawLine(0, 0, 0, 0, 0, -lineLength);
				ofSetLineWidth(1);
				ofPopMatrix();
				ofPopView();

			}

			//cout << instance.getBoundingBox().getTopLeft() << endl;
			boxX = ofGetWidth() - instance.getBoundingBox().getCenter().x;
			boxY = instance.getBoundingBox().getCenter().y;
			boxW = instance.getBoundingBox().getWidth();
			boxH = instance.getBoundingBox().getHeight();

			//caluclate before sending out via osc
			if (abs(smoothedfaceX - boxX) > noiseOffset
				|| abs(smoothedfaceY - boxY) > noiseOffset
				|| abs(smoothedfaceW - boxW) > noiseOffset
				|| abs(smoothedfacePitch - pitch) > noiseAngleOffset
				|| abs(smoothedfaceYaw - yaw) > noiseAngleOffset
				|| abs(smoothedfaceRoll - roll)> noiseAngleOffset)
			{

				smoothedfaceX = ofLerp(smoothedfaceX, boxX, 0.02);
				smoothedfaceY = ofLerp(smoothedfaceY, boxY, 0.02);
				smoothedfaceW = ofLerp(smoothedfaceW, boxW, 0.02);
				smoothedfaceH = ofLerp(smoothedfaceH, boxH, 0.02);
				smoothedfacePitch = ofLerp(smoothedfacePitch, pitch, 0.02);
				smoothedfaceYaw = ofLerp(smoothedfaceYaw, yaw, 0.02);
				smoothedfaceRoll = ofLerp(smoothedfaceRoll, roll, 0.02);

				/*cout << smoothedfaceX << endl;
				cout << smoothedfaceY << endl;
				cout << smoothedfaceW << endl;
				cout << smoothedfaceH << endl;

				cout << "===================================" << endl;*/


				//calucalte -roll



			}


		}

	}

	//more than one face detected = no face
	/*trackerSize = tracker.size();
	if (trackerSize > 1) {
	trackerSize = 0;
	}
	*/

	//add timer to remain normal in case face lost
	if (tracker.size() == 1) {
		myTimer = 0;
		trackerSize = 1;
	}
	else {
		myTimer++;
		if (myTimer > 200) {
			trackerSize = ofLerp(trackerSize, 0, 0.02);
		}
	}


	//osc
	if (isSendable && smoothedfaceX < ofGetWidth() && smoothedfaceY < ofGetHeight() && smoothedfaceW < ofGetWidth()
		&& fabs(smoothedfacePitch) < 30 && fabs(smoothedfaceYaw) < 30 && fabs(smoothedfaceRoll) < 30) {
		ofxOscMessage m;
		m.setAddress("/face");


		m.addFloatArg(smoothedfaceX);                  //1
		m.addFloatArg(smoothedfaceY);                  //2
		m.addFloatArg(smoothedfaceW);                  //3
		m.addFloatArg(ofRadToDeg(smoothedfacePitch));  //4
		m.addFloatArg(ofRadToDeg(smoothedfaceYaw));    //5
		m.addFloatArg(ofRadToDeg(smoothedfaceRoll));   //6
		m.addFloatArg(trackerSize);                    //7
													   //m.addFloatArg(ofMap(ofGetMouseY(), 0, ofGetHeight(), 0.f, 1.f, true));
		sender.sendMessage(m, false);
		//cout << m << endl;


	}

	if (trackerSize == 0) {
		isSendable = false;
	}








}

