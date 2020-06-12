//  KdTree Template - Simple KdTree class
//
//  SJSU  - CS134 Game Development
//
//  Kevin M. Smith   04/19/20

//  **Important:  Vertices (x, y, z) in the mesh are stored in the Tree node as an integer index.
//  to read the 3D vertex point from the mesh given index i,  use the function ofMesh::getVertex(i);  See
//  KdTree::meshBounds() for an example of usage;
//
//

#include "KdTree.h"
 
//  Comparators for vertices in any axis
//
bool compareInXAxis(const ofVec3f &a, const ofVec3f &b) {return a.x < b.x;}
bool compareInYAxis(const ofVec3f &a, const ofVec3f &b) {return a.y < b.y;}
bool compareInZAxis(const ofVec3f &a, const ofVec3f &b) {return a.z < b.z;}

// draw KdTree (recursively)
//
void KdTree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// draw only leaf Nodes
//
void KdTree::drawLeafNodes(TreeNode & node) {

}


//draw a box from a "Box" class  
//
void KdTree::drawBox(const Box &box) {
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
Box KdTree::meshBounds(const ofMesh & mesh) {
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
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int KdTree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box &box, vector<int> & pointsRtn)
{
    int count = 0;
    for (int i = 0; i < points.size(); i++) {
        glm::vec3 v = mesh.getVertex(points[i]);
        if (box.inside(Vector3(v.x, v.y, v.z))) {
            count++;
            pointsRtn.push_back(points[i]);
        }
    }
    return count;
}



// Subdivide a Box; return children in boxList, splits in X axis
//
void KdTree::subDivideBoxInX(const Box &box, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];
    float xdist = (max.x() - min.x()) / 2;
    
    Box b[2];
    b[0] = Box(min + Vector3(xdist, 0, 0), max);
    b[1] = Box(min, max - Vector3(xdist, 0, 0));
    
    boxList.clear();
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
    
}

//  Subdivide a Box; return children in  boxList, splits in Y axis
//
void KdTree::subDivideBoxInY(const Box &box, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];
    float ydist = (max.y() - min.y()) / 2;
    
    Box b[2];
    b[0] = Box(min + Vector3(0, ydist, 0), max);
    b[1] = Box(min, max - Vector3(0, ydist, 0));
    
    boxList.clear();
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
    
}

// Subdivide a Box; return children in  boxList, splits in Z axis
//
void KdTree::subDivideBoxInZ(const Box &box, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];
    float zdist = (max.z() - min.z()) / 2;
    
    Box b[2];
    b[0] = Box(min + Vector3(0, 0, zdist), max);
    b[1] = Box(min, max - Vector3(0, 0, zdist));
    
    boxList.clear();
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
}

// Subdivide a Box through a median value; return children in  boxList, splits in x axis
//
void KdTree::subDivideMedianInX(const Box &box, const float xVal, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];

    Box b[2];
    b[0] = Box(min, Vector3(xVal, max.y(), max.z()));       // Takes the x value, but keeps everything else the same
    b[1] = Box(Vector3(xVal, min.y(), min.z()), max);
    boxList.clear();
    
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
}

// Subdivide a Box through a median value; return children in  boxList, splits in y axis
//
void KdTree::subDivideMedianInY(const Box &box, const float yVal, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];

    Box b[2];
    b[0] = Box(min, Vector3(max.x(), yVal, max.z()));
    b[1] = Box(Vector3(min.x(), yVal, min.z()), max);
    boxList.clear();
    
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
}

// Subdivide a Box through a median value; return children in  boxList, splits in z axis
//
void KdTree::subDivideMedianInZ(const Box &box, const float zVal, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];

    Box b[2];
    b[0] = Box(min, Vector3(max.x(), max.y(), zVal));
    b[1] = Box(Vector3(min.x(), max.y(), zVal), max);
    boxList.clear();
    
    for (int i = 0; i < 2; i++) {
        boxList.push_back(b[i]);
    }
}

// Creates the KdTree using either equal or median split boxes
//
void KdTree::create(const ofMesh & geo, int numLevels) {
    // Initialize  KdTree Structure
    //
    mesh = geo;
    int level = 0;
    root.box = meshBounds(mesh);
    for (int i = 0; i < mesh.getNumVertices(); i++) {
        root.points.push_back(i);
    }
    level++;
    
    subdivideTwoEqual(mesh, root, numLevels, level);        // Start of recursion
//    subdivideTwoMedian(mesh, root, numLevels, level);
    

}

// Creates two tree nodes from its parent
//
void KdTree::subdivideTwoEqual(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
    // Stops when max level has reached
    if (level >= numLevels) return;
    
    vector<Box> boxList;
    int axisToCut = level % 3;                         // Depending on level, cuts in x, y, or z axis
    
    if (axisToCut == 1)
        subDivideBoxInX(node.box, boxList);            // Subdivides box and returns two half boxes in X axis
    if (axisToCut == 2)
        subDivideBoxInY(node.box, boxList);            // Subdivides box and returns two half boxes in Y axis
    if (axisToCut == 0)
        subDivideBoxInZ(node.box, boxList);            // Subdivides box and returns two half boxes in Z axis
    
    TreeNode c1;                                // Two children
    c1.box = boxList[0];
    
    TreeNode c2;
    c2.box = boxList[1];
    
    vector<int> indicesC1;
    vector<int> indicesC2;
    // Return the points in each box and store in indicesC1/2
    getMeshPointsInBox(mesh, node.points, c1.box, indicesC1);
    getMeshPointsInBox(mesh, node.points, c2.box, indicesC2);
    c1.points = indicesC1;
    c2.points = indicesC2;
    
    if (c1.points.size() != 0) {                       // If there are no points in the box, don't make them children
        node.children.push_back(c1);
    }
    if (c2.points.size() != 0) {
        node.children.push_back(c2);
    }
    
    // Recursive call
    for (int i = 0; i < node.children.size(); i++) {
        subdivideTwoEqual(mesh, node.children[i], numLevels, level + 1);
    }
    
    
}

// Subdivides by calculating median of node and splits in x, y, or z axis
//
void KdTree::subdivideTwoMedian(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
    if (level >= numLevels) return;
    
    vector<Box> boxList;
    int axisToCut = level % 3;                      // 1 = X-Axis, 2 = Y-Axis, 0 = Z-Axis
    int axisToSort;
    if (axisToCut == 1) {axisToSort = 0;}           // Translates to subdivide function
    if (axisToCut == 2) {axisToSort = 1;}
    if (axisToCut == 0) {axisToSort = 2;}

    // Obtains all of the vertices from indices in node
    vector<ofVec3f> sortedList;
    for (int i = 0; i < node.points.size(); i++) {
        sortedList.push_back(mesh.getVertex(node.points[i]));       // From node, push back all vertices in sortedList
    }
    sortVertices(axisToSort, sortedList);                           // Sort vertices
    
    if (axisToCut == 1)
        subDivideMedianInX(node.box, sortedList.at(sortedList.size()/2).x, boxList);        // Subdivides boxes by median
    if (axisToCut == 2)
        subDivideMedianInY(node.box, sortedList.at(sortedList.size()/2).y, boxList);
    if (axisToCut == 0)
        subDivideMedianInZ(node.box, sortedList.at(sortedList.size()/2).z, boxList);
    
    TreeNode c1;                                // Two children
    c1.box = boxList[0];
    
    TreeNode c2;
    c2.box = boxList[1];
    
    vector<int> indicesC1;
    vector<int> indicesC2;
    // Return the points in each box and store in indicesC1/2
    getMeshPointsInBox(mesh, node.points, c1.box, indicesC1);
    getMeshPointsInBox(mesh, node.points, c2.box, indicesC2);
    c1.points = indicesC1;
    c2.points = indicesC2;
    
    if (c1.points.size() != 0) {                       // If there are no points in the box, don't make them children
        node.children.push_back(c1);
    }
    if (c2.points.size() != 0) {
        node.children.push_back(c2);
    }
    
    // Recursive call
    for (int i = 0; i < node.children.size(); i++) {
        subdivideTwoMedian(mesh, node.children[i], numLevels, level + 1);
    }
    
    
}

// Sorts and returns the vertices given a set of indices
//
void KdTree::sortVertices(const int &val, vector<ofVec3f> &set) {
    if (val == 0) {
        std::sort(set.begin(), set.end(), compareInXAxis);
    }
    if (val == 1) {
        std::sort(set.begin(), set.end(), compareInYAxis);
    }
    if (val == 2) {
        std::sort(set.begin(), set.end(), compareInZAxis);
    }
}



// Checks intersection between ray (mouse ray) and node's box, checks its children
// and checks its box for intersection... and so on...
//
bool KdTree::intersect(const Ray &ray, const TreeNode &node, TreeNode & nodeRtn) {
    // MOON IMPLEMENTATION
    if (node.points.size() == 1) {                          // Checks if node contains only one point
        nodeRtn = node;                                     // Doesn't need to be precisely 1 point, can be changed for accomodation
        return true;
    }
    
    // Selecting child node, alternative to checking if node contains only one point
//    if (node.children.size() == 0) {
//        nodeRtn = node;
//        return true;
//    }
    
    // MARS IMPLEMENTATION
//    if (node.points.size() <= 10 && node.points.size() != 0) {                          // Checks if node contains less than ten points
//        nodeRtn = node;
//        return true;
//    }
    
    
    if (node.box.intersect(ray, 0, 1000)) {                 // Checks if box intersects with ray
        if (node.children.size() != 0) {
            for (int i = 0; i < node.children.size(); i++) {
                intersect(ray, node.children[i], nodeRtn);      // Recursive call on its children until number of points inside box is 1
            }
        }
    }
    else return false;
}


void KdTree::pointIntersect(const ofVec3f & point, const TreeNode & node, TreeNode & nodeRtn) {
    bool inside = (point.x >= node.box.parameters[0].x() && point.x <= node.box.parameters[1].x()) &&
                 (point.y >= node.box.parameters[0].y() && point.y <= node.box.parameters[1].y()) &&
                (point.z >= node.box.parameters[0].z() && point.z <= node.box.parameters[1].z());
    
    if (node.children.size() == 0 && inside) {
        nodeRtn = node;
        return;
    }
    
    if (inside) {
        for (int i = 0; i < node.children.size(); i++) {
            pointIntersect(point, node.children[i], nodeRtn);
        }
    }
}


