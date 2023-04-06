#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SOIL/SOIL.h>
#include <cstdio>
/* u or U is for UP , d or D is for Down
 * Notes : Global changes whole scene color
 *  Specular only change the cow surface reflacitions
 */
float rotationAngle = 0;
bool flat_flag = false;
int width = 800, height = 800;
float camX = 0.0, camY = 0.5, camZ = 0.0;
float camYaw = 1.5, camPitch = 2.5;
int mouseX = 0, mouseY = 0;
static int reportSpeed = 0;

bool light0Enabled = true;
bool light1Enabled = true;
bool light2Enabled = true;

GLuint cowTexture;

GLfloat mat_ambient[] = {0.5, 0.5, 0.5, 1.0};
GLfloat mat_diffuse[] = {0.8, 0.8, 0.8, 1.0};
GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_shininess[] = {100.0};

GLfloat light0_position[] = {-5.0, 5.0, 5.0, 1.0};
GLfloat light0_color[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light1_position[] = {5.0, 5.0, -10.0, 1.0};
GLfloat light1_color[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light2_position[] = {5.0, 5.0, 5.0, 1.0};
GLfloat light2_color[] = {1.0, 1.0, 1.0, 1.0};

void makingMenu();
void drawLights();

enum {
    M_NONE,M_EXIT,M_LIGHT0,M_LIGHT1,M_LIGHT2,M_FLAT_GOURAUD,M_R,M_LR,M_G,M_LG,M_B,M_LB,M_RESET,M_A,M_LA,
    M_SR,M_SLR,M_SG,M_SLG,M_SB,M_SLB,M_SA,M_SLA,M_FPS
};

static void controlLights(int value)
{
    switch (value) {
        case M_NONE:
            return;
        case M_EXIT:
            exit(0);
        case M_LIGHT0:
            light0Enabled = !light0Enabled;
            break;
        case M_LIGHT1:
            light1Enabled = !light1Enabled;
            break;
        case M_LIGHT2:
            light2Enabled = !light2Enabled;
            break;
        case M_FLAT_GOURAUD:
            flat_flag = !flat_flag;
            break;
        case M_R:
            light0_color[0] += 0.5;
            break;
        case M_LR:
            light0_color[0] -= 0.5;
            break;
        case M_B:
            light0_color[2] += 0.5;
            break;
        case M_LB:
            light0_color[2] -= 0.5;
            break;
        case M_G:
            light0_color[1] += 0.5;
            break;
        case M_LG:
            light0_color[1] -= 0.5;
            break;
        case M_A:
            light0_color[3] += 0.5;
            break;
        case M_LA:
            light0_color[3] -= 0.5;
            break;
        case M_SR:
             mat_specular[0] += 0.5;
             break;
         case M_SLR:
            mat_specular[0] -= 0.5;
            break;
        case M_SG:
             mat_specular[1] += 0.5;
             break;
         case M_SLG:
             mat_specular[1] -= 0.5;
            break;
        case M_SB:
             mat_specular[2] += 0.5;
            break;
        case M_SLB:
             mat_specular[2] -= 0.5;
            break;
        case M_SA:
             mat_specular[3] += 0.5;
            break;
        case M_SLA:
             mat_specular[3] -= 0.5;
            break;
        case M_RESET:
            light0_color[0] = 1.0;
            light0_color[1] = 1.0;
            light0_color[2] = 1.0;
            light0_color[3] = 1.0;
            mat_specular[0] = 1.0;
            mat_specular[1] = 1.0;
            mat_specular[2] = 1.0;
            mat_specular[3] = 1.0;
        break;
        case M_FPS:
            reportSpeed = 1 - reportSpeed;
            break;


    }
    glutPostRedisplay();
}



void drawCows(float x, float y, float z) {

    drawLights();
    glEnable(GL_DEPTH_TEST);

    // Set material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_color);

    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_color);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_color);

    glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_color);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_color);

    glPushMatrix();
    glTranslatef(x, y, z + 5.0);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile("..\\..\\Downloads\\cow.obj", aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene) {
        std::cerr << "Failed to load file: " << importer.GetErrorString() << std::endl;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, cowTexture);
    glEnable(GL_TEXTURE_2D);

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh *mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            const aiFace &face = mesh->mFaces[j];
            // Draw the mesh
            glBegin(GL_TRIANGLES);
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                unsigned int index = face.mIndices[k];
                glTexCoord2f(mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y);
                glNormal3f(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
                glVertex3f(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
                if(k==2){
                    glEnd();
                }
            }
        }
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawLights() {
    if (light0Enabled) {
        glPushMatrix();
        glTranslatef(-9.0, 9.0, 0.0);
        glutSolidSphere(.3, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHT0);
    } else {
        glDisable(GL_LIGHT0);
    }

    if (light1Enabled) {
        glPushMatrix();
        glTranslatef(0.0, 9.0, 0.0);
        glutSolidSphere(.3, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHT1);
    } else {
        glDisable(GL_LIGHT1);
    }

    if (light2Enabled) {
        glPushMatrix();
        glTranslatef(9.0, 9.0, 0.0);
        glutSolidSphere(.3, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHT2);
    } else {
        glDisable(GL_LIGHT2);
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseX = x;
        mouseY = y;
    }
}

void motion(int x, int y) {
    float deltaX = x - mouseX;
    float deltaY = y - mouseY;

    mouseX = x;
    mouseY = y;

    camYaw -= deltaX * 0.005f;
    camPitch -= deltaY * 0.005f;

    if (camPitch < -1.5f)
        camPitch = -1.5f;
    else if (camPitch > 1.5f)
        camPitch = 1.5f;

    glutPostRedisplay();
}

void special(int key, int x, int y) {
    const float moveSpeed = 0.1f;
    switch (key) {
        case GLUT_KEY_UP:
            camZ -= moveSpeed;
            break;
        case GLUT_KEY_DOWN:
            camZ += moveSpeed;
            break;
        case GLUT_KEY_LEFT:
            camX -= moveSpeed;
            break;
        case GLUT_KEY_RIGHT:
            camX += moveSpeed;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    const float moveSpeed = 0.1f;
    switch (key) {
        case 'u':
        case 'U':
            camY += moveSpeed;
            break;
        case 'd':
        case 'D':
            camY -= moveSpeed;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}


void drawWalls(float width, float length, float alpha){

    //Ground
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-15.0, 0.0, -15.0);
    glVertex3f(-15.0, 0.0, 15.0);
    glVertex3f(15.0, 0.0, 15.0);
    glVertex3f(15.0, 0.0, -15.0);
    glEnd();

    // Left wall
    glBegin(GL_QUADS);
    glColor3f(0.2, 0.2, 0.2);
    glVertex3f(-15.0, 0.0, -15.0);
    glVertex3f(-15.0, 10.0, -15.0);
    glVertex3f(-15.0, 10.0, 15.0);
    glVertex3f(-15.0, 0.0, 15.0);
    glEnd();

    // Right wall
    glBegin(GL_QUADS);
    glColor3f(0.2, 0.2, 0.2);
    glVertex3f(15.0, 0.0, -15.0);
    glVertex3f(15.0, 0.0, 15.0);
    glVertex3f(15.0, 10.0, 15.0);
    glVertex3f(15.0, 10.0, -15.0);
    glEnd();

    // Back wall
    glBegin(GL_QUADS);
    glColor3f(0.2, 0.2, 0.2);
    glVertex3f(-15.0, 0.0, -15.0);
    glVertex3f(15.0, 0.0, -15.0);
    glVertex3f(15.0, 10.0, -15.0);
    glVertex3f(-15.0, 10.0, -15.0);
    glEnd();
}


void initRendering(){

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightAmbient[] = {0.5f, 0.5f ,0.5f, 1.0f};
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void handleResize(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 1000.0);
}

void drawScene(){
 int start, end;
  if (reportSpeed) {
    start = glutGet(GLUT_ELAPSED_TIME);
  }
    glClearStencil(0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -45.0f);
    glRotatef(45, 1.0f, 0.0f, 0.0f);

    GLfloat lightPosition[] = {0.0f, 10.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    gluLookAt(camX, camY, camZ, camX + cos(camYaw), camY + sin(camPitch), camZ - sin(camYaw), 0.0, 1.0, 0.0);

    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    drawCows(1.0f, 1.0f, 1.0f);
    drawCows(5.0f, 1.0f, 5.0f);
    drawCows(-5.0f, 1.0f, -5.0f);

    glPopMatrix();
    glShadeModel(flat_flag ? GL_FLAT : GL_SMOOTH);

    glEnable(GL_STENCIL_TEST);
    glColorMask(0, 0, 0, 0);
    glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    drawWalls(30.0f, 30.f, 1.0f);
    glColorMask(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


    glPushMatrix();
    glScalef(1, -1, 1);
    glTranslatef(0, 5.0f, 0);
    drawCows(1.0f, 1.0f, 1.0f);
    drawCows(5.0f, 1.0f, 5.0f);
    drawCows(-5.0f, 1.0f, -5.0f);

    glPopMatrix();

    glDisable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    drawWalls(30.0f, 30.f, 0.8f);
    glDisable(GL_BLEND);
if (reportSpeed) {
    glFinish();
    end = glutGet(GLUT_ELAPSED_TIME);
    printf("Speed %.3g frames/sec (%d ms)\n", 1000.0/(end-start), end-start);
  }
    glutSwapBuffers();
}

void update(int value){
    rotationAngle += 1.0f;
    if (rotationAngle > 360)
        rotationAngle = 0;
    glutPostRedisplay();
    glutTimerFunc(25, update, 0);
}


int main(int argc, char *argv[]){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Part 1");
    makingMenu();


    GLenum err = glewInit();
    if (err != GLEW_OK) {
        printf("Error initializing GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }
    cowTexture = SOIL_load_OGL_texture("..\\..\\Downloads\\cow-tex-fin.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (cowTexture == 0) {
        printf("Error loading texture: '%s'\n", SOIL_last_result());
    }

    initRendering();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_COLOR_MATERIAL);

    glShadeModel(GL_SMOOTH);

    glutDisplayFunc(drawScene);
    gluPerspective(60.0, (GLfloat) width / (GLfloat) height, 0.1, 100.0);
    glutTimerFunc(25, update, 0);
    glutReshapeFunc(handleResize);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();

    return 0;
}

void makingMenu() {
    glutCreateMenu(controlLights);
    glutAddMenuEntry("Light 1 Global", M_LIGHT0);
    glutAddMenuEntry("Light 2", M_LIGHT1);
    glutAddMenuEntry("Light 3", M_LIGHT2);
    glutAddMenuEntry("----------------------------", M_NONE);
    glutAddMenuEntry("Flat/Gouraud", M_FLAT_GOURAUD);
    glutAddMenuEntry("----------------------------", M_NONE);
    glutAddMenuEntry("Global Ambient Lights", M_NONE);
    glutAddMenuEntry("Increase Red ", M_R);
    glutAddMenuEntry("Decrease Red ", M_LR);
    glutAddMenuEntry("Increase Blue ", M_B);
    glutAddMenuEntry("Decrease Blue ", M_LB);
    glutAddMenuEntry("Increase Green ", M_G);
    glutAddMenuEntry("Decrease Green ", M_LG);
    glutAddMenuEntry("Increase Alpha ", M_A);
    glutAddMenuEntry("Decrease Alpha ", M_LA);

    glutAddMenuEntry("----------------------------", M_NONE);
    glutAddMenuEntry("Specular Material Properties", M_NONE);
    glutAddMenuEntry("Increase Red ", M_SR);
    glutAddMenuEntry("Decrease Red ", M_SLR);
    glutAddMenuEntry("Increase Blue ", M_SB);
    glutAddMenuEntry("Decrease Blue ", M_SLB);
    glutAddMenuEntry("Increase Green ", M_SG);
    glutAddMenuEntry("Decrease Green ", M_SLG);
    glutAddMenuEntry("Increase Alpha ", M_SA);
    glutAddMenuEntry("Decrease Alpha ", M_SLA);
    glutAddMenuEntry("----------------------------", M_NONE);
    glutAddMenuEntry("Reset Global And Specular", M_RESET);
    glutAddMenuEntry("Display FPS", M_FPS);

    glutAddMenuEntry("Exit", M_EXIT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}
