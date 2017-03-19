/*  =================== File Information =================
        File Name: main.cpp
        Description:
        Author: Michael Shah, mostly

        Purpose: Driver for 3D program to load .ply models 
        ===================================================== */

#include <string.h>
#include <iostream>
#include <fstream>
#include <GL/glui.h>
#include <math.h>
#include "ply.h"
#include "Algebra.h"
#define SPHERE 1
#define CUBE 0
/** These are the live variables passed into GLUI ***/
int main_window;
int  wireframe = 0;
int  silhouette = 0;
int  filled = 1;
int  rotY = 0;
int drop = true;
float heightY = 3;
float radius = 0.1;
int  scale = 40;
int objType = 0;
float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float mouseX;
float mouseY;
/* This is a textbox that we can edit, we
        use it to 
*/
GLUI_EditText* filenameTextField = NULL;
string filenamePath = "cow.ply";

/****************************************/
/*         PLY Object                   */
/****************************************/
ply* myPLY = new ply (filenamePath);
Point spherePos;
Vector sphereTrajectory;


Point cubePos;
Vector cubeTrajectory;
/***************************************** myGlutIdle() ***********/
void callback_obj(int obj) {
    cerr << objType << endl;
}
void myMouse(int button, int state, int x, int y) {
    float width = glutGet(GLUT_WINDOW_WIDTH);
    float height = glutGet(GLUT_WINDOW_HEIGHT);
    mouseX = ((x/width) - 0.5) * 3.0;
    mouseY = -((1 - (y/height)) - 0.5) * 3.0;
    Vector back(0, 0, 1);
    //myPLY->deformModel(mouseX, mouseY, rot_mat(up, DEG_TO_RAD(rotY)))a
    if (objType == SPHERE) {
        if (!drop) {
            Matrix transform = rotY_mat(DEG_TO_RAD(-rotY));
            sphereTrajectory = Vector(0, 0, 0);
            sphereTrajectory[2] = -0.05;
            sphereTrajectory = transform * sphereTrajectory;

            spherePos = Point(0, 0, 0);
            spherePos[2] = 1.7;
            spherePos[1] = heightY;
            spherePos = transform * spherePos;
        } else {
            sphereTrajectory = Vector(0, -0.005, 0);

            spherePos = Point(mouseX, heightY, mouseY);
        }
    }
    if (objType == CUBE) {
        if (!drop) {
            Matrix transform = rotY_mat(DEG_TO_RAD(-rotY));
            cubeTrajectory = Vector(0, 0, 0);
            cubeTrajectory[2] = -0.05;
            cubeTrajectory = transform * cubeTrajectory;

            cubePos = Point(0, 0, 0);
            cubePos[2] = 1.7;
            cubePos[1] = heightY;
            cubePos = transform * spherePos;
        } else {
            cubeTrajectory = Vector(0, -0.005, 0);
            cubePos = Point(mouseX, heightY, mouseY);
        }
    }
}

void myGlutIdle(void)
{
        /* According to the GLUT specification, the current window is
        undefined during an idle callback.  So we need to explicitly change
        it if necessary */
        if (glutGetWindow() != main_window){
                glutSetWindow(main_window);
        }
        glutPostRedisplay();
}


/**************************************** myGlutReshape() *************/

/*
        We configure our window so that it correctly displays our objects
        in the correct perspective.
*/
void myGlutReshape(int x, int y)
{
        float xy_aspect;
        xy_aspect = (float)x / (float)y;

        glViewport(0, 0, x, y);
        // Determine if we are modifying the camera(GL_PROJECITON) matrix(which is our viewing volume)
        // Otherwise we could modify the object transormations in our world with GL_MODELVIEW
        glMatrixMode(GL_PROJECTION);
        // Reset the Projection matrix to an identity matrix
        glLoadIdentity();
        // The frustrum defines the perspective matrix and produces a perspective projection.
        // It works by multiplying the current matrix(in this case, our matrix is the GL_PROJECTION)
        // and multiplies it.
        //glFrustum(-xy_aspect*.08, xy_aspect*.08, -.08, .08, .1, 15.0);
        //glFrustum(-xy_aspect*.125, xy_aspect*.125, -.125, .125, .1, 15.0);
        glOrtho(-xy_aspect*0.6, xy_aspect*0.6, -0.6, 0.6, .01, 15.0);
        // Since we are in projection mode, here we are setting the camera to the origin (0,0,0)
        glTranslatef(0, 0, -0.5);
        // Call our display function.
        glutPostRedisplay();
}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay(void)
{
        // Clear the buffer of colors in each bit plane.
        // bit plane - A set of bits that are on or off (Think of a black and white image)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Set the mode so we are modifying our objects.
        glMatrixMode(GL_MODELVIEW);
        // Load the identify matrix which gives us a base for our object transformations
        // (i.e. this is the default state)
        glLoadIdentity();
        //allow for user controlled rotation and scaling
        glMultMatrixf(view_rotate);
        glScalef(scale / 100.0, scale / 100.0, scale / 100.0);
        glRotatef(rotY, 0.0, 1.0, 0.0);
        if (objType == SPHERE) {
            if (sphereTrajectory.length() > 0) {
                glPushMatrix();
                glTranslated(spherePos[0], spherePos[1], spherePos[2]);
                glColor3f(1, 1, 0);
                glutWireSphere(radius, 5, 5);
                glPopMatrix(); 
            }
        }
        if (objType == CUBE) {
            if (cubeTrajectory.length() > 0) {
                glPushMatrix();
                glTranslated(cubePos[0], cubePos[1], cubePos[2]);
                glColor3f(1, 0, 1);
                glutWireCube(radius);
                glPopMatrix(); 
            }

        }
        glPushMatrix();

        float rotRad = PI * (rotY / 180.0);
        myPLY->lookX = sinf(-rotRad);
        myPLY->lookZ = cosf(-rotRad);

        //draw the axes
        glLineWidth(1);
        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(0, 0, 0); glVertex3f(1.0, 0, 0);
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(0, 0, 0); glVertex3f(0.0, 1.0, 0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.0);
        glEnd();

        if (objType == CUBE && cubeTrajectory.length() > 0) {
            cubePos = cubePos + cubeTrajectory;
            Vector r(radius / 2, radius / 2, radius / 2); 
            if (myPLY->deformModel(cubePos - r, cubePos + r, cubeTrajectory / 100)) {
                cubeTrajectory = cubeTrajectory * 0.1;
            } else {

                if (drop) {
                    cubeTrajectory = cubeTrajectory + Vector(0, -0.001, 0);
                }
            }
        }

        if (objType == SPHERE && sphereTrajectory.length() > 0) {
            spherePos = spherePos + sphereTrajectory;
            if (myPLY->deformModel(spherePos, radius, sphereTrajectory / 100)) {
                sphereTrajectory = sphereTrajectory * 0.01;
                if (sphereTrajectory.length() < 0.0001) {
                    sphereTrajectory = Vector();
                } 
            } else {
                if (drop) {
                    sphereTrajectory = sphereTrajectory + Vector(0, -0.001, 0);
                }
            }

        }
        //myPLY->adjustModel(wireframe);

        if (filled) {
            glEnable(GL_LIGHTING);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glColor3f(0.6, 0.6, 0.6);
            glPolygonMode(GL_FRONT, GL_FILL);
            myPLY->render();
        }

        if (wireframe) {
            glDisable(GL_LIGHTING);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glColor3f(1.0, 1.0, 0.0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            myPLY->render();
        }

        if(silhouette){
            glColor3f(1.0, 1.0, 1.0);
            glLineWidth(2);
            myPLY->renderSilhouette();
        }
        glPopMatrix();
        glutSwapBuffers();
}

/*  ==========================================
    Clean up all dynamically allocated memory
    ========================================== */
void onExit()
{
    delete myPLY;
}

/*   ==========================================
     Callback function
     A callback function is a function that is triggered
     by some operating system event (like clicking a button)
     and then running this function when an action has occurred.
     ========================================== */
void callback_load(int id) {

    if (filenameTextField == NULL) {
        return;
    }
    // 
    cout << "Loading new ply file from: " << filenameTextField->get_text() << endl;
    // Reload our model
    myPLY->reload(filenameTextField->get_text());
    // Print out the attributes
    myPLY->printAttributes();
}


/**************************************** main() ********************/

int main(int argc, char* argv[])
{

    atexit(onExit);

    /****************************************/
    /*   Initialize GLUT and create window  */
    /****************************************/

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(500, 500);

    main_window = glutCreateWindow("COMP 175 In Class Lab 7");
    glutDisplayFunc(myGlutDisplay);
    glutReshapeFunc(myGlutReshape);

    /****************************************/
    /*       Set up OpenGL lighting         */
    /****************************************/

    // Essentially set the background color of the 3D scene.
    //glClearColor(.9f, .9f, .9f, 1.0f);
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glShadeModel(GL_FLAT);

    GLfloat light_pos0[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 0.0f };
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);

    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    /****************************************/
    /*          Enable z-buferring          */
    /****************************************/

    glEnable(GL_DEPTH_TEST);
    glPolygonOffset(1, 1);

    /****************************************/
    /*         Here's the GLUI code         */
    /****************************************/

    GLUI *glui = GLUI_Master.create_glui("GLUI");

    GLUI_Panel *render_panel = glui->add_panel("Render");
    glutMouseFunc(myMouse);
    new GLUI_Checkbox(render_panel, "Wireframe", &wireframe);
    new GLUI_Checkbox(render_panel, "Filled", &filled);
    new GLUI_Checkbox(render_panel, "Silhouette", &silhouette);
    new GLUI_Checkbox(render_panel, "Drop", &drop);

    GLUI_Panel *camera_panel = glui->add_panel("Camera");
    GLUI_Rotation *view_rot = new GLUI_Rotation(camera_panel, "Objects", view_rotate );
    (new GLUI_Spinner(camera_panel, "Rotate Y:", &rotY))->set_int_limits(0, 359);
    (new GLUI_Spinner(camera_panel, "Scale:", &scale))
        ->set_int_limits(1, 1000);


    GLUI_Panel *obj_panel = glui->add_panel("Projectile Type");
    new GLUI_Spinner(obj_panel, "Height Y:", &heightY);
    new GLUI_Spinner(obj_panel, "Radius", &radius);
    GLUI_RadioGroup *group1 =
        glui->add_radiogroup_to_panel(obj_panel, (int*)(&objType), 3, callback_obj);
    glui->add_radiobutton_to_group(group1, "Cube");
    glui->add_radiobutton_to_group(group1, "Sphere");
    filenameTextField = new GLUI_EditText( glui, "Filename:", filenamePath);
    filenameTextField->set_w(300);
    glui->add_button("Load PLY", 0, callback_load);


    glui->add_column(true);
    glui->add_button("Quit", 0, (GLUI_Update_CB)exit);

    glui->set_main_gfx_window(main_window);
    /* We register the idle callback with GLUI, *not* with GLUT */
    GLUI_Master.set_glutIdleFunc(myGlutIdle);



    glutMainLoop();

    return EXIT_SUCCESS;
}



