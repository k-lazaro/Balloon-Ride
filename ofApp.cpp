
//--------------------------------------------------------------
//
//  CS134 - Game Development
//
//  Project 3 - Final
// 
// Implemented a total of 3 landing areas by making our own class to help us check if our lander
// is inside
//
// Implemented vehicle manuevering with physics through the use of ThrusterForces
//
// Implemented Altitude Sensor by having a ray shoot out from the bottom of our lander and checking
// when it comes into contact with a node from our octree
//
// Implemented visual effect rendering using OpenGL Shaders (The exhaust in the balloon and the explosion effects)
//
// 1st Use of Physics: Movement
// 2nd Use of Physics: Exhaust
// 3rd Use of Physics: Explosion when collision
//
//  Game Lighting:
//
//
//
//
// Supports multiple camera such as
// 1: Free Cam
// 2: Top camera
// 3: Inside camera, where you can act like you're inside (controllable with mouse!)
// 4: Afar Camera, where we can view the balloon from the ground
//
// Has Collision Detection
// Will have a bounce effect due to ImpulseForce when it collides with a surface

//  A lot of this code's foundation is thanks to Kevin Smith
//
//  Student Name:   Kenny Lazaro && Kevin Viet Nguyen
//  Date: <05-15-20>


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>
#include<algorithm>




//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {

    fuel = 500000;
   
    // Boolean Setups
    gameDone = false;
    gameStart = false;
    hideGui = true;
    bWireframe = false;
    bDisplayPoints = false;
    bAltKeyDown = false;
    bCtrlKeyDown = false;
    bLanderLoaded = false;
    bTerrainSelected = true;
    //	ofSetWindowShape(1024, 768);


    // Free Cam Setup
    cam.setDistance(10);
    cam.setNearClip(.1);
    cam.setPosition(3, 3, 3);
    cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    ofSetVerticalSync(true);
    cam.enableMouseInput();
    ofEnableSmoothing();

    // Cameras set up (Top)
    top.setPosition(0, 150, 0);
    top.lookAt(glm::vec3(0, 0, 0));
    top.setNearClip(.1);
    top.setFov(65.5);   // approx equivalent to 28mm in 35mm format

    // Cameras set up (Inside)
    insideCam.setPosition(ofVec3f(0, 0, 0));
    insideCam.lookAt(heading);
    insideCam.setNearClip(.1);
    insideCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format

    // set up physics particle
    //
    physParticle.position = ofVec3f(0, 30, 0);   // Starts at position of lander
    physParticle.radius = 0;
    physParticle.lifespan = 100000;

    // texture loading
    //
    ofDisableArbTex();     // disable rectangular textures

    // load textures
    //
    if (!ofLoadImage(particleTex, "images/dot.png")) {
        cout << "Particle Texture File: images/dot.png not found" << endl;
        ofExit();
    }

    // load the shader
    //
    #ifdef TARGET_OPENGLES
        shader.load("shaders_gles/shader");
    #else
        shader.load("shaders/shader");
    #endif

    // Adding forces to our particle to help with movement and making it more realistic
    sys.add(physParticle);
    sys.addForce(new TurbulenceForce(ofVec3f(0, 1, 0), ofVec3f(0, 5, 0)));  // Add forces
    sys.addForce(new GravityForce(ofVec3f(0, -3, 0)));
    deadForce = new GravityForce(ofVec3f(0, 0, 0));
    sys.addForce(deadForce);
    verticalThruster = new ThrusterForce(ofVec3f(0, 0, 0));
    sys.addForce(verticalThruster);                     // Separate forces for veritcal and xz plane
    xzThruster = new ThrusterForce(ofVec3f(0, 0, 0));
    sys.addForce(xzThruster);

    // Heading vector used for arrow key movement
    //
    heading = ofVec3f(3.5, 0, 0);

    // Exhaust particle emitter used when thruster is activated
    //
    //exhaust.setPosition(lander.getPosition() + ofVec3f(-.4, 4.5, .08));
    exhaust.setEmitterType(DiscEmitter);
    exhaust.setVelocity(ofVec3f(0, 3, 0));
    exhaust.setParticleRadius(0.02);
    exhaust.radius = 0.25;
    exhaust.setLifespan(0.3);
    exhaust.setRate(0);
    exhaust.setGroupSize(10);
    exhaust.particleColor = ofColor::red;
    exhaust.start();

    // Set-up for Explosion effect when collides with surface
    explosion = new ParticleEmitter(new ParticleSystem());
    explosion->sys->addForce(new ImpulseRadialForce(100));
    explosion->setOneShot(true);
    explosion->fired = false;
    explosion->setEmitterType(RadialEmitter);
    explosion->setGroupSize(20);

    // Initializing the camera you get when you first get into the game
    theCam = &insideCam;

    // setup rudimentary lighting 
    //
    initLightingAndMaterials();

    // Loading terrain
    terrain.loadModel("geo/minecraft.obj");
    //terrain.loadModel("geo/mars-5k.obj");
    terrain.setScaleNormalization(false);
    boundingBox = meshBounds(terrain.getMesh(0));

    // Set up "tracking" camera
    viewCam.setPosition((boundingBox.max().x() - boundingBox.min().x()) / 2 + boundingBox.min().x(),
        boundingBox.max().y(), boundingBox.max().z());
    viewCam.setNearClip(.1);
    viewCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format

    // load lander model
    //
    if (lander.loadModel("geo/texturedBalloon.obj")) {
        lander.setScaleNormalization(false);
        //lander.setScale(.5, .5, .5);
        //    lander.setRotation(0, -180, 1, 0, 0);
        lander.setPosition(0, 30, 0);

        bLanderLoaded = true;
    }
    else {
        cout << "Error: Can't load model" << "geo/texturedBalloon.obj" << endl;
        ofExit(0);
    }
    ofVec3f min = lander.getSceneMin() + lander.getPosition();
    ofVec3f max = lander.getSceneMax() + lander.getPosition();
    points.push_back(ofVec3f(min.x, min.y, min.z));
    points.push_back(ofVec3f(min.x, min.y, max.z));
    points.push_back(ofVec3f(max.x, min.y, min.z));
    points.push_back(ofVec3f(max.x, min.y, max.z));
    sensor = Ray(Vector3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z), Vector3(0, -100, 0));


    // create KdTree for terrain
    //kdtree.create(terrain.getMesh(0), 40);                      // 40 Levels for moon, 20 for mars

    // create Octree
    octree.create(terrain.getMesh(0), 7);

    // gui setup
    gui.setup();
    gui.add(levels.setup("Levels", 2, 2, 20));
    prevValue = levels;

    sphere.setRadius(7.0);                                      // 7 for Moon
    //sphere.setRadius(.3);                                         // .3 for Mars

    // Light Setup
    ofEnableLighting();
    // Setup 3 - Light System
    //

    keyLight.setup();
    keyLight.enable();
    keyLight.setAreaLight(2, 2);
    keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    keyLight.setSpecularColor(ofFloatColor(1, 1, 1));
    keyLight.setPosition(85, 15, 105);

    fillLight.setup();
    fillLight.enable();
    fillLight.setSpotlight();
    fillLight.setScale(.05);
    fillLight.setSpotlightCutOff(95);
    fillLight.setAttenuation(2, .001, .001);
    fillLight.setAmbientColor(ofFloatColor(255, 69, 0.1));
    fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
    fillLight.rotateDeg(-10, ofVec3f(1, 0, 0));
    fillLight.setPosition(0 ,0 ,180);

    rimLight.setup();
    rimLight.enable();
    rimLight.setSpotlight();
    rimLight.setScale(.05);
    rimLight.setSpotlightCutOff(30);
    rimLight.setAttenuation(.2, .001, .001);
    rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
    rimLight.rotateDeg(-90, glm::vec3(0, 1, 0));
    rimLight.setPosition(lander.getPosition());


    // Background load
    sky.load("images/skies.png");
    sky.resize(ofGetWindowWidth(), ofGetWindowHeight());

    // Sound effects load
    fire.load("sounds/qubodupFireLoop.ogg");
    descent.load("sounds/wind woosh loop.ogg");

    // Screen loads
    startScreen.load("images/startScreen.png");
    endScreen.load("images/endScreen.png");
    winScreen.load("images/winScreen.png");

    // Landing zones
    zones.push_back(Landing(-5, 5, -70, -60));               // 10x10
    zones.push_back(Landing(-75, -65, 60, 70));
    zones.push_back(Landing(80, 90, 20, 30));

}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
    if (exhaust.sys->particles.size() < 1) {}
    else {
        vector<ofVec3f> sizes;
        vector<ofVec3f> points;
        for (int i = 0; i < exhaust.sys->particles.size(); i++) {
            points.push_back(exhaust.sys->particles[i].position);
            sizes.push_back(ofVec3f(5));
        }
        // upload the data to the vbo
        //
        vbo.clear();
        int total = (int)points.size();
        vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
        vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
    }
    if (explosion->sys->particles.size() < 1) {}
    else {
        //cout << "explosion system's particle size: " << explosion->sys->particles.size() << endl;
        vector<ofVec3f> sizes;
        vector<ofVec3f> points;
        for (int i = 0; i < explosion->sys->particles.size(); i++) {
            points.push_back(explosion->sys->particles[i].position);
            sizes.push_back(ofVec3f(5));
        }
        // upload the data to the vbo
        //
        vbo1.clear();
        int total = (int)points.size();
        vbo1.setVertexData(&points[0], total, GL_STATIC_DRAW);
        vbo1.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
    }
}


void ofApp::checkCollisions() {
    // only bother to check for descending particles.
    //
    ofVec3f vel = sys.particles[0].velocity; // velocity of particle
    if (vel.y >= 0)
    {
        return;
    }
    else {
        for (int i = 0; i < hitBoxes.size(); i++) {
            ofVec3f norm = ofVec3f(0, 1, 0);
            sys.addForce(new ImpulseForce(sys.particles[0].velocity, norm));
        }
    }

}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

    // Checks if all game win conditions are good
    if (zones[0].landed == true && zones[1].landed == true && zones[2].landed == true)
    {
        gameWon = true;
    }

    // If we collided for too long, we will die
    if (!bLanded && timeExploaded > 0) {
        if (ofGetElapsedTimeMillis() - timeExploaded > 3500)
            gameDone = true;
    }

    ofSeedRandom();
    rimLight.setPosition(lander.getPosition());
    // Every update, we will lose fuel
    fuel--;

    // If fuel is 0, we will begin falling due to gravity
    if (fuel <= 0)
    {
        deadForce->set(ofVec3f(0, -10, 0));
    }

    // As we fall, we will die
    if (fuel <= -250)
    {
        gameDone = true;
    }

    //cout << "x: " << lander.getPosition().x << " z: "<< lander.getPosition().z << endl;


    // If 'A' or 'D' is pressed, rotate heading vector and lander model
    //
    rimLight.setPosition(lander.getPosition());
    if (bAKeyDown == true) {
        angle += 0.5;
        heading.rotate(0.5, ofVec3f(0, 1, 0));
        rimLight.rotateDeg(0.5, glm::vec3(0, 1, 0));
    }
    else if (bDKeyDown == true) {
        angle -= 0.5;
        heading.rotate(-0.5, ofVec3f(0, 1, 0));
        rimLight.rotateDeg(-0.5, glm::vec3(0, 1, 0));
    }
    lander.setRotation(0, angle, 0, 1, 0);
    lander.setPosition(sys.particles[0].position.x, sys.particles[0].position.y, sys.particles[0].position.z);          // Set position to follow particle



    // Updates bottom contact points
    ofVec3f min = lander.getSceneMin() + lander.getPosition();
    ofVec3f max = lander.getSceneMax() + lander.getPosition();
    points[0] = ofVec3f(min.x, min.y, min.z);
    points[1] = ofVec3f(min.x, min.y, max.z);
    points[2] = ofVec3f(max.x, min.y, min.z);
    points[3] = ofVec3f(max.x, min.y, max.z);

    // Camera updates
    insideCam.setPosition(ofVec3f((max.x - min.x)/2 + min.x, min.y, (max.z - min.z)/2 + min.z));
    //insideCam.setPosition(ofVec3f(sys.particles[0].position.x, sys.particles[0].position.y + ((max - min) / 2).y, sys.particles[0].position.z));
    //insideCam.lookAt(glm::vec3(heading.x * 100, sys.particles[0].position.y + ((max - min)/2).y, heading.z * 100));

    if (theCam == &insideCam) {
        if (bInsideCamMouse) {          // Control inside cam with mouse
            // Center of camera is at windowWidth/2 and windowHeight/2
            if (mouseX > 550 && mouseX < 700 && mouseY > 300 && mouseY < 500) {
                insideCam.lookAt(theCam->screenToWorld(glm::vec3(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, 0)));
            }
            else {
                ofVec3f centerToMouse = ofVec3f(mouseX - ofGetWindowWidth() / 2, mouseY - ofGetWindowHeight() / 2, 0);     // Vector from center to mouse
                centerToMouse = centerToMouse.getNormalized() * 4;          // Normalize and multiply by constant
                insideCam.lookAt(theCam->screenToWorld(glm::vec3(ofGetWindowWidth() / 2 + centerToMouse.x, ofGetWindowHeight() / 2 + centerToMouse.y, 0)));
            }
        }
        else {                      // Inside cam follows heading vector
            insideCam.lookAt(glm::vec3(heading.x * 100, sys.particles[0].position.y + ((max - min) / 4).y, heading.z * 100));
        }

    }

    //cout << "(" << mouseX << ", " << mouseY << ")" << endl;
    viewCam.lookAt(sys.particles[0].position);


    // Altitude checking
    //sensor = Ray(Vector3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z), Vector3(0, -100, 0));
    sensor = Ray(Vector3((max.x - min.x) / 2 + min.x, min.y, (max.z - min.z) / 2 + min.z), Vector3(0, -100, 0));
    //kdtree.intersect(sensor, kdtree.root, altitudeIntersect);
    octree.intersect(sensor, octree.root, altitudeIntersect);
    float topOfBox = altitudeIntersect.box.max().y();
    altitude = lander.getPosition().y - topOfBox;



    // Tests for crashing
    hitBoxes.clear();
    for (int i = 0; i < points.size(); i++) {
        TreeNode hitBox;
        //kdtree.pointIntersect(points[i], kdtree.root, hitBox);
        octree.pointIntersect(points[i], octree.root, hitBox);
        if (hitBox.points.size() != 0 && sys.particles[0].velocity.y < 0) {                // Checks if added TreeNode is not null
            hitBoxes.push_back(hitBox);
            for (int j = 0; j < zones.size(); j++) {
                bool l = zones[j].checkInside(lander.getPosition());                  // Check if zones are touched
                bLanded = l || bLanded;
            }
            if (!bLanded) {
                explosion->setPosition(lander.getPosition());
                explosion->start();
                timeExploaded = ofGetElapsedTimeMillis();
            }
            if (bLanded) {
                fuel = fuel + 50000;
                bLanded = false;
            }

        }
    }

    checkCollisions();


    // Condition checks for button presses
    //
    if (bWKeyDown == true) {
        verticalThruster->set(thrusterVerticalAcceleration);    // Positive vertical
        exhaust.setRate(100);
        if (!fire.isPlaying())
            fire.play();
        fuel = fuel - 50;
    }
    if (bSKeyDown == true) {
        verticalThruster->set(-thrusterVerticalAcceleration);   // Negative vertical
        // exhaust.setRate(100);
        if (!descent.isPlaying())
            descent.play();
        fuel = fuel - 50;
    }
    if (bWKeyDown == false && bSKeyDown == false) {
        verticalThruster->set(ofVec3f(0, 0, 0));                // No acceleration
        exhaust.setRate(0);
        fire.stop();
        descent.stop();
    }

    if (bUpKeyDown == true) {
        xzThruster->set(ofVec3f(heading.x, 0, heading.z));      // Forward
        exhaust.setRate(100);
        fuel = fuel - 50;
    }

    if (bDownKeyDown == true) {
        xzThruster->set(ofVec3f(-heading.x, 0, -heading.z));    // Backward
        exhaust.setRate(100);
        fuel = fuel - 50;
    }

    if (bLeftKeyDown == true) {
        xzThruster->set(ofVec3f(heading.z, 0, -heading.x));     // Left
        exhaust.setRate(100);
        fuel = fuel - 50;
    }

    if (bRightKeyDown == true) {
        xzThruster->set(ofVec3f(-heading.z, 0, heading.x));     // Right
        exhaust.setRate(100);
        fuel = fuel - 50;
    }

    if (bUpKeyDown == false && bDownKeyDown == false && bLeftKeyDown == false && bRightKeyDown == false) {
        xzThruster->set(ofVec3f(0, 0, 0));
    }

    // Update particle systems/emitter
    //
    sys.update();
    exhaust.setPosition(ofVec3f((max.x - min.x)/2 + min.x, (max.y - min.y)/7 + min.y, (max.z - min.z)/2 + min.z));
    exhaust.update();
    explosion->update();

    // Pop the impulse forces
    if (hitBoxes.size() != 0) {
        for (int j = 0; j < hitBoxes.size(); j++) {
            sys.forces.pop_back();
        }
    }

}
//--------------------------------------------------------------
void ofApp::draw() {

    loadVbo();
    sky.draw(0, 0, 0);


    ofEnableDepthTest();
    theCam->begin();


    ofPushMatrix();
    if (bWireframe) {                    // wireframe mode  (include axis)
        ofDisableLighting();
        ofSetColor(ofColor::slateGray);
        terrain.drawWireframe();
        if (bLanderLoaded && theCam != &insideCam) {
            lander.drawWireframe();
            if (!bTerrainSelected) drawAxis(lander.getPosition());
        }
        if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
    }
    else {
        ofEnableLighting();              // shaded mode
        terrain.drawFaces();

        if (bLanderLoaded && theCam != &insideCam) {
            lander.drawFaces();
            if (!bTerrainSelected) drawAxis(lander.getPosition());

            ofVec3f min = lander.getSceneMin() + lander.getPosition();
            ofVec3f max = lander.getSceneMax() + lander.getPosition();

            Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

            landerBounds = bounds;

            // set color of bounding box based on selection status
            if (bLanderSelected) ofSetColor(ofColor::red);
            else ofSetColor(ofColor::white);

            //drawBox(bounds);
        }
        if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
    }


    if (bDisplayPoints) {                // display points as an option    
        glPointSize(3);
        ofSetColor(ofColor::green);
        terrain.drawVertices();
    }



    ofNoFill();
    ofSetColor(ofColor::white);
    //drawBox(boundingBox);


    // Draws from 0 to levels, used in final implementation
    //kdtree.draw(kdtree.root, levels, 0);
    //octree.draw(octree.root, levels, 0);


    // Draw particle systems/emitter
    sys.draw();

    // Draw landing zones
    for (int i = 0; i < zones.size(); i++) {
        if (zones[i].landed) {
            ofSetColor(ofColor::green);
        }
        else {
            ofSetColor(ofColor::purple);
        }
        drawLanding(zones[i]);
    }


    ofSetColor(255, 100, 90);

    // this makes everything look glowy :)
    //
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnablePointSprites();


    // begin drawing in the camera
    //
    shader.begin();

    // draw particle emitter here..
    //
    particleTex.bind();
    vbo.draw(GL_POINTS, 0, (int)exhaust.sys->particles.size());
    ofSetColor(81, 81, 16);
    vbo1.draw(GL_POINTS, 0, (int)explosion->sys->particles.size());
    ofSetColor(255, 100, 90);
    particleTex.unbind();

    //  end drawing in the camera
    // 
    shader.end();

    ofDisablePointSprites();
    ofDisableBlendMode();
    ofEnableAlphaBlending();

    // set back the depth mask
    //
    glDepthMask(GL_TRUE);

    // Draw bottom contact points
    // ofSetColor(ofColor::red);
    // ofDrawSphere(points[0].x, points[0].y, points[0].z, 0.3);
    // ofDrawSphere(points[1].x, points[1].y, points[1].z, 0.3);
    // ofDrawSphere(points[2].x, points[2].y, points[2].z, 0.3);
    // ofDrawSphere(points[3].x, points[3].y, points[3].z, 0.3);
    // for (int i = 0; i < hitBoxes.size(); i++) {
        //drawBox(hitBoxes[i].box);
    //}
    ofSetColor(ofColor::white);


    theCam->end();

    ofDisableDepthTest();

    if (!hideGui) {
        gui.draw();
    }

    // Important info such as the altitude and fuel
    string str;
    str += "Altitude: " + std::to_string(altitude);
    ofSetColor(ofColor::white);
    ofDrawBitmapString(str, ofPoint(10, 50));
    string str2;
    str2 += "Fuel Left: " + std::to_string(fuel);
    ofSetColor(ofColor::white);
    ofDrawBitmapString(str2, ofPoint(10, 30));
    string str3;
    str3 += "Frame Rate: " + std::to_string(ofGetFrameRate());
    ofSetColor(ofColor::white);
    ofDrawBitmapString(str3, ofPoint(10, 70));

    // Drawing the screens 
    if (!gameStart)	// Drawing the start screen
    {
        startScreen.draw(0, 0);
    }

    if (gameDone)
    {
        endScreen.draw(0, 0);
    }
    if (gameWon)
    {
        winScreen.draw(0, 0);
    }
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
    case ' ':               // Starts the game
        gameStart = true;
        break;
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
    case 'h':                   // Resets the game
        sys.particles[0].position = ofVec3f(0, 30, 0);
        fuel = 500000;
        timeExploaded = 0;
        bLanded = false;
        deadForce->set(ofVec3f(0, 0, 0));
        gameDone = false;
        gameWon = false;
        ofSetColor(ofColor::purple);
        for (int i = 0; i < zones.size(); i++) {
            zones[i].landed = false;
        }
        break;
    case 'r':
        cam.reset();
        break;
    case 't':
        setCameraTarget();
        break;
    case 'u':
        hideGui = !hideGui;
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
    case OF_KEY_UP:    // move forward
        bUpKeyDown = true;
        break;
    case OF_KEY_DOWN:   // move backward
        bDownKeyDown = true;
        break;
    case OF_KEY_LEFT:   // move left
        bLeftKeyDown = true;
        break;
    case OF_KEY_RIGHT:   // move right
        bRightKeyDown = true;
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
    case OF_KEY_F1:                     // Key to have inside POV for cam
        theCam = &insideCam;
        bInsideCamMouse = false;
        break;
    case OF_KEY_F2:                     // Key to have inside POV for cam and mouse can be controlled
        theCam = &insideCam;
        bInsideCamMouse = true;
        break;
    case OF_KEY_F3:
        theCam = &top;                  // Key to have top POV for cam
        break;
    case OF_KEY_F4:                     // Key to have from afar POV for cam
        theCam = &viewCam;
        break;
    case OF_KEY_F5:                     // Key to have free cam
        theCam = &cam;
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
    case 'h':
        gameReset = false;
        break;
    default:
        break;

    }
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {



}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    //timeStartedForSelection = ofGetElapsedTimeMicros();             // Starts Timer when clicked
    // if moving camera, don't allow mouse interaction
    //
    if (cam.getMouseInputEnabled()) return;



    // Compute bounds
    //
    glm::vec3 min = lander.getSceneMin() + lander.getPosition();
    glm::vec3 max = lander.getSceneMax() + lander.getPosition();
    Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box& box) {
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

// Draws the landing zone
void ofApp::drawLanding(const Landing& landing) {
    Box b = Box(Vector3(landing.parameters[0], 0, landing.parameters[2]), Vector3(landing.parameters[1], 2, landing.parameters[3]));
    ofFill();
    drawBox(b);
    ofNoFill();
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh& mesh) {
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
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

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
    { 5.0, 5.0, 5.0, 0.0 };

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
