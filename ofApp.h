#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "ofxGui.h"
#include "box.h"
#include "ray.h"
#include "KdTree.h"
#include "Octree.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "landing.h"


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void checkCollisions();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
	void toggleSelectTerrain();
	void setCameraTarget();
	glm::vec3 getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm);
	void drawBox(const Box& box);
	void loadVbo();
	Box meshBounds(const ofMesh&);
	void drawLanding(const Landing& landing);

	// Cameras
	ofEasyCam cam;
	ofCamera top;
	ofCamera viewCam;
	ofCamera insideCam;
	ofCamera* theCam;

	// Model for terrain and etc
	ofxAssimpModelLoader terrain, lander;
	ofLight light;
	Box boundingBox;
	Box landerBounds;
	bool bWireframe;
	bool bDisplayPoints;

	// Controls
	bool bWKeyDown;
	bool bSKeyDown;
	bool bAKeyDown;
	bool bDKeyDown;
	bool bUpKeyDown;
	bool bDownKeyDown;
	bool bLeftKeyDown;
	bool bRightKeyDown;
	bool bAltKeyDown;
	bool bCtrlKeyDown;
	glm::vec3 mouseDownPos;
	glm::vec3 mouseLastPos;
	
	float angle;

	// Booleans for conditions that we will use in our code
	bool bLanderLoaded = false;
	bool bTerrainSelected;
	bool bLanderSelected = false;
	bool bInDrag = false;
	bool bInsideCamMouse = false;
	bool bLanded;
	bool gameReset;
	bool gameDone;
	bool gameStart;
	bool gameWon;
	bool hideGui;


	ofVec3f selectedPoint;
	ofVec3f intersectPoint;

	// GUI stuff
	ofxPanel gui;
	ofxIntSlider levels;
	int prevValue;
	ofxVec3Slider rimLightPost;

	const float selectionRange = 4.0;

	// Stuff for our kdTree/octTree
	KdTree kdtree;
	Octree octree;
	ofSpherePrimitive sphere;

	// Particle Stuff
	Particle physParticle;
	ParticleSystem sys;
	ParticleEmitter exhaust;
	ParticleEmitter* explosion;                    // Starts whenever bullet collides with enemy

	// Physics stuff
	ThrusterForce* verticalThruster;
	ThrusterForce* xzThruster;
	ofVec3f thrusterVerticalAcceleration = ofVec3f(0, 3.5, 0);
	GravityForce* deadForce;

	ofVec3f heading;

	// Collision
	vector<ofVec3f> points;
	vector<TreeNode> hitBoxes;

	// Altitude Stuff
	Ray sensor;
	TreeNode altitudeIntersect;
	float altitude;

	// Landing Area
	vector<Landing> zones;

	// textures
	//
	ofTexture  particleTex;

	// shaders
	//
	ofVbo vbo;
	ofVbo vbo1;
	ofShader shader;

	// Light
	ofLight keyLight, fillLight, rimLight;

	// Important info
	float fuel;
	float timeExploaded;

	// Images/Sounds
	ofImage startScreen;			// First screen we see
	ofImage endScreen;				// Last screen we see before we can restart (Lose)
	ofImage winScreen;				// Last screen we see before we can restart (Win)
	ofImage sky;
	ofSoundPlayer fire;
	ofSoundPlayer descent;

};

