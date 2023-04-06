#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
static int stencilReflection = 1, stencilShadow = 1, offsetShadow = 1;
static int renderShadow = 1, renderDinosaur = 1, renderReflection = 1;
static int linearFiltering = 0, useMipmaps = 0;
static int reportSpeed = 0;
static int animation = 1;
static GLboolean lightSwitch = GL_TRUE;
static int directionalLight = 1;
static int forceExtension = 0;
static float lightAngle = 0.0, lightHeight = 20;
GLfloat angle = -150;
GLfloat angle2 = 30;
int moving, startx, starty;
int lightMoving = 0, lightStartX, lightStartY;
enum {
  MISSING, EXTENSION, ONE_DOT_ONE
};
int polygonOffsetVersion;
static GLdouble bodyWidth = 3.0;
static GLfloat body[][2] = { {0, 3}, {1, 1}, {5, 1}, {8, 4}, {10, 4}, {11, 5},
  {11, 11.5}, {13, 12}, {13, 13}, {10, 13.5}, {13, 14}, {13, 15}, {11, 16},
  {8, 16}, {7, 15}, {7, 13}, {8, 12}, {7, 11}, {6, 6}, {4, 3}, {3, 2},
  {1, 2} };
static GLfloat arm[][2] = { {8, 10}, {9, 9}, {10, 9}, {13, 8}, {14, 9}, {16, 9},
  {15, 9.5}, {16, 10}, {15, 10}, {15.5, 11}, {14.5, 10}, {14, 11}, {14, 10},
  {13, 9}, {11, 11}, {9, 11} };
static GLfloat leg[][2] = { {8, 6}, {8, 4}, {9, 3}, {9, 2}, {8, 1}, {8, 0.5}, {9, 0},
  {12, 0}, {10, 1}, {10, 2}, {12, 4}, {11, 6}, {10, 7}, {9, 7} };
static GLfloat eye[][2] = { {8.75, 15}, {9, 14.7}, {9.6, 14.7}, {10.1, 15},
  {9.6, 15.25}, {9, 15.25} };
static GLfloat lightPosition[4];
static GLfloat lightColor[] = {0.8, 1.0, 0.8, 1.0};
static GLfloat skinColor[] = {0.1, 1.0, 0.1, 1.0}, eyeColor[] = {1.0, 0.2, 0.2, 1.0};
enum {
  X, Y, Z, W
};
enum {
  A, B, C, D
};
void shadowMatrix(GLfloat shadowMat[4][4],
  GLfloat groundplane[4],
  GLfloat lightpos[4])
{
  GLfloat dot;
  dot = groundplane[X] * lightpos[X] +
    groundplane[Y] * lightpos[Y] +
    groundplane[Z] * lightpos[Z] +
    groundplane[W] * lightpos[W];
  shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
  shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
  shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
  shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];
  shadowMat[X][1] = 0.f - lightpos[Y] * groundplane[X];
  shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
  shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
  shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];
  shadowMat[X][2] = 0.f - lightpos[Z] * groundplane[X];
  shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
  shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
  shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];
  shadowMat[X][3] = 0.f - lightpos[W] * groundplane[X];
  shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
  shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
  shadowMat[3][3] = dot - lightpos[W] * groundplane[W];
}
void findPlane(GLfloat plane[4],
  GLfloat v0[3], GLfloat v1[3], GLfloat v2[3])
{
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
void extrudeSolidFromPolygon(GLfloat data[][2], unsigned int dataSize,
  GLdouble thickness, GLuint side, GLuint edge, GLuint whole)
{
  static GLUtriangulatorObj *tobj = NULL;
  GLdouble vertex[3], dx, dy, len;
  int i;
  int count = dataSize / (2 * sizeof(GLfloat));
  if (tobj == NULL) {
    tobj = gluNewTess();
    gluTessCallback(tobj, GLU_BEGIN, glBegin);
    gluTessCallback(tobj, GLU_VERTEX, glVertex2fv);
    gluTessCallback(tobj, GLU_END, glEnd);
  }
  glNewList(side, GL_COMPILE);
  glShadeModel(GL_SMOOTH);
  gluBeginPolygon(tobj);
  for (i = 0; i < count; i++) {
    vertex[0] = data[i][0];
    vertex[1] = data[i][1];
    vertex[2] = 0;
    gluTessVertex(tobj, vertex, data[i]);
  }
  gluEndPolygon(tobj);
  glEndList();
  glNewList(edge, GL_COMPILE);
  glShadeModel(GL_FLAT);
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= count; i++) {
    glVertex3f(data[i % count][0], data[i % count][1], 0.0);
    glVertex3f(data[i % count][0], data[i % count][1], thickness);
    dx = data[(i + 1) % count][1] - data[i % count][1];
    dy = data[i % count][0] - data[(i + 1) % count][0];
    len = sqrt(dx * dx + dy * dy);
    glNormal3f(dx / len, dy / len, 0.0);
  }
  glEnd();
  glEndList();
  glNewList(whole, GL_COMPILE);
  glFrontFace(GL_CW);
  glCallList(edge);
  glNormal3f(0.0, 0.0, -1.0);
  glCallList(side);
  glPushMatrix();
  glTranslatef(0.0, 0.0, thickness);
  glFrontFace(GL_CCW);
  glNormal3f(0.0, 0.0, 1.0);
  glCallList(side);
  glPopMatrix();
  glEndList();
}
typedef enum {
  RESERVED, BODY_SIDE, BODY_EDGE, BODY_WHOLE, ARM_SIDE, ARM_EDGE, ARM_WHOLE,
  LEG_SIDE, LEG_EDGE, LEG_WHOLE, EYE_SIDE, EYE_EDGE, EYE_WHOLE
} displayLists;
static void makeDinosaur(void)
{
  extrudeSolidFromPolygon(body, sizeof(body), bodyWidth,
    BODY_SIDE, BODY_EDGE, BODY_WHOLE);
  extrudeSolidFromPolygon(arm, sizeof(arm), bodyWidth / 4,
    ARM_SIDE, ARM_EDGE, ARM_WHOLE);
  extrudeSolidFromPolygon(leg, sizeof(leg), bodyWidth / 2,
    LEG_SIDE, LEG_EDGE, LEG_WHOLE);
  extrudeSolidFromPolygon(eye, sizeof(eye), bodyWidth + 0.2,
    EYE_SIDE, EYE_EDGE, EYE_WHOLE);
}
static void drawDinosaur(void)
{
  glPushMatrix();
  glTranslatef(-8, 0, -bodyWidth / 2);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, skinColor);
  glCallList(BODY_WHOLE);
  glTranslatef(0.0, 0.0, bodyWidth);
  glCallList(ARM_WHOLE);
  glCallList(LEG_WHOLE);
  glTranslatef(0.0, 0.0, -bodyWidth - bodyWidth / 4);
  glCallList(ARM_WHOLE);
  glTranslatef(0.0, 0.0, -bodyWidth / 4);
  glCallList(LEG_WHOLE);
  glTranslatef(0.0, 0.0, bodyWidth / 2 - 0.1);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, eyeColor);
  glCallList(EYE_WHOLE);
  glPopMatrix();
}
static GLfloat floorVertices[4][3] = {
  { -20.0, 0.0, 20.0 },
  { 20.0, 0.0, 20.0 },
  { 20.0, 0.0, -20.0 },
  { -20.0, 0.0, -20.0 },
};
static void drawFloor(void)
{
  glDisable(GL_LIGHTING);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(floorVertices[0]);
    glTexCoord2f(0.0, 16.0);
    glVertex3fv(floorVertices[1]);
    glTexCoord2f(16.0, 16.0);
    glVertex3fv(floorVertices[2]);
    glTexCoord2f(16.0, 0.0);
    glVertex3fv(floorVertices[3]);
  glEnd();
  glEnable(GL_LIGHTING);
}
static GLfloat floorPlane[4];
static GLfloat floorShadow[4][4];
static void redraw(void)
{
  int start, end;
  if (reportSpeed) {
    start = glutGet(GLUT_ELAPSED_TIME);
  }
  if ((stencilReflection && renderReflection) || (stencilShadow && renderShadow)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  lightPosition[0] = 12*cos(lightAngle);
  lightPosition[1] = lightHeight;
  lightPosition[2] = 12*sin(lightAngle);
  if (directionalLight) {
    lightPosition[3] = 0.0;
  } else {
    lightPosition[3] = 1.0;
  }
  shadowMatrix(floorShadow, floorPlane, lightPosition);
  glPushMatrix();
    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    if (renderReflection) {
      if (stencilReflection) {
        glDisable(GL_DEPTH_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
        drawFloor();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_EQUAL, 1, 0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      }
      glPushMatrix();
        glScalef(1.0, -1.0, 1.0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        glEnable(GL_NORMALIZE);
        glCullFace(GL_FRONT);
        drawDinosaur();
        glDisable(GL_NORMALIZE);
        glCullFace(GL_BACK);
      glPopMatrix();
      glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
      if (stencilReflection) {
        glDisable(GL_STENCIL_TEST);
      }
    }
    glFrontFace(GL_CW);
    glColor4f(0.1, 0.1, 0.7, 1.0);
    drawFloor();
    glFrontFace(GL_CCW);
    if (renderShadow) {
      if (stencilShadow) {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 3, 0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      }
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.7, 0.0, 0.0, 0.3);
    glColor4f(1.0, 1.0, 1.0, 0.3);
    drawFloor();
    glDisable(GL_BLEND);
    if (renderDinosaur) {
      drawDinosaur();
    }
    if (renderShadow) {
      if (stencilShadow) {
        glStencilFunc(GL_LESS, 2, 0xffffffff);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      }
      if (offsetShadow) {
    switch (polygonOffsetVersion) {
    case EXTENSION:
#ifdef GL_EXT_polygon_offset
      glEnable(GL_POLYGON_OFFSET_EXT);
      break;
#endif
#ifdef GL_VERSION_1_1
    case ONE_DOT_ONE:
          glEnable(GL_POLYGON_OFFSET_FILL);
      break;
#endif
    case MISSING:
      break;
    }
      }
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_LIGHTING);
      glColor4f(0.0, 0.0, 0.0, 0.5);
      glPushMatrix();
        glMultMatrixf((GLfloat *) floorShadow);
        drawDinosaur();
      glPopMatrix();
      glDisable(GL_BLEND);
      glEnable(GL_LIGHTING);
      if (offsetShadow) {
    switch (polygonOffsetVersion) {
#ifdef GL_EXT_polygon_offset
    case EXTENSION:
      glDisable(GL_POLYGON_OFFSET_EXT);
      break;
#endif
#ifdef GL_VERSION_1_1
    case ONE_DOT_ONE:
          glDisable(GL_POLYGON_OFFSET_FILL);
      break;
#endif
    case MISSING:
      break;
    }
      }
      if (stencilShadow) {
        glDisable(GL_STENCIL_TEST);
      }
    }
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 0.0);
    if (directionalLight) {
      glDisable(GL_CULL_FACE);
      glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
      glRotatef(lightAngle * -180.0 / M_PI, 0, 1, 0);
      glRotatef(atan(lightHeight/12) * 180.0 / M_PI, 0, 0, 1);
      glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0);
    glVertex3f(2, 1, 1);
    glVertex3f(2, -1, 1);
    glVertex3f(2, -1, -1);
    glVertex3f(2, 1, -1);
    glVertex3f(2, 1, 1);
      glEnd();
      glColor3f(1.0, 1.0, 1.0);
      glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(5, 0, 0);
      glEnd();
      glEnable(GL_CULL_FACE);
    } else {
      glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
      glutSolidSphere(1.0, 5, 5);
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
  glPopMatrix();
  if (reportSpeed) {
    glFinish();
    end = glutGet(GLUT_ELAPSED_TIME);
    printf("Speed %.3g frames/sec (%d ms)\n", 1000.0/(end-start), end-start);
  }
  glutSwapBuffers();
}
static void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      moving = 1;
      startx = x;
      starty = y;
    }
    if (state == GLUT_UP) {
      moving = 0;
    }
  }
  if (button == GLUT_MIDDLE_BUTTON) {
    if (state == GLUT_DOWN) {
      lightMoving = 1;
      lightStartX = x;
      lightStartY = y;
    }
    if (state == GLUT_UP) {
      lightMoving = 0;
    }
  }
}
static void motion(int x, int y)
{
  if (moving) {
    angle = angle + (x - startx);
    angle2 = angle2 + (y - starty);
    startx = x;
    starty = y;
    glutPostRedisplay();
  }
  if (lightMoving) {
    lightAngle += (x - lightStartX)/40.0;
    lightHeight += (lightStartY - y)/20.0;
    lightStartX = x;
    lightStartY = y;
    glutPostRedisplay();
  }
}
static void idle(void)
{
  static float time = 0.0;
  time = glutGet(GLUT_ELAPSED_TIME) / 500.0;
  if (!lightMoving) {
    lightAngle += 0.03;
  }
  glutPostRedisplay();
}
enum {
  M_NONE, M_MOTION, M_SHADOWS, M_REFLECTION,
  M_POSITIONAL, M_DIRECTIONAL, M_PERFORMANCE
};
static void controlLights(int value)
{
  switch (value) {
  case M_NONE:
    return;
  case M_MOTION:
    animation = 1 - animation;
    if (animation) {
      glutIdleFunc(idle);
    } else {
      glutIdleFunc(NULL);
    }
    break;
  case M_SHADOWS:
    renderShadow = 1 - renderShadow;
    break;
  case M_REFLECTION:
    renderReflection = 1 - renderReflection;
    break;
  case M_POSITIONAL:
    directionalLight = 0;
    break;
  case M_DIRECTIONAL:
    directionalLight = 1;
    break;
  case M_PERFORMANCE:
    reportSpeed = 1 - reportSpeed;
    break;
  }
  glutPostRedisplay();
}
static void visible(int vis)
{
  if (vis == GLUT_VISIBLE) {
    if (animation)
      glutIdleFunc(idle);
  } else {
    if (!animation)
      glutIdleFunc(NULL);
  }
}
static void key(unsigned char c, int x, int y)
{
  if (c == 27) {
    exit(0);
  }
  glutPostRedisplay();
}
static void special(int k, int x, int y)
{
  glutPostRedisplay();
}
static int supportsOneDotOne(void)
{
  const char *version;
  int major, minor;
  version = (char *) glGetString(GL_VERSION);
  if (sscanf(version, "%d.%d", &major, &minor) == 2)
    return major >= 1 && minor >= 1;
  return 0;
}
int main(int argc, char **argv)
{
  int i;
  glutInit(&argc, argv);
  for (i=1; i<argc; i++) {
    if (!strcmp("-linear", argv[i])) {
      linearFiltering = 1;
    } else if (!strcmp("-mipmap", argv[i])) {
      useMipmaps = 1;
    } else if (!strcmp("-ext", argv[i])) {
      forceExtension = 1;
    }
  }
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
#if 0
  glutInitDisplayString("samples stencil>=2 rgb double depth");
#endif
  glutCreateWindow("Part 2");
  glutDisplayFunc(redraw);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutVisibilityFunc(visible);
  glutKeyboardFunc(key);
  glutSpecialFunc(special);
  glutCreateMenu(controlLights);
  glutAddMenuEntry("Toggle motion", M_MOTION);
  glutAddMenuEntry("Toggle shadows", M_SHADOWS);
  glutAddMenuEntry("Toggle reflection", M_REFLECTION);
  glutAddMenuEntry("Positional light", M_POSITIONAL);
  glutAddMenuEntry("Directional light", M_DIRECTIONAL);
  glutAddMenuEntry("Display FPS", M_PERFORMANCE);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  makeDinosaur();
#ifdef GL_VERSION_1_1
  if (supportsOneDotOne() && !forceExtension) {
    polygonOffsetVersion = ONE_DOT_ONE;
    glPolygonOffset(-2.0, -1.0);
  } else
#endif
  {
#ifdef GL_EXT_polygon_offset
  if (glutExtensionSupported("GL_EXT_polygon_offset")) {
    polygonOffsetVersion = EXTENSION;
    glPolygonOffsetEXT(-0.1, -0.002);
  } else
#endif
    {
      polygonOffsetVersion = MISSING;
      printf("\ndinoshine: Missing polygon offset.\n");
      printf("           Expect shadow depth aliasing artifacts.\n\n");
    }
  }
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( 40.0, 1.0, 20.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 8.0, 60.0,
    0.0, 8.0, 0.0,
    0.0, 1.0, 0.);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  findPlane(floorPlane, floorVertices[1], floorVertices[2], floorVertices[3]);
  glutMainLoop();
  return 0;
}
