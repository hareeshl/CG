#include <stdio.h>    
#include <stdlib.h>   
#include <math.h>     
#include <assert.h>   

#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifdef _WIN32
#include <GL/wglew.h>
#else
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/glxew.h>
#endif
#endif

#include <Cg/cg.h>    
#include <Cg/cgGL.h>
#include<glm.hpp>
#include "vector3d.h"
#include<iostream>
#include<string>
#include<fstream>

using namespace std;

int winW=250,winH=250;
int eyex_f,eyey_f,eyez_f;
int viewx_f,viewy_f,viewz_f;
int lookatx_f,lookaty_f,lookatz_f;

static CGcontext   myCgContext;
static CGprofile   myCgVertexProfile,
                   myCgFragmentProfile;

static CGprogram   myCgVertexProgram,
                   myCgFragmentProgram;

static CGparameter myCgVertexParam_modelViewProj,
                   myCgFragmentParam_globalAmbient,
                   myCgFragmentParam_lightColor,
                   myCgFragmentParam_lightPosition,
                   myCgFragmentParam_eyePosition,
                   myCgFragmentParam_Ke,
                   myCgFragmentParam_Ka,
                   myCgFragmentParam_Kd,
                   myCgFragmentParam_Ks,
                   myCgFragmentParam_shininess;

static CGparameter myCgFragmentParam_decal;

static const char *myProgramName = "fragment_lighting",
                  *myVertexProgramFileName = "fragmentLighting.cg",
				  *myVertexProgramName = "fragmentLighting",
                  *myFragmentProgramFileName = "basicLight.cg",
			      *myFragmentProgramName = "basicLight";

static const GLubyte
myDemonTextureImage[3*(128*128)] = {
#include "demon_image.h"
};

static float myLightAngle = -0.4;  
static float myProjectionMatrix[16];
static float myGlobalAmbient[3] = { 1, 1, 1 }; 
static float myLightColor[3] = { 1, 1, 1 };  
static const float Kd[3] = { 1, 1, 1 };

static void checkForCgError(const char *situation)
{
  CGerror error;
  const char *string = cgGetLastErrorString(&error);

  if (error != CG_NO_ERROR) {
    printf("%s: %s: %s\n",
      myProgramName, situation, string);
    if (error == CG_COMPILER_ERROR) {
      printf("%s\n", cgGetLastListing(myCgContext));
    }
    exit(1);
  }
}

/* Forward declared GLUT callbacks registered by main. */
static void reshape(int width, int height);
static void display(void);
static void keyboard(unsigned char c, int x, int y);
static void menu(int item);
static void requestSynchronizedSwapBuffers(void);
static void setupDemonSampler(void);

int main(int argc, char **argv)
{
  //Get the window co-ords from window.txt
  string line;
  fstream file ("window.txt",ios::in);
  getline(file,line);
  sscanf(line.c_str(),"%d %d",&winW,&winH);   
  file.close();

  //Get eye,viewup,lookat details
  string line2;   
  fstream file2 ("view.txt",ios::in);
  getline(file2,line2);
  sscanf(line2.c_str(),"%d %d %d %d %d %d %d %d %d",&eyex_f,&eyey_f,&eyez_f,&viewx_f,&viewy_f,&viewz_f,&lookatx_f,&lookaty_f,&lookatz_f);   
  file2.close();

  glutInitWindowSize(winW, winH);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInit(&argc, argv);

  glutCreateWindow(myProgramName);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutReshapeFunc(reshape);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_2D, 666);
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8,
    128, 128, GL_RGB, GL_UNSIGNED_BYTE, myDemonTextureImage);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  /* Initialize OpenGL entry points. */
  if (glewInit()!=GLEW_OK || !GLEW_VERSION_1_1) {
    fprintf(stderr, "%s: failed to initialize GLEW, OpenGL 1.1 required.\n", myProgramName);    
    exit(1);
  }

  requestSynchronizedSwapBuffers();
  glClearColor(0, 0, 0, 0);  
  glEnable(GL_DEPTH_TEST);   

  myCgContext = cgCreateContext();
  checkForCgError("creating context");
  cgGLSetDebugMode(CG_FALSE);
  cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);

  myCgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  cgGLSetOptimalOptions(myCgVertexProfile);
  checkForCgError("selecting vertex profile");

  myCgVertexProgram =
    cgCreateProgramFromFile(
      myCgContext,             
      CG_SOURCE,               
      myVertexProgramFileName, 
      myCgVertexProfile,       
      myVertexProgramName,     
      NULL);                   
  checkForCgError("creating vertex program from file");
  cgGLLoadProgram(myCgVertexProgram);
  checkForCgError("loading vertex program");

#define GET_VERTEX_PARAM(name) \
  myCgVertexParam_##name = \
    cgGetNamedParameter(myCgVertexProgram, #name); \
  checkForCgError("could not get " #name " parameter");

  GET_VERTEX_PARAM(modelViewProj);

  myCgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(myCgFragmentProfile);
  checkForCgError("selecting fragment profile");

  myCgFragmentProgram =
    cgCreateProgramFromFile(
      myCgContext,              
      CG_SOURCE,                
      myFragmentProgramFileName,
      myCgFragmentProfile,      
      myFragmentProgramName,    
      NULL); 
  checkForCgError("creating fragment program from string");
  cgGLLoadProgram(myCgFragmentProgram);
  checkForCgError("loading fragment program");

#define GET_FRAGMENT_PARAM(name) \
  myCgFragmentParam_##name = \
    cgGetNamedParameter(myCgFragmentProgram, #name); \
  checkForCgError("could not get " #name " parameter");

  GET_FRAGMENT_PARAM(globalAmbient);
  GET_FRAGMENT_PARAM(lightColor);
  GET_FRAGMENT_PARAM(lightPosition);
  GET_FRAGMENT_PARAM(eyePosition);
  GET_FRAGMENT_PARAM(Ke);
  GET_FRAGMENT_PARAM(Ka);
  GET_FRAGMENT_PARAM(Kd);
  GET_FRAGMENT_PARAM(Ks);
  GET_FRAGMENT_PARAM(shininess);
  
  /* Set light source color parameters once. */
  cgSetParameter3fv(myCgFragmentParam_globalAmbient, myGlobalAmbient);
  cgSetParameter3fv(myCgFragmentParam_lightColor, myLightColor);

  myCgFragmentParam_decal =
    cgGetNamedParameter(myCgFragmentProgram, "decal");
  checkForCgError("getting decal parameter");

  cgGLSetTextureParameter(myCgFragmentParam_decal, 666);
  checkForCgError("setting decal 2D texture");

  glutCreateMenu(menu);
  glutAddMenuEntry("[ ] Animate", ' ');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  glutMainLoop();
  return 0;
}

/* Forward declared routine used by reshape callback. */
static void buildPerspectiveMatrix(double fieldOfView,
                                   double aspectRatio,
                                   double zMin, double zMax,
                                   float m[16]);

static void reshape(int width, int height)
{
  double aspectRatio = (float) width / (float) height;
  double fieldOfView = 40.0; /* Degrees */

  /* Build projection matrix once. */
  buildPerspectiveMatrix(fieldOfView, aspectRatio,
                         1.0, 20.0,  /* Znear and Zfar */
                         myProjectionMatrix);
  glViewport(0, 0, width, height);
}

static void makeClipToTextureMatrix(float m[16])
{
  m[0]  = 0.5f;  m[1]  = 0;     m[2]  = 0;     m[3]  = 0.5f;
  m[4]  = 0;     m[5]  = 0.5f;  m[6]  = 0;     m[7]  = 0.5f;
  m[8]  = 0;     m[9]  = 0;     m[10] = 0.5f;  m[11] = 0.5f;
  m[12] = 0;     m[13] = 0;     m[14] = 0;     m[15] = 1;
}

static void multMatrix(float dst[16],
                       const float src1[16], const float src2[16])
{
  float tmp[16];
  int i, j;

  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      tmp[i*4+j] = src1[i*4+0] * src2[0*4+j] +
                   src1[i*4+1] * src2[1*4+j] +
                   src1[i*4+2] * src2[2*4+j] +
                   src1[i*4+3] * src2[3*4+j];
    }
  }
  /* Copy result to dst (so dst can also be src1 or src2). */
  for (i=0; i<16; i++)
    dst[i] = tmp[i];
}

static const double myPi = 3.14159265358979323846;

static void buildPerspectiveMatrix(double fieldOfView,
                                   double aspectRatio,
                                   double zNear, double zFar,
                                   float m[16])
{
  double sine, cotangent, deltaZ;
  double radians = fieldOfView / 2.0 * myPi / 180.0;
  
  deltaZ = zFar - zNear;
  sine = sin(radians);
  /* Should be non-zero to avoid division by zero. */
  assert(deltaZ);
  assert(sine);
  assert(aspectRatio);
  cotangent = cos(radians) / sine;
  
  m[0*4+0] = cotangent / aspectRatio;
  m[0*4+1] = 0.0;
  m[0*4+2] = 0.0;
  m[0*4+3] = 0.0;
  
  m[1*4+0] = 0.0;
  m[1*4+1] = cotangent;
  m[1*4+2] = 0.0;
  m[1*4+3] = 0.0;
  
  m[2*4+0] = 0.0;
  m[2*4+1] = 0.0;
  m[2*4+2] = -(zFar + zNear) / deltaZ;
  m[2*4+3] = -2 * zNear * zFar / deltaZ;
  
  m[3*4+0] = 0.0;
  m[3*4+1] = 0.0;
  m[3*4+2] = -1;
  m[3*4+3] = 0;
}

static void buildTextureMatrix(const float viewMatrix[16],
                               const float modelMatrix[16],
                               float textureMatrix[16])
{
  static float eyeToClipMatrix[16];
  float modelViewMatrix[16];
  static int needsInit = 1;

  if (needsInit) {
    const float fieldOfView = 50.0f;
    const float aspectRatio = 1;
    float textureProjectionMatrix[16];
    float clipToTextureMatrix[16];

    /* Build texture projection matrix once. */
    buildPerspectiveMatrix(fieldOfView, aspectRatio,
                           0.25, 20.0,  /* Znear and Zfar */
                           textureProjectionMatrix);

    makeClipToTextureMatrix(clipToTextureMatrix);

    /* eyeToClip = clipToTexture * textureProjection */
    multMatrix(eyeToClipMatrix,
      clipToTextureMatrix, textureProjectionMatrix);
    needsInit = 1;
  }

  /* modelView = view * model */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);
  /* texture = eyeToClip * modelView */
  multMatrix(textureMatrix, eyeToClipMatrix, modelViewMatrix);
}

static void buildLookAtMatrix(double eyex, double eyey, double eyez,
                              double centerx, double centery, double centerz,
                              double upx, double upy, double upz,
                              float m[16])
{
  double x[3], y[3], z[3], mag;

  /* Difference eye and center vectors to make Z vector. */
  z[0] = eyex - centerx;
  z[1] = eyey - centery;
  z[2] = eyez - centerz;
  /* Normalize Z. */
  mag = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);
  if (mag) {
    z[0] /= mag;
    z[1] /= mag;
    z[2] /= mag;
  }

  /* Up vector makes Y vector. */
  y[0] = upx;
  y[1] = upy;
  y[2] = upz;

  /* X vector = Y cross Z. */
  x[0] =  y[1]*z[2] - y[2]*z[1];
  x[1] = -y[0]*z[2] + y[2]*z[0];
  x[2] =  y[0]*z[1] - y[1]*z[0];

  /* Recompute Y = Z cross X. */
  y[0] =  z[1]*x[2] - z[2]*x[1];
  y[1] = -z[0]*x[2] + z[2]*x[0];
  y[2] =  z[0]*x[1] - z[1]*x[0];

  /* Normalize X. */
  mag = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  if (mag) {
    x[0] /= mag;
    x[1] /= mag;
    x[2] /= mag;
  }

  /* Normalize Y. */
  mag = sqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2]);
  if (mag) {
    y[0] /= mag;
    y[1] /= mag;
    y[2] /= mag;
  }

  /* Build resulting view matrix. */
  m[0*4+0] = x[0];  m[0*4+1] = x[1];
  m[0*4+2] = x[2];  m[0*4+3] = -x[0]*eyex + -x[1]*eyey + -x[2]*eyez;

  m[1*4+0] = y[0];  m[1*4+1] = y[1];
  m[1*4+2] = y[2];  m[1*4+3] = -y[0]*eyex + -y[1]*eyey + -y[2]*eyez;

  m[2*4+0] = z[0];  m[2*4+1] = z[1];
  m[2*4+2] = z[2];  m[2*4+3] = -z[0]*eyex + -z[1]*eyey + -z[2]*eyez;

  m[3*4+0] = 0.0;   m[3*4+1] = 0.0;  m[3*4+2] = 0.0;  m[3*4+3] = 1.0;
}

static void makeRotateMatrix(float angle,
                             float ax, float ay, float az,
                             float m[16])
{
  float radians, sine, cosine, ab, bc, ca, tx, ty, tz;
  float axis[3];
  float mag;

  axis[0] = ax;
  axis[1] = ay;
  axis[2] = az;
  mag = sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
  if (mag) {
    axis[0] /= mag;
    axis[1] /= mag;
    axis[2] /= mag;
  }

  radians = angle * myPi / 180.0;
  sine = sin(radians);
  cosine = cos(radians);
  ab = axis[0] * axis[1] * (1 - cosine);
  bc = axis[1] * axis[2] * (1 - cosine);
  ca = axis[2] * axis[0] * (1 - cosine);
  tx = axis[0] * axis[0];
  ty = axis[1] * axis[1];
  tz = axis[2] * axis[2];

  m[0]  = tx + cosine * (1 - tx);
  m[1]  = ab + axis[2] * sine;
  m[2]  = ca - axis[1] * sine;
  m[3]  = 0.0f;
  m[4]  = ab - axis[2] * sine;
  m[5]  = ty + cosine * (1 - ty);
  m[6]  = bc + axis[0] * sine;
  m[7]  = 0.0f;
  m[8]  = ca + axis[1] * sine;
  m[9]  = bc - axis[0] * sine;
  m[10] = tz + cosine * (1 - tz);
  m[11] = 0;
  m[12] = 0;
  m[13] = 0;
  m[14] = 0;
  m[15] = 1;
}

static void makeTranslateMatrix(float x, float y, float z, float m[16])
{
  m[0]  = 1;  m[1]  = 0;  m[2]  = 0;  m[3]  = x;
  m[4]  = 0;  m[5]  = 1;  m[6]  = 0;  m[7]  = y;
  m[8]  = 0;  m[9]  = 0;  m[10] = 1;  m[11] = z;
  m[12] = 0;  m[13] = 0;  m[14] = 0;  m[15] = 1;
}

/* Invert a row-major (C-style) 4x4 matrix. */
static void invertMatrix(float *out, const float *m)
{
/* Assumes matrices are ROW major. */
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(r)*4+(c)]

  double wtmp[4][8];
  double m0, m1, m2, m3, s;
  double *r0, *r1, *r2, *r3;

  r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

  r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
  r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
  r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

  r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
  r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
  r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

  r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
  r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
  r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

  r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
  r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
  r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

  /* Choose myPivot, or die. */
  if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
  if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
  if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
  if (0.0 == r0[0]) {
    assert(!"could not invert matrix");
  }

  /* Eliminate first variable. */
  m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
  s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
  s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
  s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
  s = r0[4];
  if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r0[5];
  if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r0[6];
  if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r0[7];
  if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* Choose myPivot, or die. */
  if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
  if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
  if (0.0 == r1[1]) {
    assert(!"could not invert matrix");
  }

  /* Eliminate second variable. */
  m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
  r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
  r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
  s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* Choose myPivot, or die. */
  if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
  if (0.0 == r2[2]) {
    assert(!"could not invert matrix");
  }

  /* Eliminate third variable. */
  m3 = r3[2]/r2[2];
  r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
  r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
  r3[7] -= m3 * r2[7];

  /* Last check. */
  if (0.0 == r3[3]) {
    assert(!"could not invert matrix");
  }

  s = 1.0/r3[3];              /* Now back substitute row 3. */
  r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

  m2 = r2[3];                 /* Now back substitute row 2. */
  s  = 1.0/r2[2];
  r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
  r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
  m1 = r1[3];
  r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
  r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
  m0 = r0[3];
  r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
  r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

  m1 = r1[2];                 /* Now back substitute row 1. */
  s  = 1.0/r1[1];
  r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
  r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
  m0 = r0[2];
  r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
  r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

  m0 = r0[1];                 /* Now back substitute row 0. */
  s  = 1.0/r0[0];
  r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
  r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

  MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
  MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
  MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
  MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
  MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
  MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
  MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
  MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7]; 

#undef MAT
#undef SWAP_ROWS
}

/* Simple 4x4 matrix by 4-component column vector multiply. */
static void transform(float dst[4],
                      const float mat[16], const float vec[4])
{
  double tmp[4], invW;
  int i;

  for (i=0; i<4; i++) {
    tmp[i] = mat[i*4+0] * vec[0] +
             mat[i*4+1] * vec[1] +
             mat[i*4+2] * vec[2] +
             mat[i*4+3] * vec[3];
  }
  invW = 1 / tmp[3];
  /* Apply perspective divide and copy to dst (so dst can vec). */
  for (i=0; i<3; i++)
    dst[i] = tmp[i] * tmp[3];
  dst[3] = 1;
}

static void display(void)
{
  
  const float eyePosition[4] = { eyex_f, eyey_f, eyez_f, 1 };
  const float lightPosition[4] = { 5 *sin(myLightAngle), 
                                   1,
                                   5 *cos(myLightAngle), 1 };

  float translateMatrix[16], rotateMatrix[16],
        modelMatrix[16], invModelMatrix[16], viewMatrix[16],
        modelViewMatrix[16], modelViewProjMatrix[16],lightViewMatrix[16],textureMatrix[16];
  float objSpaceEyePosition[4], objSpaceLightPosition[4];

  buildLookAtMatrix(eyePosition[0], eyePosition[1], eyePosition[2],
	  lookatx_f, lookaty_f, lookatz_f,
	  viewx_f, viewy_f, viewz_f,
                    viewMatrix);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  cgGLBindProgram(myCgVertexProgram);
  checkForCgError("binding vertex program");

  cgGLEnableProfile(myCgVertexProfile);
  checkForCgError("enabling vertex profile");

  cgGLBindProgram(myCgFragmentProgram);
  checkForCgError("binding fragment program");

  cgGLEnableProfile(myCgFragmentProfile);
  checkForCgError("enabling fragment profile");
  
  cgGLEnableTextureParameter(myCgFragmentParam_decal);
  checkForCgError("enable decal texture");

  glEnable(GL_COLOR_MATERIAL);

  /* modelView = rotateMatrix * translateMatrix */
  makeRotateMatrix(0, 0, 0, 0, rotateMatrix);
  makeTranslateMatrix(0, 0, 0, translateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);
  
  //Code to draw the obj file
	GLMmodel *objmodel = NULL;
	int i=0,j;  
	objmodel = glmReadOBJ("dolphins.obj");
	glmUnitize(objmodel);
	glmFacetNormals(objmodel);
	glmVertexNormals(objmodel,90);
	printf("triangles %d\n",objmodel->numtriangles);
	
	GLMgroup *group;
	GLMtriangle *triangle;

	group = objmodel->groups;
	int objects;

	while(group){			
		objects = group->numtriangles;
		GLMmaterial *material;					
		material = &objmodel->materials[group->material];
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material->ambient);
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material->diffuse);
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material->specular);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material->shininess);
		
		cgSetParameter3fv(myCgFragmentParam_Ka, material->ambient);
		cgSetParameter3fv(myCgFragmentParam_Kd, material->diffuse);
		cgSetParameter3fv(myCgFragmentParam_Ks, material->specular);
		cgSetParameter1f(myCgFragmentParam_shininess, material->shininess);
		
		cgUpdateProgramParameters(myCgVertexProgram);
		cgUpdateProgramParameters(myCgFragmentProgram);
  	
		for(i=0;i<objects;i++){
			glBegin(GL_TRIANGLES);

			int tri = group->triangles[i];
			int index = objmodel->triangles[tri].vindices[0];
			float x = objmodel->vertices[3*index];
			float y = objmodel->vertices[3*index+1];
			float z = objmodel->vertices[3*index+2];
		
			int normalIndex = objmodel->triangles[tri].nindices[0];
			float nx = objmodel->normals[3*normalIndex];
			float ny = objmodel->normals[3*normalIndex+1];
			float nz = objmodel->normals[3*normalIndex+2];
			
			glTexCoord2f(0, 0);
			glNormal3f(nx,ny,nz);
			glVertex3f(x,y,z);

			index = objmodel->triangles[tri].vindices[1];
			x = objmodel->vertices[3*index];
			y = objmodel->vertices[3*index+1];
			z = objmodel->vertices[3*index+2];

			normalIndex = objmodel->triangles[tri].nindices[1];
			nx = objmodel->normals[3*normalIndex];
			ny = objmodel->normals[3*normalIndex+1];
			nz = objmodel->normals[3*normalIndex+2];

			glTexCoord2f(1, 0);
			glNormal3f(nx,ny,nz);
			glVertex3f(x,y,z);

			index = objmodel->triangles[tri].vindices[2];
			x = objmodel->vertices[3*index];
			y = objmodel->vertices[3*index+1];
			z = objmodel->vertices[3*index+2];
			normalIndex = objmodel->triangles[tri].nindices[2];
			nx = objmodel->normals[3*normalIndex];
			ny = objmodel->normals[3*normalIndex+1];
			nz = objmodel->normals[3*normalIndex+2];
	
			glTexCoord2f(1, 1);
			glNormal3f(nx,ny,nz);
			glVertex3f(x,y,z);
			glEnd();
		}

		group=group->next;
	}

  cgGLDisableProfile(myCgVertexProfile);
  checkForCgError("disabling vertex profile");

  cgGLDisableProfile(myCgFragmentProfile);
  checkForCgError("disabling fragment profile");

  cgGLDisableTextureParameter(myCgFragmentParam_decal);
  checkForCgError("disabling decal texture");

  glutSwapBuffers();
}

static void idle(void)
{
  myLightAngle += 0.008;  /* Add a small angle (in radians). */
  if (myLightAngle > 2*myPi) {
    myLightAngle -= 2*myPi;
  }
  glutPostRedisplay();
}

static void keyboard(unsigned char c, int x, int y)
{
  static int animating = 0;

  switch (c) {
  case ' ':
    animating = !animating; /* Toggle */
    if (animating) {
      glutIdleFunc(idle);
    } else {
      glutIdleFunc(NULL);
    }    
    break;
  case 27:  /* Esc key */
    /* Demonstrate proper deallocation of Cg runtime data structures.
       Not strictly necessary if we are simply going to exit. */
    cgDestroyProgram(myCgVertexProgram);
    cgDestroyContext(myCgContext);
    exit(0);
    break;
  }
}

static void menu(int item)
{
  /* Pass menu item character code to keyboard callback. */
  keyboard((unsigned char)item, 0, 0);
}

/* Platform-specific code to request synchronized buffer swaps. */

static void requestSynchronizedSwapBuffers(void)
{
#if defined(__APPLE__)
#ifdef CGL_VERSION_1_2
  const GLint sync = 1;
#else
  const long sync = 1;
#endif
  CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);

#elif defined(_WIN32)
  if (wglSwapIntervalEXT) {
    wglSwapIntervalEXT(1);
  }
#else
  if (glXSwapIntervalSGI) {
    glXSwapIntervalSGI(1);
  }
#endif
}

static void setupDemonSampler(void)
{
  GLuint texobj = 666;
  CGparameter sampler;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); /* Tightly packed texture data. */

  glBindTexture(GL_TEXTURE_2D, texobj);
  /* Load demon decal texture with mipmaps. */
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8,
    128, 128, GL_RGB, GL_UNSIGNED_BYTE, myDemonTextureImage);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR);

  /* Assumes OpenGL 1.3 or ARB_texture_border_clamp */
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER                0x812D
#endif

  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Kd);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  sampler = cgGetNamedParameter(myCgFragmentProgram, "projectiveMap");
  checkForCgError("getting projectiveMap sampler parameter");
  cgGLSetTextureParameter(sampler, texobj);
}

