#include <gl/glut.h>

float _angle = 0.0f;

void Display(void){
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,(double)500/(double)500,0.1,10.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	glTranslatef(0.0f,0.0f,-6.0f);
	glRotatef(_angle,0.0f,1.0f,0.0f);

	glBegin(GL_TRIANGLES);
		//Red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(0.0f,1.0f,0.0f);	
		//Green
		glColor3f(0.0,1.0,0.0);
		glVertex3f(-1.0f,-1.0f,0.0f);	
		//Blue
		glColor3f(0.0,0.0,1.0);
		glVertex3f(1.0f,-1.0f,0.0f);			
	glEnd();

	glutSwapBuffers();
}

void init(void){
	glEnable(GL_DEPTH_TEST);	
}

void Reshape(int w,int h){
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,(double)w/(double)h,1.0,900.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
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

void drawChars(){
	float xptr = 0.0f,yptr=0.0f;
	int cnt =0;
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.5,1.5,1.0,-1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();				
	glPointSize(3.0);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_POINTS);	
	xptr = -1.0f;
	yptr = -1.0f;
	for(cnt=0;cnt<100;cnt++,yptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}
	xptr = -1.0f;
	yptr = 0.0f;
	for(cnt=0;cnt<100;cnt++,xptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}		
	xptr = 1.0f;
	yptr = -1.0f;
	for(cnt=0;cnt<100;cnt++,yptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}		
	glEnd();

	glutSwapBuffers();
}

void draw3dchars(){
	float xptr = 0.0f,yptr=0.0f;
	int cnt =0;
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,(double)500/(double)500,0.1,10.0);	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-3,2,-2,0,0,0,0,1,0);
	glPointSize(3.0);
	glColor3f(0.0,1.0,1.0);
	glBegin(GL_POINTS);	
	xptr = -1.0f;
	yptr = -1.0f;
	for(cnt=0;cnt<100;cnt++,yptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}
	xptr = -1.0f;
	yptr = 0.0f;
	for(cnt=0;cnt<100;cnt++,xptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}		
	xptr = 1.0f;
	yptr = -1.0f;
	for(cnt=0;cnt<100;cnt++,yptr+=0.02){
		glVertex3f(xptr,yptr,0.0f);				
	}		
	glEnd();
	glutSwapBuffers();
}

void keyMap(unsigned char key, int x,int y){
	
	switch(key){
	
		//Code to display a alphabet with pixels
		case '1':
			drawChars();
		break;

		//Code to display a alphabet with pixels
		case '2':
			draw3dchars();
		break;

		//Code to display the triangle
		case '3':
			glutPostRedisplay();
		break;

		//Code to display a alphabet with pixels
		case ' ':
			draw3dchars();
		break;
	}

}

int main(int argc, char** argv){
	
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
	glutInitWindowSize(500,500);
	glutInitWindowPosition(200,200);
	glutCreateWindow("Hw1");
	init();
	glutDisplayFunc(Display);	
	glutReshapeFunc(Reshape);
	glutSpecialFunc(specialMap);
	glutKeyboardFunc(keyMap);
	glutMainLoop();
	
	return 0;
} 