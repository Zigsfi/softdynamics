#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Algebra.h"

/*  ============== Vertex ==============
	Purpose: Stores properties of each vertex
	Use: Used in face structure
	==================================== */  

class vertex{
public:
	float x,y,z;		// position in 3D space
    float centerLen;
    Vector velocity;
	float confidence;
	float intensity;
	float r,g,b;		// Color values
};
class face{
public:
	int vertexCount;  // assuming 3 for this
	int* vertexList;  // assuming this has size 3
  
        //normal vector
        float normX, normY, normZ;
        //dot of normal and look
        float dotProd;

        // Default constructor
	face(){
		vertexCount = 0;
	}

};
class edge{
public:
        int vertices[2];
        int faces[2];
        float len;

        //default constructor
        edge(){
            //these are -1 because 0 would be a meaningful value
			vertices[0] = -1;
			vertices[1] = -1;
            faces[0] = -1;
            faces[1] = -1;
        }
};

struct VertexNode {

    VertexNode() {
        nextNodes.reserve(10);
    };

    int vertex;
    std::vector<VertexNode *> nextNodes;
    bool marked;
    void addNext(VertexNode *v) {
        nextNodes.push_back(v);
    };
};

class VertexGraph{
private:
    VertexNode *nodes;
    int nodeCount;
    vertex *vertexList;
public:
    void construct(vertex *vertexList, edge *edgeList, int vertexCount, int edgeCount) {
        nodes = new VertexNode[vertexCount];
        nodeCount = vertexCount;
        this->vertexList = vertexList;
        for (int i = 0; i < vertexCount; i++) {
            nodes[i].vertex = i;
            nodes[i].marked = false;
        }
        std::cerr << "Start constructing " << edgeCount << std::endl;

        for (int i = 0; i < edgeCount; i++) {
            int v1 = edgeList[i].vertices[0];
            int v2 = edgeList[i].vertices[1];

            nodes[v1].addNext(&nodes[v2]);
            nodes[v2].addNext(&nodes[v1]);

        }
        std::cerr << "Done constructing" << std::endl;

    };
    void deform(int source, Vector force, int depth) {
        source = source % nodeCount;
        if (depth > 0 || !nodes[source].marked) {
            nodes[source].marked = true;
            vertexList[source].x += force[0];
            vertexList[source].y += force[1];
            vertexList[source].z += force[2];
            for (auto &node : nodes[source].nextNodes) {
                deform(node->vertex, force / 2, depth-1);
            }
        }
    };

    int pickVert(float x, float y) {
        float minDist = 1;
        int index = -1;
        for (int i = 0; i < nodeCount; i++) {
            Point p(x, y, 0);
            Point vp(vertexList[i].x, vertexList[i].y, vertexList[i].z);
            Vector v = vp - p;
            if (v.length() < minDist) {
                minDist = v.length();
                index = i;
            }
            nodes[i].marked = false;
        }
        return index;
    };
};

/*  ============== Face ==============
Purpose: Store list of vertices that make up a polygon.
In modern versions of OpenGL this value will always be 3(a triangle)
Use: Used in Shape data structure.
==================================== */  

/* Edge: Connects two vertices, and two faces. 
*/

#endif
