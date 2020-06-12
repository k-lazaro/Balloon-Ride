
//--------------------------------------------------------------
//
//  CS134 - Game Development
//
//  Project 2 - 3D Spatial Data Structures
// 
//  Demonstration of a KdTree and its interaction with the mouse.
//  The KdTree can be either divided into equal-sized boxes or
//  into balanced boxes by obtaining the median in each sorted
//  vertices from its axis.
//
//  To change from Moon to Mars:
//  1. Comment line 70 and uncomment line 71 in ofApp.cpp
//  2. Comment line 88 and uncomment line 89 in ofApp.cpp (Changes radius of sphere drawn)
//  3. Comment lines 335-338 and uncomment lines 347-350 in KdTree.cpp
//
//  To change from equal divided boxes to balanced split boxes:
//  - Comment line 210 and uncomment line 211 in KdTree.cpp
//
//  To change amount of levels for KdTree to build:
//  - Change line 79, 2nd argument in ofApp.cpp
//
//
//  Kevin Smith   10-20-19
//
//  Student Name:   Kenny Lazaro
//  Date: <04-28-20>


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>
#include<algorithm>




//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	

	top.setPosition(0, 25, 0);
	top.lookAt(glm::vec3(0, 0, 0));
	top.setNearClip(.1);
	top.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    
    // set up physics particle
    //
    physParticle.position = lander.getPosition();   // Starts at position of lander
    physParticle.radius = 0;
    physParticle.lifespan = 100000;
    
    sys.add(physParticle);
    sys.addForce(new TurbulenceForce(ofVec3f(0, 1, 0), ofVec3f(0, 5, 0)));  // Add forces
    sys.addForce(new GravityForce(ofVec3f(0, -3, 0)));
    verticalThruster = new ThrusterForce(ofVec3f(0, 0, 0));
    sys.addForce(verticalThruster);                     // Separate forces for veritcal and xz plane
    xzThruster = new ThrusterForce(ofVec3f(0, 0, 0));
    sys.addForce(xzThruster);
    
    // Heading vector used for arrow key movement
    //
    heading = ofVec3f(3.5, 0, 0);
    
    // Exhaust particle emitter used when thruster is activated
    //
    exhaust.setPosition(lander.getPosition());
    exhaust.setEmitterType(DiscEmitter);
    exhaust.setVelocity(ofVec3f(0, -3, 0));
    exhaust.setParticleRadius(0.02);
    exhaust.radius = 0.25;
    exhaust.setLifespan(0.3);
    exhaust.setRate(0);
    exhaust.setGroupSize(10);
    exhaust.particleColor = ofColor::yellow;
    exhaust.start();

	theCam = &cam;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	terrain.loadModel("geo/canthisjustwork.obj");
    //terrain.loadModel("geo/mars-5k.obj");
	terrain.setScaleNormalization(false);
	boundingBox = meshBounds(terrain.getMesh(0));
    
    // load lander model
    //
    if (lander.loadModel("geo/hot_air_baloon_4.obj")) {
        lander.setScaleNormalization(false);
        //lander.setScale(.5, .5, .5);
        //    lander.setRotation(0, -180, 1, 0, 0);
        lander.setPosition(0, 3, 0);

        bLanderLoaded = true;
    }
    else {
        cout << "Error: Can't load model" << "geo/hot_air_baloon_4.obj" << endl;
        ofExit(0);
    }

    
    // create KdTree for terrain
    //
   // timeStartedForTreeBuild = ofGetElapsedTimeMillis();
    kdtree.create(terrain.getMesh(0), 40);                      // 40 Levels for moon, 20 for mars
    //timeEndedForTreeBuild = ofGetElapsedTimeMillis();
    //cout << "Time to build tree for Moon at 20 levels: " << timeEndedForTreeBuild - timeStartedForTreeBuild << " milliseconds" << endl;
    
    // gui setup
    gui.setup();
    gui.add(levels.setup("Levels", 2, 2, 20));
    prevValue = levels;
    
    sphere.setRadius(7.0);                                      // 7 for Moon
    //sphere.setRadius(.3);                                         // .3 for Mars
    
    
//    // TESTING FOR VERTICES SORTING
//    for (int i = 0; i < kdtree.root.points.size(); i++) {
//        sort.push_back(kdtree.mesh.getVertex(kdtree.root.points[i]));
//    }
//    kdtree.sortVertices(0, sort);      // Sort in x
//    //sortVertices(1, sort);      // Sort in y
//    kdtree.sortVertices(2, sort);      // Sort in z
//    for (int j = 0; j < sort.size(); j++) {
//        cout << j << ": " << sort[j] << endl;
//    }
//    kdtree.subDivideMedianInZ(kdtree.root.box, sort.at(sort.size()/2).z, medianSplit);
//    cout << sort.at(sort.size()/2).z << endl;
    
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    // If 'A' or 'D' is pressed, rotate heading vector and lander model
    //
    if (bAKeyDown == true) {
        angle += 0.5;
        heading.rotate(0.5, ofVec3f(0, 1, 0));
    }
    else if (bDKeyDown == true) {
        angle -= 0.5;
        heading.rotate(-0.5, ofVec3f(0, 1, 0));
    }
    lander.setRotation(0, angle, 0, 1, 0);
    lander.setPosition(sys.particles[0].position.x, sys.particles[0].position.y, sys.particles[0].position.z);          // Set position to follow particle
    
    
    // Condition checks for button presses
    //
    if (bWKeyDown == true) {
        verticalThruster->set(thrusterVerticalAcceleration);    // Positive vertical
        exhaust.setRate(100);
    }
    if (bSKeyDown == true) {
        verticalThruster->set(-thrusterVerticalAcceleration);   // Negative vertical
        exhaust.setRate(100);
    }
    if (bWKeyDown == false && bSKeyDown == false) {
        verticalThruster->set(ofVec3f(0, 0, 0));                // No acceleration
        exhaust.setRate(0);
    }
    
    if (bUpKeyDown == true) {
        xzThruster->set(ofVec3f(heading.x, 0, heading.z));      // Forward
        exhaust.setRate(100);
    }
    
    if (bDownKeyDown == true) {
        xzThruster->set(ofVec3f(-heading.x, 0, -heading.z));    // Backward
        exhaust.setRate(100);
    }
    
    if (bLeftKeyDown == true) {
        xzThruster->set(ofVec3f(heading.z, 0, -heading.x));     // Left
        exhaust.setRate(100);
    }
    
    if (bRightKeyDown == true) {
        xzThruster->set(ofVec3f(-heading.z, 0, heading.x));     // Right
        exhaust.setRate(100);
    }
    
    if (bUpKeyDown == false && bDownKeyDown == false && bLeftKeyDown == false && bRightKeyDown == false) {
        xzThruster->set(ofVec3f(0, 0, 0));
    }
    
    // Update particle systems/emitter
    //
    sys.update();
    exhaust.setPosition(lander.getPosition());
    exhaust.update();
}
//--------------------------------------------------------------
void ofApp::draw(){
    
	ofBackground(ofColor::black);

    ofEnableDepthTest();
	theCam->begin();

	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		terrain.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		terrain.drawFaces();

		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());

			ofVec3f min = lander.getSceneMin() + lander.getPosition();
			ofVec3f max = lander.getSceneMax() + lander.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

			landerBounds = bounds;
            
            // set color of bounding box based on selection status
            if (bLanderSelected) ofSetColor(ofColor::red);
            else ofSetColor(ofColor::white);

			drawBox(bounds);
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		terrain.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
    // FINAL IMPLEMENTATION
    if (bPointSelected) {
       ofSetColor(ofColor::red);
        glm::vec3 vertex = kdtree.mesh.getVertex(nodeSelected.points[0]);
        sphere.setPosition(vertex.x, vertex.y, vertex.z);
        sphere.draw();
        ofSetColor(ofColor::white);
    }
	
	ofNoFill();
	ofSetColor(ofColor::white);
	//drawBox(boundingBox);

    
    // Draws from 0 to levels, used in final implementation
    kdtree.draw(kdtree.root, levels, 0);
    
    
    // DIAGNOSTIC FOR ROOT CHILD BOXES
//    ofSetColor(ofColor::blue);
//    drawBox(medianSplit[0]);
//    ofSetColor(ofColor::red);
//    drawBox(medianSplit[1]);
    //
    
    // Draw particle systems/emitter
    sys.draw();
    exhaust.draw();
    
	theCam->end();

    ofDisableDepthTest();
    gui.draw();
    
}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'r':
		cam.reset();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
    case 'd':     // rotate spacecraft clockwise (about Y (UP) axis)
        bDKeyDown = true;
        break;
    case 'a':     // rotate spacecraft counter-clockwise (about Y (UP) axis)
        bAKeyDown = true;
        break;
	case 'w':   // spacecraft thrust UP
//		toggleWireframeMode();
        bWKeyDown = true;
		break;
    case 's':     // spacefraft thrust DOWN
//        savePicture();
        bSKeyDown = true;
        break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &top;
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
    case 'a':
        bAKeyDown = false;
    case 'd':
        bDKeyDown = false;
    case 'w':
        bWKeyDown = false;
        break;
    case 's':
        bSKeyDown = false;
        break;
    case OF_KEY_UP:    // move forward
        bUpKeyDown = false;
        break;
    case OF_KEY_DOWN:   // move backward
        bDownKeyDown = false;
        break;
    case OF_KEY_LEFT:   // move left
        bLeftKeyDown = false;
        break;
    case OF_KEY_RIGHT:   // move right
        bRightKeyDown = false;
        break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
    //timeStartedForSelection = ofGetElapsedTimeMicros();             // Starts Timer when clicked
	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;
    
    glm::vec3 p = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
    glm::vec3 rayDir = glm::normalize(p - theCam->getPosition());
    
    // FINAL IMPLEMENTATION
    mouseRay = Ray(Vector3(p.x, p.y, p.z) , Vector3(rayDir.x, rayDir.y, rayDir.z));         // Used for mouse/terrain interaction
    bool intersection = kdtree.intersect(mouseRay, kdtree.root, nodeSelected);                                  // Returns node that is selected
    
//    if (intersection) {
//        timeEndedForSelection = ofGetElapsedTimeMicros();
//        cout << "Time for selection: " << timeEndedForSelection - timeStartedForSelection << " microseconds" << endl;
//    }
    
    if (nodeSelected.box.intersect(mouseRay, 0, 1000)) {
        bPointSelected = true;
    }

    
    // DEBUG FOR RAY/MOUSE
//    if (kdtree.root.children[1].children[1].box.intersect(mouseRay, 0, 1000)) {
//        bLanderSelected = true;             // Repurpose landerselected
//    }
    
    // Compute bounds
    //
    glm::vec3 min = lander.getSceneMin() + lander.getPosition();
    glm::vec3 max = lander.getSceneMax() + lander.getPosition();
    Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

    // Test interaction with ray
    // bool intersect(const Ray &, float t0, float t1) const;
    if (bounds.intersect(Ray(Vector3(p.x, p.y, p.z), Vector3(rayDir.x, rayDir.y, rayDir.z)), 0, 1000)) {
        bLanderSelected = true;
        mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
        mouseLastPos = mouseDownPos;
        bInDrag = true;
    }
    else {
        bLanderSelected = false;
    }
    

}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    // if moving camera, don't allow mouse interaction
    //
    if (cam.getMouseInputEnabled()) return;
    
        
    glm::vec3 p = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
    glm::vec3 rayDir = glm::normalize(p - theCam->getPosition());
        
    // ALLOWS FOR DRAGGING OF MOUSE
    mouseRay = Ray(Vector3(p.x, p.y, p.z) , Vector3(rayDir.x, rayDir.y, rayDir.z));         // Used for mouse/terrain interaction
    kdtree.intersect(mouseRay, kdtree.root, nodeSelected);                                  // Returns node that is selected

    
    if (bInDrag) {
        glm::vec3 landerPos = lander.getPosition();
        
        glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
        glm::vec3 delta = mousePos - mouseLastPos;
        landerPos += delta;
        
        lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
        mouseLastPos = mousePos;
    }


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    bInDrag = false;
    bLanderSelected = false;
    bPointSelected = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
//	if (lander.loadModel(dragInfo.files[0])) {
//		bLanderLoaded = true;
//		lander.setScaleNormalization(false);
//	//	lander.setScale(.5, .5, .5);
//		lander.setPosition(0, 0, 0);
//		//lander.setRotation(1, 180, 1, 0, 0);
//
//		// We want to drag and drop a 3D object in space so that the model appears
//		// under the mouse pointer where you drop it !
//		//
//		// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
//		// once we find the point of intersection, we can position the lander/lander
//		// at that location.
//		//
//
//		// Setup our rays
//		//
//		glm::vec3 origin = theCam->getPosition();
//		glm::vec3 camAxis = theCam->getZAxis();
//		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
//		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
//		float distance;
//
//		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
//
//		if (hit) {
//			// find the point of intersection on the plane using the distance
//			// We use the parameteric line or vector representation of a line to compute
//			//
//			// p' = p + s * dir;
//			//
//			glm::vec3 intersectPoint = origin + distance * mouseDir;
//
//			// Now position the lander's origin at that intersection point
//			//
//		    glm::vec3 min = lander.getSceneMin();
//			glm::vec3 max = lander.getSceneMax();
//			float offset = (max.y - min.y) / 2.0;
//			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);
//
//			// set up bounding box for lander while we are at it
//			//
//			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
//		}
//	}
	

}


//  intersect the mouse ray with the normal plane
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = theCam->getPosition();
	glm::vec3 camAxis = theCam->getZAxis();
	glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;
		
		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
