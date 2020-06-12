//  KdTree Template - Simple KdTree class
//
//  SJSU  - CS134 Game Development
//
//  Kevin M. Smith   04/19/20

#pragma once
#include "ofMain.h"
#include "box.h"
#include "ray.h"

//  General purpose TreeNode class
//


class TreeNode {
public:
	Box box;
	vector<int> points;
	vector<TreeNode> children;    // for binary KdTree, this array has just two(2) members)
};

class KdTree {
public:
	
	// Create and initilize tree given an OpenFrameWorks Mesh and # of levels.  if numLevels
	// is larger than the number of levels possible in the data, then the tree will be built to
	// its maximum depth with only one point in each leaf node.
	//
	void create(const ofMesh & mesh, int numLevels);

	//  Recursive call to subdivide mesh starting at the "node"
	//
	void subdivideTwoEqual(const ofMesh & mesh, TreeNode & node, int numLevels, int currentLevel);

	//  Test for intersection of Tree given a ray and root node.  If a node is hit, return true
	//  and the corresponding node in "nodeRtn"
	//
	bool intersect(const Ray &, const TreeNode & node, TreeNode & nodeRtn);


    
	//  Recursive calls to draw the tree
	//
	void draw(TreeNode & node, int numLevels, int level);
	void draw(int numLevels, int level) {
		draw(root, numLevels, level);
	}

	// Draw just the leaf nodes
	//  
	void drawLeafNodes(TreeNode & node);

	// utility function to draw a box in OF from a "Box" class
	//
	static void drawBox(const Box &box);

	//  Returns a bounding Box for the mesh
	//
	static Box meshBounds(const ofMesh &);

	//  Test which mesh points in "points" are inside "box"; return inside box in "pointsRtn"
	//
	int getMeshPointsInBox(const ofMesh &mesh, const vector<int> & points, Box &box, vector<int> & pointsRtn);

	//   subdvide a Box into separate boxes.  for a binary tree, subdivide into two equal boxes.
	//
    void subDivideBoxInX(const Box &box, vector<Box> & boxList);
	void subDivideBoxInY(const Box &box, vector<Box> & boxList);
    void subDivideBoxInZ(const Box &box, vector<Box> & boxList);
    
    // Subdivide in boxes that are in the median vertex value of an axis
    void subDivideMedianInX(const Box &box, const float xVal, vector<Box> & boxList);
    void subDivideMedianInY(const Box &box, const float yVal, vector<Box> & boxList);
    void subDivideMedianInZ(const Box &box, const float zVal, vector<Box> & boxList);
    void subdivideTwoMedian(const ofMesh & mesh, TreeNode & node, int numLevels, int level);
    

    // Sorts and returns the vertices given a set of indices
    //
    void sortVertices(const int &val, vector<ofVec3f> &set);
    
    //  Tests for intersection between current node's box and a point, proceeds
    //  to check children until leaf node is hit. Return leaf node in "nodeRtn"
    //
    void pointIntersect(const ofVec3f & point, const TreeNode & node, TreeNode & nodeRtn);
    
	//  local data
    //
	ofMesh mesh;
	TreeNode root;
};
