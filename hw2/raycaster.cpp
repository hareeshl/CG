#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glut.h>
#include "glm.h"
#define mapWidth 4
#define mapHeight 4

int map[mapHeight][mapWidth]=
{
	{1,1,1,1},
	{0,0,0,0},
	{0,0,0,0},
	{0,0,0,0}
};

void drawVertices(int x,int drawStart,int end){
	int temp=0;

	//Draw the vertices
	glColor3f(1,1,1);
	glBegin(GL_POINTS);
	for(temp=0;temp<end;temp++,drawStart++){
		glVertex2i(x,drawStart);				
	}
	glEnd();
}

void init(void) 
{
   glClearColor (0.0, 0.0, 0.0, 0.0);   
   glDisable(GL_DEPTH_TEST);   
}

bool checkMap(int x,int y){
	if(map[y][x] ==0) return false;
	else return true;
}

void display(void)
{	
	int w=320,cnt=0;
	double angle = 45;
	double viewangle = 60.0,betaangle=0;
	int px=115,py=191,cx,cy;
	int initpx=115,initpy=191;
	int ya=-64,xa,xb=64,yb;
	int hit=0;
	double xdist=0;
	int rounddist = 0;
	int projMiddle = 100;
	int drawStart = 0;
	double correctdist=0;
	
	//Iterate for each vertical stripe
	for(cnt=0;cnt<w;cnt++){		
		hit=0;
		px=115,py=191;
		xa = 64/tan(60 * 3.14/180);
		px = px + xa;
		py = py + ya;
		
		//*****Find horizontal intersection*****
		while(hit==0){
			cx = px/64;
			cy= py/64;
			//If wall in map at cx,cy
			if(checkMap(cx,cy)){ 
				hit=1;
				//Calculate the distance
				xdist = abs(initpx-cx)/cos(angle * 3.14/180);								
				//betaangle = angle - viewangle;
				//correctdist = xdist * cos(betaangle);
				rounddist = (64*277)/xdist;
				drawStart = projMiddle - (rounddist/2);	
				angle = angle + (double)(60.0/320.0);
			}else{
				px = px+xa;
				py = py+ya;
			}
		}
		//**************************************************

		hit=0;
		px=115,py=191;
		yb = 64/tan(60 * 3.14/180);
		px = px + xb;
		py = py + yb;

	}

	glutSwapBuffers();
   
}

void reshape (int w, int h)
{
   glViewport (0, 0,  w,  h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0,320,200,0);
   glMatrixMode (GL_MODELVIEW);
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize (320, 200); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("window");
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);  
   glutMainLoop();
   return 0;
}

