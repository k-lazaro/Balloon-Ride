//  Kevin M. Smith - Basic Octree Class - CS134/235 4/18/18
//


#include "Octree.h"
 

// draw Octree (recursively)
//
void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// draw only leaf Nodes
//
void Octree::drawLeafNodes(TreeNode & node) {

}


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
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
Box Octree::meshBounds(const ofMesh & mesh) {
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
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
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



//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure
	//
    mesh = geo;
    int level = 0;
    root.box = meshBounds(mesh);
    for (int i = 0; i < mesh.getNumVertices(); i++) {
        root.points.push_back(i);
    }
    level++;
    subdivide(mesh, root, numLevels, level);        // Start of recursion

}

void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
    // Stops when max level has reached
    if (level >= numLevels) return;
    
    vector<Box> boxList;
    subDivideBox8(node.box, boxList);       // Subdivide box into eight and store in boxList
    
    // Eight children
    vector<TreeNode> children;
    for (int i = 0; i < 8; i++) {
        TreeNode child;
        child.box = boxList[i];
        children.push_back(child);
    }
    // Eight vectors of indices
    vector<vector<int>> indices;
    for (int j = 0; j < 8; j++) {
        vector<int> i;
        indices.push_back(i);
    }
    
    // Return the points in each box and store in indices
    for (int k = 0; k < 8; k++) {
        getMeshPointsInBox(mesh, node.points, children[k].box, indices[k]);
    }
    
    for (int m = 0; m < 8; m++) {
        children[m].points = indices[m];
    }
    
    for (int n = 0; n < 8; n++) {
        if (children[n].points.size() != 0) {
            node.children.push_back(children[n]);
        }
    }
    
    // Recursive call
    for (int i = 0; i < node.children.size(); i++) {
        subdivide(mesh, node.children[i], numLevels, level + 1);
    }
}

bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
    if (node.points.size() == 1) {                          // Checks if node contains only one point
        nodeRtn = node;                                     // Doesn't need to be precisely 1 point, can be changed for accomodation
        return true;
    }
    
    if (node.box.intersect(ray, 0, 1000)) {                 // Checks if box intersects with ray
        if (node.children.size() != 0) {
            for (int i = 0; i < node.children.size(); i++) {
                intersect(ray, node.children[i], nodeRtn);      // Recursive call on its children until number of points inside box is 1
            }
        }
    }
    else return false;
}

void Octree::pointIntersect(const ofVec3f & point, const TreeNode & node, TreeNode & nodeRtn) {

    // Checks if point is inside node's box
    bool inside = (point.x >= node.box.parameters[0].x() && point.x <= node.box.parameters[1].x()) &&
                 (point.y >= node.box.parameters[0].y() && point.y <= node.box.parameters[1].y()) &&
                (point.z >= node.box.parameters[0].z() && point.z <= node.box.parameters[1].z());
    
    // Point has to be inside and leaf node
    if (node.children.size() == 0 && inside) {
        nodeRtn = node;
        return;
    }
    // if inside, recursive call
    if (inside) {
        for (int i = 0; i < node.children.size(); i++) {
            pointIntersect(point, node.children[i], nodeRtn);
        }
    }
}


