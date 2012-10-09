#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glut.h>
#include "glm.h"
#include "vector3d.h"

#define SMALL_NUM 0.00000001

float _angle = 0.0;
GLMmodel *objmodel;
static float ypoz = 0;

void init(void) 
{	
    glEnable(GL_DEPTH_TEST);  
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	//glEnable(GL_NORMALIZE);
}

void drawmodel(){
	int i=0,j;
	GLfloat lightpos[] = {0,5,0,1};
    GLfloat specular[] = {1,1,1,1};
	GLfloat ambient[] = {1,1,1,1};
	GLfloat diffuse[] = {1,1,1,1};

	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glLightfv(GL_LIGHT0,GL_POSITION,lightpos);

	objmodel = glmReadOBJ("box.obj");
	glmUnitize(objmodel);
	glmFacetNormals(objmodel);
	glmVertexNormals(objmodel,180);
	printf("triangles %d\n",objmodel->numtriangles);
	
	GLMgroup *group;
	GLMtriangle *triangle;

	/*glBegin(GL_TRIANGLES);
	for(i=0;i<objmodel->numtriangles;i++){
		
		int index = objmodel->triangles[i].vindices[0];
		float x = objmodel->vertices[3*index];
		float y = objmodel->vertices[3*index+1];
		float z = objmodel->vertices[3*index+2];
		
		int normalIndex = objmodel->triangles[i].nindices[0];
		float nx = objmodel->normals[3*normalIndex];
		float ny = objmodel->normals[3*normalIndex+1];
		float nz = objmodel->normals[3*normalIndex+2];

		glNormal3f(nx,ny,nz);
		glVertex3f(x,y,z);

		index = objmodel->triangles[i].vindices[1];
		x = objmodel->vertices[3*index];
		y = objmodel->vertices[3*index+1];
		z = objmodel->vertices[3*index+2];

		normalIndex = objmodel->triangles[i].nindices[1];
		nx = objmodel->normals[3*normalIndex];
		ny = objmodel->normals[3*normalIndex+1];
		nz = objmodel->normals[3*normalIndex+2];

		glNormal3f(nx,ny,nz);
		glVertex3f(x,y,z);

		index = objmodel->triangles[i].vindices[2];
		x = objmodel->vertices[3*index];
		y = objmodel->vertices[3*index+1];
		z = objmodel->vertices[3*index+2];
		normalIndex = objmodel->triangles[i].nindices[2];
		nx = objmodel->normals[3*normalIndex];
		ny = objmodel->normals[3*normalIndex+1];
		nz = objmodel->normals[3*normalIndex+2];

		glNormal3f(nx,ny,nz);
		glVertex3f(x,y,z);
		
	}
	glEnd();*/
	glmDraw(objmodel,GLM_SMOOTH);
}

void display(void)
{	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glClearColor (0.0, 0.0, 0.0, 0.0);
	GLMgroup *group;
	GLMtriangle *triangle;

	int w =250,h=250,d=1,objects,k=0;
	int i,j;
	int fov = 60;
	float dx,dy;
	float px,py;

	//Eye vector
	Vector3D eye(0,0,-2);
	//Lookat vector
	Vector3D lookat(0,0,1);
	Vector3D look(lookat.x-eye.x,lookat.y-eye.y,lookat.z-eye.z);
	look.normalize();

	//Up vector
	Vector3D up (0,1,0);

	//Right 
	Vector3D du = cross(look,up);
	du.normalize();

	Vector3D dv = cross(look,du);
	dv.normalize();

	float f1 = (float)(w/ (2 * tan((0.5 * fov)*3.14/180)));

	look.normalize();

	Vector3D vp;
	vp.x = look.x;
	vp.y = look.y;
	vp.z = look.z;

	vp.x = vp.x * f1 - 0.5 * (w * du.x + h * dv.x);
	vp.y = vp.y * f1 - 0.5 * (w * du.y + h * dv.y);
	vp.z = vp.z * f1 - 0.5 * (w * du.z + h * dv.z);

	//Get the scene
	drawmodel();
	objects = objmodel->numtriangles;

	glColor3f(1,1,1);
	glBegin(GL_POINTS);

	for(k=0;k<objects;k++){

		for(i=0;i<w;i++){
			for(j=0;j<h;j++){

				Vector3D dir(i*du.x+j*dv.x+vp.x,
						i*du.y+j*dv.y+vp.y,
						i*du.z+j*dv.z+vp.z);
				dir.normalize();
				//printf("%Lf,%Lf,%Lf\n",dir.x,dir.y,dir.z);
			
				//Find intersection
				int index = objmodel->triangles[k].vindices[0];
				float x = objmodel->vertices[3*index];
				float y = objmodel->vertices[3*index+1];
				float z = objmodel->vertices[3*index+2];
				Vector3D p0(x,y,z);
				
				index = objmodel->triangles[k].vindices[1];
				x = objmodel->vertices[3*index];
				y = objmodel->vertices[3*index+1];
				z = objmodel->vertices[3*index+2];
				Vector3D p1(x,y,z);

				index = objmodel->triangles[k].vindices[2];
				x = objmodel->vertices[3*index];
				y = objmodel->vertices[3*index+1];
				z = objmodel->vertices[3*index+2];
				Vector3D p2(x,y,z);

				Vector3D e1,e2,q,s,r;
				float a,b,f,u,v,t;

				e1 = p1 - p0;
				e2 = p2 - p0;
				q=cross(dir,e2);
				a = dot(e1,q);
				
				if(a >-SMALL_NUM && a < SMALL_NUM){
					//printf("Vector parallel to plane\n");
					continue;
				}

				f = 1/a;
				s = eye-p0;
				u = f * dot(s,q);

				if(u<0.0){
					//printf("Intersection outside triangle\n");
					continue;
				}
				
				r = cross(s,e1);
				v = f * dot(dir,r);

				if(v <0.0 || u+v > 1.0){
					//printf("Intersection outside the triangle\n");
					continue;
				}

				t = f * dot(e2,r);
				glVertex2d(i,j);
			}
		}
	}
	glEnd();
	printf("end");
	glLoadIdentity();
	/*gluLookAt(0,0,-3,0,0,1,0,1,0);
	
	glPushMatrix();
	glRotatef(_angle,1.0f,0.0f,0.0f);
	drawmodel();
	glPopMatrix();
	*/
	glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0,  w,  h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0,250,250,0,0,1);
   //gluPerspective(60,w/h,1,10);
   glMatrixMode (GL_MODELVIEW);
}

void animate()
{
	ypoz+=0.5;
	if (ypoz>360) ypoz=0;
	glutPostRedisplay();
}

void specialMap(int key,int x,int y){

	switch (key){

	case GLUT_KEY_LEFT:
		_angle += 2.0f;
		if(_angle > 360)
			_angle -= 360;
		glutPostRedisplay();
	break;

	case GLUT_KEY_RIGHT:
		_angle -= 2.0f;
		if(_angle < 0)
			_angle = 360 - _angle;	
		glutPostRedisplay();
	break;

	}
}


int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (250, 250); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("window");
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape); 
   glutSpecialFunc(specialMap);
   //glutIdleFunc(animate);
   glutMainLoop();
   return 0;
}

