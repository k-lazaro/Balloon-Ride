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





class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

        void checkCollisions();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
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
		void drawBox(const Box &box);
		void loadVbo();
		Box meshBounds(const ofMesh &);
    

        ofImage sky;
        ofSoundPlayer fire;
        ofSoundPlayer descent;

		ofEasyCam cam;
		ofCamera top;
        ofCamera viewCam;
        ofCamera insideCam;
		ofCamera *theCam;
		ofxAssimpModelLoader terrain, lander;
		ofLight light;
		Box boundingBox;
		Box landerBounds;
	
		bool bAltKeyDown;
		bool bCtrlKeyDown;
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
        float angle;
		
		bool bLanderLoaded = false;
		bool bTerrainSelected;
		bool bLanderSelected = false;
        bool bInDrag = false;
        bool bCrashed = false;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
        ofxPanel gui;
        ofxIntSlider levels;
        int prevValue;
		
		glm::vec3 mouseDownPos;
        glm::vec3 mouseLastPos;


		const float selectionRange = 4.0;
    
        KdTree kdtree;
        Octree octree;
        ofSpherePrimitive sphere;
    
    
        Particle physParticle;
        ParticleSystem sys;
        ParticleEmitter exhaust;
        ParticleEmitter *explosion;                    // Starts whenever bullet collides with enemy
    
        ThrusterForce* verticalThruster;
        ThrusterForce* xzThruster;
        ofVec3f thrusterVerticalAcceleration = ofVec3f(0, 3.5, 0);
    
        ofVec3f heading;
        vector<ofVec3f> points;
        vector<TreeNode> hitBoxes;
        Ray sensor;
        TreeNode altitudeIntersect;
        float altitude;
    
        vector<Landing> zones;

		// textures
		//
		ofTexture  particleTex;

		// shaders
		//
		ofVbo vbo;
		ofVbo vbo1;
		ofShader shader;

		ofLight keyLight, fillLight, rimLight;

		float fuel;
		bool gameReset;
		bool gameDone;
		bool gameStart;
		GravityForce* deadForce;

		ofImage startScreen;			// First screen we see
		ofImage endScreen;				// Last screen we see before we can restart

};

