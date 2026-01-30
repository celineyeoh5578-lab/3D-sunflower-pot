#include <math.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

static float lightAngle = 0.0;
GLfloat angle1 = -150 ,angle2 = 30;
bool open=false;
int lightMoving=0;

GLuint ledArmTextures[3], floorTextureID;
int currentTextureIndex = 0;

// lid
float lidSpeed=0.1f,lidPosition=0;

// Stopper
float stopperPostion=0;

// rotate
float baseRotateAngle=0,rotateAngle=0,rotateSpeed=0;

// camera
float cameraZ=38.0f,cameraY=5.0f,cameraX=0.0f;
float cameraAngle=0.0f;

// petals curve
const int n=3;
int numdrawsegs=50;

enum { X, Y, Z, W };
enum { A, B, C, D };

// font
void *header = GLUT_BITMAP_HELVETICA_18;
void *content = GLUT_BITMAP_HELVETICA_12;
void *content2 = GLUT_BITMAP_HELVETICA_10;

GLfloat controlpts[n][3] = {
    {-0.6, 0.0, 0.0},
    {-0.2, 0.3, 0.0},
    {0.2, 0.0, 0.0},
};

void getBezierPoint(float t, GLfloat* result) {
    float u = 1 - t;
    float b0 = u * u;
    float b1 = 2 * u * t;
    float b2 = t * t;

    for (int i = 0; i < 3; i++) {
        result[i] = b0 * controlpts[0][i] +
                    b1 * controlpts[1][i] +
                    b2 * controlpts[2][i];
    }
}

bool loadTexture(const char* filename, GLuint &textureID) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cout << "Failed to load texture: " << filename << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return true;
}

void drawText(float x, float y, void *font, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

void desc(){
    int leftMargin=30,leftMargin2=120;
    int rightColumn1=680,rightColumn2=550;

    //left
    drawText(leftMargin,550,header,"3D Smart Plant Pot");

    drawText(leftMargin,520,content,"Rotate");
    drawText(leftMargin,500,content2,"s - Start");
    drawText(leftMargin,480,content2,"p - Pause");

    drawText(leftMargin,440,content,"View");
    drawText(leftMargin,420,content2,"t - Top");
    drawText(leftMargin,400,content2,"b - Bottom ");
    drawText(leftMargin,380,content2,"d - Default ");

    drawText(leftMargin2,440,content,"Zoom");
    drawText(leftMargin2,420,content2,"+ - Zoom In");
    drawText(leftMargin2,400,content2,"- - Zoom Out ");

    //right side
    drawText(rightColumn2,550,content,"Stopper");
    drawText(rightColumn2,530,content2,"w - Remove");
    drawText(rightColumn2,510,content2,"n - Back to Normal ");

    drawText(rightColumn1,550,content,"Lid");
    drawText(rightColumn1,530,content2,"o - Open Lid");
    drawText(rightColumn1,510,content2,"c - Close Lid");
    drawText(rightColumn1,490,content2,"r - Back to normal");

    drawText(rightColumn1,450,content,"Light");
    drawText(rightColumn1,430,content2,"l - Open");
    drawText(rightColumn1,410,content2,"k - Close");
}

void shadowMatrix(GLfloat shadowMat[4][4], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot;
    dot = groundplane[X] * lightpos[X] + groundplane[Y] * lightpos[Y] + groundplane[Z] * lightpos[Z] + groundplane[W] * lightpos[W];

    shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
    shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
    shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
    shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

    shadowMat[0][1] = 0.f - lightpos[Y] * groundplane[X];
    shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
    shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
    shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

    shadowMat[0][2] = 0.f - lightpos[Z] * groundplane[X];
    shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
    shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
    shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

    shadowMat[0][3] = 0.f - lightpos[W] * groundplane[X];
    shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
    shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
    shadowMat[3][3] = dot - lightpos[W] * groundplane[W];
}

void findPlane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3]) {
    GLfloat vec0[3], vec1[3];
    vec0[X] = v1[X] - v0[X];
    vec0[Y] = v1[Y] - v0[Y];
    vec0[Z] = v1[Z] - v0[Z];

    vec1[X] = v2[X] - v0[X];
    vec1[Y] = v2[Y] - v0[Y];
    vec1[Z] = v2[Z] - v0[Z];

    plane[A] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
    plane[B] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
    plane[C] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];
    plane[D] = -(plane[A] * v0[X] + plane[B] * v0[Y] + plane[C] * v0[Z]);
}

void drawSolidPetal() {
    GLfloat top[3], bottom[3];
    float thickness = 0.2f;

    glBegin(GL_POLYGON);
    for (int i = 0; i <= numdrawsegs; i++) {
        float t = (float)i / numdrawsegs;
        getBezierPoint(t, top);
        glVertex3f(top[0], top[1], top[2]);
    }
    for (int i = numdrawsegs; i >= 0; i--) {
        float t = (float)i / numdrawsegs;
        getBezierPoint(t, top);
        glVertex3f(top[0], -top[1], top[2]);
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i <= numdrawsegs; i++) {
        float t = (float)i / numdrawsegs;
        getBezierPoint(t, bottom);
        glVertex3f(bottom[0], bottom[1], bottom[2] - thickness);
    }
    for (int i = numdrawsegs; i >= 0; i--) {
        float t = (float)i / numdrawsegs;
        getBezierPoint(t, bottom);
        glVertex3f(bottom[0], -bottom[1], bottom[2] - thickness);
    }
    glEnd();

    for (int i = 0; i < numdrawsegs; i++) {
        float t1 = (float)i / numdrawsegs;
        float t2 = (float)(i + 1) / numdrawsegs;

        GLfloat pt1_top[3], pt2_top[3];
        GLfloat pt1_bot[3], pt2_bot[3];

        getBezierPoint(t1, pt1_top);
        getBezierPoint(t2, pt2_top);

        pt1_bot[0] = pt1_top[0]; pt1_bot[1] = pt1_top[1]; pt1_bot[2] = pt1_top[2] - thickness;
        pt2_bot[0] = pt2_top[0]; pt2_bot[1] = pt2_top[1]; pt2_bot[2] = pt2_top[2] - thickness;

        glBegin(GL_QUADS);
        glVertex3f(pt1_top[0], pt1_top[1], pt1_top[2]);
        glVertex3f(pt2_top[0], pt2_top[1], pt2_top[2]);
        glVertex3f(pt2_bot[0], pt2_bot[1], pt2_bot[2]);
        glVertex3f(pt1_bot[0], pt1_bot[1], pt1_bot[2]);
        glEnd();

        pt1_top[1] = -pt1_top[1]; pt2_top[1] = -pt2_top[1];
        pt1_bot[1] = -pt1_bot[1]; pt2_bot[1] = -pt2_bot[1];

        glBegin(GL_QUADS);
        glVertex3f(pt1_top[0], pt1_top[1], pt1_top[2]);
        glVertex3f(pt2_top[0], pt2_top[1], pt2_top[2]);
        glVertex3f(pt2_bot[0], pt2_bot[1], pt2_bot[2]);
        glVertex3f(pt1_bot[0], pt1_bot[1], pt1_bot[2]);
        glEnd();
    }
}

void drawScreen(bool isShadow=false){
    glPushMatrix();
    glTranslated(0,1.2,0);
    glRotatef(baseRotateAngle, 0, 1, 0);

    if (isShadow)
        glColor3f(0.0f, 0.0f, 0.0f);

    // screen
    if(!isShadow)
        glColor3ub(255,255,255);
    for(int i=35;i<135;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslatef(3.3,0,0);
        glRotatef(-90,1,0,0);
        glutSolidCylinder(0.05,3,32,32);
        glPopMatrix();
    }

    // Quote -
    if(!isShadow)
        glColor3ub(0,0,0);
    glPushMatrix();
    glRotatef(95,0,1,0);
    glTranslatef(3.35,0,0);
    glRotatef(-90,1,0,0);
    glutSolidCylinder(0.05,3,32,32);
    glPopMatrix();

    // Date -
    for(int i=35;i<95;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslated(3.35,2,0);

        if(!isShadow)
            glColor3ub(0,0,0);
        glRotatef(-90,1,0,0);
        glutSolidCylinder(0.05,0.1,32,32);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawPlate(bool isShadow=false){
    glPushMatrix();
    glRotatef(baseRotateAngle, 0, 1, 0);

    // Bottom
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);

        if(!isShadow)
            glColor3ub(101,67,33);
        glutSolidCylinder(0.1,4,32,32);
        glPopMatrix();
    }

    //Outer Layer
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslated(4,0,0);

        if(!isShadow)
            glColor3ub(101,67,33);
        glRotatef(-90,1,0,0);
        glRotatef(15,0,1,0);
        glutSolidCylinder(0.07,0.8,32,32);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawLEDlight(bool isShadow=false){
    if (!isShadow) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, ledArmTextures[currentTextureIndex]);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glScalef(0.1f, 1.0f, 1.0f);  // Shrinks X texture scale = repeat 3.3x
        glMatrixMode(GL_MODELVIEW);

        glColor3f(1,1,1);
    }

    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(baseRotateAngle, 0, 1, 0);

    if (isShadow)
        glColor3f(0.0f, 0.0f, 0.0f);

    // -
    glPushMatrix();
    glRotatef(90,0,1,0);
    gluCylinder(quad, 0.2, 0.2, 5, 32, 32);
    glPopMatrix();

    // |
    glPushMatrix();
    glTranslatef(8.5,15,0);
    glRotatef(90,1,0,0);
    glRotatef(-15,0,1,0);
    gluCylinder(quad, 0.2, 0.2, 16.5, 32, 32);
    glPopMatrix();

    // -
    glPushMatrix();
    glTranslatef(4.5,16.5,0);
    glRotatef(90,0,1,0);
    glRotatef(30,1,0,0);
    gluCylinder(quad, 0.2, 0.2, 5, 32, 32);
    glPopMatrix();

    // |
    glPushMatrix();
    glTranslatef(5,17,0);
    glRotatef(90,1,0,0);
    glRotatef(-30,0,1,0);
    gluCylinder(quad, 0.2, 0.2, 2.5, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(quad);

    if (!isShadow) {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();  // Reset
        glMatrixMode(GL_MODELVIEW);
        glDisable(GL_TEXTURE_2D);
    }


    // light
    if(!isShadow){
        if(open)
           glColor3ub(255,255,255);
        else
            glColor3ub(100,100,100);
    }
    glPushMatrix();
    glTranslatef(2.2,16.6,0);
    glRotatef(-50,0,0,1);
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslated(1.5,0,0);
        glutSolidCylinder(0.2,2,32,32);
        glPopMatrix();
    }
    glPopMatrix();

    // light on
    if(!isShadow)
        glColor3ub(87,88,87);
    glPushMatrix();
    glTranslatef(2.3,16.8,0);
    glRotatef(-50,0,0,1);
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslated(1.5,0,0);
        glutSolidCylinder(0.05,2,32,32);
        glPopMatrix();
    }
    glPopMatrix();

    // Joint
    if(!isShadow)
        glColor3ub(87,88,87);
    glPushMatrix();
    glTranslatef(4.5,0,0);
    glutSolidSphere(0.4,32,32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(8.25,14.25,0);
    glutSolidSphere(0.45,32,32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.75,16.25,0);
    glutSolidSphere(0.45,32,32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.8,14.9,0);
    glutSolidCube(1);
    glPopMatrix();

    glPopMatrix();

}

void drawLid(bool isShadow=false){
    glPushMatrix();
    glTranslatef(lidPosition/3,lidPosition,0);
    glRotatef(baseRotateAngle, 0, 1, 0);

    if (isShadow)
        glColor3f(0.0f, 0.0f, 0.0f);

    // Lid surface
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslatef(2,6,0);

        if(!isShadow)
            glColor3ub(110,22,22);
        glutSolidCylinder(0.1,3,32,32);
        glPopMatrix();
    }

    // Inner Lid
    for(int i=0;i<180;i++){
        glPushMatrix();
        glRotatef(i*2,0,1,0);
        glTranslated(1.9,5.9,0);

        if(!isShadow)
            glColor3ub(110,22,22);
        glRotatef(-90,1,0,0);
        glutSolidCylinder(0.07,0.2,32,32);
        glPopMatrix();
    }

    // Outer Lid
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glTranslated(3.6,5.6,0);

        if(!isShadow)
            glColor3ub(110,22,22);
        glRotatef(-90,1,0,0);
        glutSolidCylinder(0.07,0.5,32,32);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawPot(bool isShadow=false){
    glPushMatrix();
    glRotatef(baseRotateAngle, 0, 1, 0);

    if (isShadow)
        glColor3f(0.0f, 0.0f, 0.0f);

    // Layer
    if(!isShadow)
        glColor3ub(94,22,22);
    for(int i=0;i<360;i++){
        glPushMatrix();
        glRotatef(i,0,1,0);
        glRotatef(-90,1,0,0);

        // Inner Layer
        glPushMatrix();
        glTranslated(2,0,0);
        glutSolidCylinder(0.1,6,32,32);
        glPopMatrix();

        // Outer Layer
        glPushMatrix();
        glTranslated(3.1,0,0);
        glutSolidCylinder(0.15,6,32,32);
        glPopMatrix();

        glPopMatrix();
    }

    // Soil
    if(!isShadow)
        glColor3ub(98,76,54);
    glPushMatrix();
    glRotatef(-90,1,0,0);
    glutSolidCylinder(2,4.4,32,32);
    glPopMatrix();

    // Stopper
    if(!isShadow)
        glColor3ub(240,240,0);
    glPushMatrix();
    glTranslatef(0,3,3.1+stopperPostion);
    glutSolidCube(1);
    glPopMatrix();

    // Water (stopper)
    if(!isShadow)
        glColor3ub(137,207,240);
    glPushMatrix();
    glTranslatef(0,3,2.88);
    glutSolidCube(0.75);
    glPopMatrix();

    // Water (inside)
    for(int i=0;i<72;i++){
        glPushMatrix();
        glRotatef(i*5,0,1,0);
        glTranslatef(2.6,0,0);
        glRotatef(-90,1,0,0);
        glutSolidCylinder(0.4,4.5,32,32);
        glPopMatrix();
    }

    drawLid(isShadow);
    drawPlate(isShadow);
    glPopMatrix();
}

void drawSunflower(bool isShadow = false) {
    glPushMatrix();
    glTranslated(0, 4, 0);
    glRotatef(baseRotateAngle, 0, 1, 0);

    if (isShadow)
        glColor3f(0.0f, 0.0f, 0.0f);

    if (!isShadow)
        glColor3f(0,0.7,0);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCylinder(0.3, 10, 32, 32);
    glPopMatrix();


    if (!isShadow)
        glColor3ub(160,82,45);
    glPushMatrix();
    glTranslatef(0,10,-0.3);
    glRotatef(0,0.5,0,0);
    glutSolidCylinder(2,0.6,32,32);
    glPopMatrix();

    // Curve Petals
    // curve evaluator
    glMap1f(GL_MAP1_VERTEX_3,0.0,1.0,3,n,&controlpts[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glMapGrid1d(numdrawsegs,0.0,1.0);

    for(int i=0;i<2;i++){
        glPushMatrix();
        glTranslatef(0.0f,5.0f,0);

        if (i == 0) {
            glTranslatef(-0.75, 0.0f, 0.0f);
            glRotatef(-35, 0, 0, 1);
        } else {
            glTranslatef(2.0f, 0.0f, 0.0f);
            glRotatef(35, 0, 0, 1);
        }

        if (!isShadow)
            glColor3ub(0,128,0);
        glDisable(GL_CULL_FACE);
        glScalef(3.5,3,1);
        drawSolidPetal();
        glPopMatrix();

    }

    for(int i=0; i<12; i++) {
        glPushMatrix();
        glTranslatef(0.0f,10.3f,0);
        glRotatef(i*30,0,0,1);
        glTranslatef(2.8f,0,0.0f);

        if (!isShadow)
            glColor3ub(255,215,0);
        glDisable(GL_CULL_FACE);
        glScalef(3.5,3,1);
        drawSolidPetal();
        glPopMatrix();
    }

    for(int i=0; i<12; i++) {
        glPushMatrix();
        glTranslatef(0.0f,10.3f,0);
        glRotatef(i*30+15,0,0,1);
        glTranslatef(2.8f,0,0.0f);

        if (!isShadow)
            glColor3ub(255,240,120);
        glDisable(GL_CULL_FACE);
        glScalef(3,2.6,1);
        drawSolidPetal();
        glPopMatrix();
    }

    glPopMatrix();
}

static GLfloat floorVertices[4][3] = {
    { -25.0, 0.0, 25.0 },
    { 25.0, 0.0, 25.0 },
    { 25.0, 0.0, -25.0 },
    { -25.0, 0.0, -25.0 }
};

void drawFloor(void) {
    glBindTexture(GL_TEXTURE_2D, floorTextureID);
    glColor3ub(211,211,211);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);

    glTexCoord2f(0.0f, 0.0f); glVertex3fv(floorVertices[0]);
    glTexCoord2f(1.0f, 0.0f); glVertex3fv(floorVertices[1]);
    glTexCoord2f(1.0f, 1.0f); glVertex3fv(floorVertices[2]);
    glTexCoord2f(0.0f, 1.0f); glVertex3fv(floorVertices[3]);

    glEnd();
}


static GLfloat lightPosition[4], floorPlane[4], floorShadow[4][4];

void idle(void) {

    if (!lightMoving) {
        lightAngle += rotateSpeed;
    }
    glutPostRedisplay();
}

void rotateBase(int value) {
    baseRotateAngle += rotateSpeed;
    if (baseRotateAngle > 360.0f) baseRotateAngle -= 360.0f;
    glutPostRedisplay();
    glutTimerFunc(30, rotateBase, 0);
}

void keyboard(unsigned char key,int x,int y){
    // lid
    if (key == 'o')
        if (lidPosition < 2.5f)
            lidPosition += lidSpeed;
    if (key == 'c')
        if (lidPosition > 0.1f)
            lidPosition -= lidSpeed;
    if (key == 'r')
        lidPosition = 0.0f;

    // Stopper
    if(key=='w')
        stopperPostion=1;
    if(key=='n')
        stopperPostion=0;

    // zoom
    if(key=='+')
        if(cameraZ>=15.0f)
            cameraZ-=2.0f;
    if(key=='-')
        if(cameraZ<=38.0f)
            cameraZ+=2.0f;

    // view direction
    if(key=='t')
        cameraY=10.0f;
    if(key=='b')
        cameraY=2.0f;
    if(key=='d'){
        cameraZ=38.0f;
        cameraY=5.0f;
        cameraX=0.0f;
    }

    // rotate
    if(key=='s')
        rotateSpeed=0.5f;
    if(key=='p')
        rotateSpeed=0.0f;

    // light
    if(key=='l')
        open=true;
    if(key=='k')
        open=false;

    glutPostRedisplay();
}

void redraw(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(cameraX, cameraY, cameraZ, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0);

    // Set light position
    //lightAngle = -45;
    lightPosition[0] = -15.0f;
    lightPosition[1] = 30.0f;
    lightPosition[2] = 10.0f;
    lightPosition[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    GLfloat lightPosition2[4] = {15.0f, 25.0f, -10.0f, 2.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);

    shadowMatrix(floorShadow, floorPlane, lightPosition);
    glPushMatrix();
    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle1, 0.0, 1.0, 0.0);

    if(open)
        glEnable(GL_LIGHT1);
    else
        glDisable(GL_LIGHT1);

    // Draw the floor
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawFloor();
    glDisable(GL_BLEND);

    // Draw without shadow
    drawPot(false);
    drawScreen(false);
    drawSunflower(false);
    drawLEDlight(false);

    // Draw shadow
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDepthMask(GL_FALSE);
    glColor4f(0.1f, 0.1f, 0.1f,0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);

    glPushMatrix();
    glMultMatrixf((GLfloat*)floorShadow);
    drawPot(true);
    drawScreen(true);
    drawSunflower(true);
    drawLEDlight(true);
    glPopMatrix();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // Text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,800,0,600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1,1,1);

    glDisable(GL_TEXTURE_2D);
    desc();
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopMatrix();
    glutSwapBuffers();
}

void initGL() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glLineWidth(3.0);

    glPolygonOffset(-2.0, -2.0);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    GLfloat lightAmbient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHTING);

    GLfloat matAmb[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat matDif[] = { 1.f, 1.0, 1.f, 1.0 };
    GLfloat matSpe[] = { 0.5f, 0.5f, 0.5f, 1.0 };
    glLightfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmb);
    glLightfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDif);
    glLightfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpe);
    glLightf(GL_FRONT_AND_BACK, GL_SHININESS, 25);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    loadTexture("C:\\c\\year 3 sem 1\\computer graphics\\Assg3\\image5.jpg", ledArmTextures[0]);
    loadTexture("C:\\c\\year 3 sem 1\\computer graphics\\Assg3\\image2.jpg", ledArmTextures[1]);
    loadTexture("C:\\c\\year 3 sem 1\\computer graphics\\Assg3\\image4.jpg", ledArmTextures[2]);

    glDisable(GL_TEXTURE_2D);

    glClearColor(0.2,0.2,0.2,1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.f, 1.f, 0.1f, 100.f);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(cameraX,cameraY,cameraZ, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(cameraAngle,0.0f,1.0f,0.0f);
    findPlane(floorPlane, floorVertices[1], floorVertices[2], floorVertices[3]);
}

void onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        currentTextureIndex = (currentTextureIndex + 1) % 3;
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Final Project - 3D Smart Plant Pot");
    initGL();
    glutDisplayFunc(redraw);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(onMouseClick);
    glutIdleFunc(idle);
    glutTimerFunc(25,rotateBase,0);
    glutMainLoop();
    return 0;
}
