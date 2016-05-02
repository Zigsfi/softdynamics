/*  =================== File Information =================
  File Name: ply.cpp
  Description: Loads a .ply file and renders it on screen.  
        New to this version: also renders the silhouette!
  Author: Paul Nixon
  ===================================================== */
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <GL/glui.h>
#include "ply.h"
#include "geometry.h"
#include <math.h>
#include "Algebra.h"

#define KS 0
#define KV 0.1
#define G 1
#define M 1
#define DT .01

using namespace std;

/*  ===============================================
      Desc: Default constructor for a ply object
      Precondition: _filePath is set to a valid filesystem location
            which contains a valid .ply file (triangles only)
      Postcondition: vertexList, faceList are filled in
    =============================================== */ 
ply::ply(string _filePath){
        filePath = _filePath;
        vertexList = NULL;
        faceList = NULL;
        edgeList = NULL;
        properties = 0; 
		vertexCount = 0;
		faceCount = 0;
		edgeCount = 0;
        // Call helper function to load geometry
        loadGeometry();
}

/*  ===============================================
      Desc: Destructor for a ply object
      Precondition: Memory has been already allocated
      =============================================== */ 
ply::~ply(){
  deconstruct();
}

inline float sqt(float n) {
    return sqrt(n < 0 ? 0 : n);
}

void ply::deconstruct(){
  int i;
  // Delete the allocated arrays
  delete[] vertexList;

  for (i = 0; i < faceCount; i++) {
          delete [] faceList[i].vertexList;
  }

  delete[] faceList;
  delete[] edgeList;
  
  // Set pointers to NULL
  vertexList = NULL;
  faceList = NULL;
  edgeList = NULL;
}

/*  ===============================================
      Desc: reloads the geometry for a 3D object
            (or loads a different file)
    =============================================== */ 
void ply::reload(string _filePath){
  
  filePath = _filePath;
  deconstruct();
  // Call our function again to load new vertex and face information.
  loadGeometry();
}
/*  ===============================================
      Desc: Loads the data structures (look at geometry.h and ply.h)
      Precondition: filePath is something valid, arrays are NULL
      Postcondition: data structures are filled 
          (including edgeList, this calls scaleAndCenter and findEdges)
      =============================================== */ 
void ply::loadGeometry(){

  /* You will implement this section of code
        1. Parse the header
        2.) Update any private or helper variables in the ply.h private section
        3.) allocate memory for the vertexList 
            3a.) Populate vertices
        4.) allocate memory for the faceList
            4a.) Populate faceList
  */


    ifstream myfile (filePath.c_str()); // load the file
    if ( myfile.is_open()) { // if the file is accessable
        properties = -2; // set the properties because there are extras labeled
        
        string line;
        char * token_pointer; 
        char * lineCopy = new char[80]; 
        int count;
        bool reading_header = true;
        // loop for reading the header 
        while (reading_header && getline ( myfile, line)) {

            // get the first token in the line, this will determine which
            // action to take. 
            strcpy(lineCopy, line.c_str());
            token_pointer = strtok(lineCopy, " ");
            // case when the element label is spotted:
            if (strcmp(token_pointer, "element") == 0){
                token_pointer = strtok(NULL, " ");

                // When the vertex token is spotted read in the next token
                // and use it to set the vertexCount and initialize vertexList
                if (strcmp(token_pointer, "vertex") == 0){
                    token_pointer = strtok(NULL, " ");
                    vertexCount = atoi(token_pointer);
                    vertexList = new vertex[vertexCount];
                }

                // When the face label is spotted read in the next token and 
                // use it to set the faceCount and initialize faceList.
                if (strcmp(token_pointer, "face") == 0){
                    token_pointer = strtok(NULL, " ");
                    faceCount = atoi(token_pointer);
                    faceList = new face[faceCount];
                }
            }
            // if property label increment the number of properties.
            if (strcmp(token_pointer, "property") == 0) {properties++;} 
            // if end_header break the header loop and move to reading vertices.
            if (strcmp(token_pointer, "end_header") == 0) {reading_header = false; }
        }
        
        // Read in exactly vertexCount number of lines after reading the header
        // and set the appropriate vertex in the vertexList.
        for (int i = 0; i < vertexCount; i++){

            getline ( myfile, line); 
            strcpy(lineCopy, line.c_str());
            
            // depending on how many properties there are set that number of 
            // elements (x, y, z, confidence, intensity, r, g, b) (max 7) with
            // the input given
            if ( properties >= 0){
                vertexList[i].x = atof(strtok(lineCopy, " "));
            }
            if ( properties >= 1){
                vertexList[i].y = atof(strtok(NULL, " "));
            }
            if (properties >= 2){
                vertexList[i].z = atof(strtok(NULL, " "));
            }
            if (properties >= 3){
                vertexList[i].confidence = atof(strtok(NULL, " "));
            }
            if (properties >= 4){
                vertexList[i].intensity = atof(strtok(NULL, " "));
            }
            if (properties >= 5){
                vertexList[i].r = atof(strtok(NULL, " "));
            }
            if (properties >= 6) {
                vertexList[i].g = atof(strtok(NULL, " "));
            }
            if (properties >= 7) {
                vertexList[i].b = atof(strtok(NULL, " "));
            }
        } 
        
        // Read in the faces (exactly faceCount number of lines) and set the 
        // appropriate face in the faceList
        for (int i = 0; i < faceCount; i++){

            getline ( myfile, line);

            strcpy(lineCopy, line.c_str());
            count = atoi(strtok(lineCopy, " "));
            faceList[i].vertexCount = count; // number of vertices stored 
            faceList[i].vertexList = new int[count]; // initialize the vertices
            
            // set the vertices from the input, reading only the number of 
            // vertices that are specified
            for (int j = 0; j < count; j++){
                faceList[i].vertexList[j] = atoi(strtok(NULL, " "));
            }
        }
        delete(lineCopy);
    }
    // if the path is invalid, report then exit.
    else {
		cout << "cannot open file " << filePath.c_str() << "\n";
        exit(1);
    }
    myfile.close();

    forceList   = new Vector[vertexCount];
    centerForce = Vector();
    scaleAndCenter();
    findEdges();
};

/*  ===============================================
Desc: Moves all the geometry so that the object is centered at 0, 0, 0 and scaled to be between 0.5 and -0.5
Precondition: after all the vetices and faces have been loaded in
Postcondition: points have reasonable values
=============================================== */
void ply::scaleAndCenter() {
    float avrg_x = 0.0;
    float avrg_y = 0.0;
    float avrg_z = 0.0;
    float max = 0.0;
    int i; 

    //loop through each vertex in the given image
    for (i = 0; i < vertexCount; i++){
        
        // obtain the total for each property of the vertex
        avrg_x += vertexList[i].x;
        avrg_y += vertexList[i].y;
        avrg_z += vertexList[i].z;

        // obtain the max dimension to find the furthest point from 0,0
        if (max < (vertexList[i].x)) max = (vertexList[i].x);
        if (max < (vertexList[i].y)) max = (vertexList[i].y);
        if (max < (vertexList[i].z)) max = (vertexList[i].z);
    }
    // compute the average for each property
    avrg_x = avrg_x / vertexCount;
    avrg_y = avrg_y / vertexCount;
    avrg_z = avrg_z / vertexCount;
 
    // *******multiply the max by 2.5 to shrink the image to fit it into the 
    // given window dimensions. *******

    // center and scale each vertex 
    for (i = 0; i < vertexCount; i++){
        vertexList[i].x = (vertexList[i].x - avrg_x) / max;
        vertexList[i].y = (vertexList[i].y - avrg_y) / max;
        vertexList[i].z = (vertexList[i].z - avrg_z) / max;
    }

    center   = vertex();
    center.x = 0;
    center.y = 0;
    center.z = 0;
    center.velocity = Vector();
}

/*  ===============================================
      Desc: Draws a filled 3D object
      Precondition: arrays are EITHER valid data OR NULL
      Postcondition: no changes to data
      Error Condition: If we haven't allocated memory for our
      faceList or vertexList then do not attempt to render.
    =============================================== */  
void ply::render(){
    if(vertexList==NULL || faceList==NULL){
                return;
    }

    glPushMatrix();
    glTranslatef(getXPosition(),getYPosition(),getZPosition());
    glScalef(getXScale(),getYScale(),getZScale());
    // For each of our faces
    glBegin(GL_TRIANGLES);
          for(int i = 0; i < faceCount; i++) {
            // All of our faces are actually triangles for PLY files
                        // Get the vertex list from the face list
                        int index0 = faceList[i].vertexList[0];
                        int index1 = faceList[i].vertexList[1];
                        int index2 = faceList[i].vertexList[2];

                        setNormal(i, vertexList[index0].x, vertexList[index0].y, vertexList[index0].z,
                                          vertexList[index1].x, vertexList[index1].y, vertexList[index1].z,
                                          vertexList[index2].x, vertexList[index2].y, vertexList[index2].z);

            for(int j = 0; j < faceList[i].vertexCount; j++){
                                // Get each vertices x,y,z and draw them
                int index = faceList[i].vertexList[j];
                glVertex3f(vertexList[index].x,vertexList[index].y,vertexList[index].z);
            }
        }
        glEnd();        
        glPopMatrix();
}

float ply::findLen (int i1, int i2) {
    vertex v1 = vertexList[i1];
    vertex v2 = vertexList[i2];
    float xd = v2.x - v1.x;
    float yd = v2.y - v1.y;
    float zd = v2.z - v1.z;

    float l = sqt(xd*xd + yd*yd + zd*zd);

    return l;
}

float ply::findCenterLen (int index) {
    vertex v = vertexList[index];

    float xd = center.x - v.x;
    float yd = center.y - v.y;
    float zd = center.z - v.z;
    
    float l  = sqt(xd*xd + yd*yd + zd*zd);
 
    return l;
}

Point ply::asPoint(int index) {
    vertex v = vertexList[index];
    return Point(v.x, v.y, v.z);
}

Vector ply::computeEdgeContribution(edge e) {
    int v1 = e.vertices[0];
    int v2 = e.vertices[1];

    Vector d = asPoint(v2) - asPoint(v1);

    float x  = findLen(v1, v2) - e.len; 

    Vector fVec = d * x;

    return fVec * KS;
}


Vector ply::computeVolumeContribution(int index) {
    vertex v = vertexList[index];
    Vector d = Point(center.x, center.y, center.z) - asPoint(index);

    float x = findCenterLen(index) - v.centerLen;

    Vector fVec = d * x;
    return fVec * KV;
}

void ply::adjustModel(bool w) {
    // For every edge, compute the force contributed by
    // the stretched or compressed edge

    for (int i = 0; i < edgeCount; i++) {
        float ft = 0;
        if (i < edgeCount / 2 && w) ft = 1;
        Vector fv = Vector(0, ft, 0);

        edge e = edgeList[i];
        int v1 = e.vertices[0];
        int v2 = e.vertices[1];

        float be = -sqrt(4 * M * KS);
        float bv = -sqrt(4 * M * KV);

        if (isnan(be)) be = 0;
        if (isnan(bv)) bv = 0;
        
        Vector fVec  = computeEdgeContribution(e);
        Vector fNorm = fVec;

        fNorm.normalize();

        //if (i == 0) ; //.print();
        if (!isnan(fVec.length())) 
            cout << fVec.length() << endl;

        Vector v1SDamping = (be * (dot(vertexList[v1].velocity, fNorm) * fNorm));
        Vector v1VDamping = (bv * (dot(vertexList[v1].velocity, fNorm) * fNorm));
        Vector v2SDamping = (be * (dot(vertexList[v2].velocity, fNorm) * fNorm));
        Vector v2VDamping = (bv * (dot(vertexList[v2].velocity, fNorm) * fNorm));
       /* 
        if (v1SDamping != Vector() || v1VDamping != Vector()){ 
            cout << "v1S: ";
            v1SDamping.print();
            fVec.print();
            cout << " v1V: " ;
            v1VDamping.print(); 
            cout << endl;
        }*/
        
        Vector v1Vec = computeVolumeContribution(v1); 
        Vector v2Vec = computeVolumeContribution(v2); 

        Vector floorForce = Vector(0, 0, 0);
        //collide with floor
        if (vertexList[v1].y < -1) floorForce = Vector(0, -G, 0);

        forceList[v1] = forceList[v1] + (fVec  + v1SDamping) + (v1Vec + v1VDamping) + floorForce + fv;
        forceList[v2] = forceList[v2] + (-fVec + v2SDamping) + (v2Vec + v2VDamping) + floorForce + fv;
        
        centerForce   = centerForce + (-v1Vec - (bv * (dot(center.velocity, fNorm) * fNorm))); 
        centerForce   = centerForce + (-v2Vec - (bv * (dot(center.velocity, fNorm) * fNorm)));
    }
    // Apply forces to vertices
    for (int i = 0; i < vertexCount; i++) {
        vertex v    = vertexList[i];
        Vector g    = Vector (0, G, 0);
        Vector fVec = (forceList[i] + g);

    //    if (i == 0) asPoint(i).print();

        Vector a    = fVec / M;
        Vector vi   = v.velocity;
        Vector vf   = vi + a * DT;
        Vector d    = vi * DT + .5 * DT * DT * a;
      
        v.x += d[0];
        v.y += d[1];
        v.z += d[2];

        v.velocity = vf;
        vertexList[i] = v;

        forceList[i] = Vector();
    }
    // Apply center forces
    Vector fVec = centerForce + Vector(0, G, 0);
    Vector a    = fVec / (M * vertexCount);
    Vector vi   = center.velocity;
    Vector vf   = vi + a * DT;
    Vector d    = vi * DT + .5 * DT * DT * a;
    
    //std::cout << center.x << " ," << center.y << " ," << center.z << std::endl;
    center.x   += d[0];
    center.y   += d[1];
    center.z   += d[2];

    center.velocity = vf;
    centerForce = Vector();
}

//loads data structures so edges are known
void ply::findEdges(){
    //edges, if you want to use this data structure
    //TODO add all the edges to the edgeList and make sure they have both faces
    edgeList = new edge[faceCount * 9];

    edgeCount = 0;
    for (int i = 0; i < faceCount; i++) {
        for (int j = i; j < faceCount; j++) {
            // find a shared vertex between the faces
            int sharedVert1 = -1;
            int sharedVert2 = -1;
            for (int fi = 0; fi < 3; fi++) {
                for (int fj = 0; fj < 3; fj++) {
                    if (faceList[i].vertexList[fi] == faceList[j].vertexList[fj]) {
                        if (sharedVert1 == -1){
                            sharedVert1 = faceList[i].vertexList[fi];
                        } else {
                            sharedVert2 = faceList[i].vertexList[fi];
                            fi = 4;
                            fj = 4;
                        }
                    }
                }
            }

            if (sharedVert1 != -1 && sharedVert2 != -1) { // 2 shared vertices
                edgeList[edgeCount].vertices[0] = sharedVert1;
                edgeList[edgeCount].vertices[1] = sharedVert2;
                edgeList[edgeCount].faces[0] = i;
                edgeList[edgeCount].faces[1] = j;
                edgeList[edgeCount].len = findLen(sharedVert1, sharedVert2);
                edgeCount++;

                vertexList[sharedVert1].centerLen = findCenterLen(sharedVert1);
                vertexList[sharedVert2].centerLen = findCenterLen(sharedVert2);
                vertexList[sharedVert1].velocity  = Vector();
                vertexList[sharedVert2].velocity  = Vector();
            }
        }
    }
} 

/* Desc: Renders the silhouette
 * Precondition: Edges are known
 */
void ply::renderSilhouette(){
    glPushMatrix();
    glBegin(GL_LINES);

    //TODO Iterate through the edgeList, and if you want to draw an edge,
    //call glVertex3f once for each vertex in that edge.  
    //
    for (int i = 0; i < edgeCount; i++) {
        face face1 = faceList[edgeList[i].faces[0]];
        face face2 = faceList[edgeList[i].faces[1]];
        if (dot(Vector(face1.normX, face1.normY, face1.normZ), Vector(lookX, 0, lookZ)) * 
                dot(Vector(face2.normX, face2.normY, face2.normZ), Vector(lookX, 0, lookZ))
                < 0) {
            vertex vertex1 = vertexList[edgeList[i].vertices[0]];
            vertex vertex2 = vertexList[edgeList[i].vertices[1]];
            glVertex3f(vertex1.x, vertex1.y, vertex1.z);
            glVertex3f(vertex2.x, vertex2.y, vertex2.z);
        }
    }
    glEnd();
    glPopMatrix();
} 

/*  ===============================================
Desc: Prints some statistics about the file you have read in
This is useful for debugging information to see if we parse our file correctly.
=============================================== */ 
void ply::printAttributes(){
    cout << "==== ply Mesh Attributes=====" << endl;
    cout << "vertex count:" << vertexCount << endl;
    cout << "face count:" << faceCount << endl;       
    cout << "properties:" << properties << endl;
}

/*  ===============================================
Desc: Iterate through our array and print out each vertex.
=============================================== */ 
void ply::printVertexList(){
    if(vertexList==NULL){
        return;
    }else{
        for(int i = 0; i < vertexCount; i++){
            cout << vertexList[i].x << "," << vertexList[i].y << "," << vertexList[i].z << endl;
        }
    }
}

/*  ===============================================
Desc: Iterate through our array and print out each face.
=============================================== */ 
void ply::printFaceList(){
    if(faceList==NULL){
        return;
    }else{
        // For each of our faces
        for(int i = 0; i < faceCount; i++){
            // Get the vertices that make up each face from the face list
            for(int j = 0; j < faceList[i].vertexCount; j++){
                // Print out the vertex
                int index = faceList[i].vertexList[j];
                cout << vertexList[index].x << "," << vertexList[index].y << "," << vertexList[index].z << endl;
            }
        }
    }
}

//makes a GL Call to set the normal but also stores it 
//so it can be accessible for silhouette finding
void ply::setNormal(int facenum, float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3) {

    float v1x, v1y, v1z;
    float v2x, v2y, v2z;
    float cx, cy, cz;

    //find vector between x2 and x1
    v1x = x1 - x2;
    v1y = y1 - y2;
    v1z = z1 - z2;

    //find vector between x3 and x2
    v2x = x2 - x3;
    v2y = y2 - y3;
    v2z = z2 - z3;

    //cross product v1xv2

    cx = v1y * v2z - v1z * v2y;
    cy = v1z * v2x - v1x * v2z;
    cz = v1x * v2y - v1y * v2x;

    //normalize

    float length = sqt(cx * cx + cy * cy + cz * cz);

    cx = cx / length;
    cy = cy / length;
    cz = cz / length;       

    faceList[facenum].normX = cx;
    faceList[facenum].normY = cy;
    faceList[facenum].normZ = cz;

    glNormal3f(cx, cy, cz);
}
