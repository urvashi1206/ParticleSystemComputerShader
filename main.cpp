#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "Text.h"
#include "ParticleSystem.h"

#include <iostream>
using namespace std;
using namespace glm;

int g_winWidth = 1024;
int g_winHeight = 512;
GLfloat radius = 2.0f;
int width = 64;
int height = 32;	

GLfloat originPosition[] = { 0.0f, 5.0f, 20.0f, 1.0f };
GLfloat wireframePosition[] = { 0.0f, 5.0f, -8.0f, 0.0f };
vec3 min_point = vec3(-10.0f, 0.0f, -5.0f);
vec3 max_point = vec3(10.0f, 10.0f, -5.0f);

Camera g_cam;
Text g_text;

unsigned char g_keyStates[256];

unsigned int curTime = 0; //the milliseconds since the start
unsigned int preTime = 0;
ParticleSystem parSys;

char v_shader_file[] = "..\\..\\shaders\\v_shader.vert";
char f_shader_file[] = "..\\..\\shaders\\f_shader.frag";
char c_shader_file[] = "..\\..\\shaders\\c_shader.comp";

void initialization()
{
	//std::cout<<"Width"<<width<<" and Height"<<height<<std::endl;
	parSys.create(width, height, min_point, max_point, c_shader_file, v_shader_file, f_shader_file);

	g_cam.set(38.0f, 13.0f, 4.0f, 0.0f, 0.0f, 0.0f, g_winWidth, g_winHeight, 45.0f, 0.01f, 10000.0f);
	g_text.setColor(0.0f, 0.0f, 0.0f);

	// add any stuff you want to initialize ...
}

/****** GL callbacks ******/
void initialGL()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
	// add any stuff to update at runtime ....
	curTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaT = (float)(curTime - preTime) / 1000.0f; // in seconds
	vec3 originPos = vec3(originPosition[0], originPosition[1], originPosition[2]);
	vec3 wireFramePos = vec3(wireframePosition[0], wireframePosition[1], wireframePosition[2]);
	float r = radius;
	parSys.update(deltaT, originPos, r, wireFramePos);

	g_cam.keyOperation(g_keyStates, g_winWidth, g_winHeight);

	glutPostRedisplay();

	preTime = curTime;
}

void display()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
		
	glColor3f(1.0f, 0.0f, 0.0f);
	
	mat4 mat1 = translate(mat4(1.0f), vec3(originPosition[0], originPosition[1], originPosition[2]));
	mat4 modelmat1 = scale(mat1, vec3(1.0f, 1.0f, 1.0f));
	mat4 viewMatrix1 = g_cam.viewMat * modelmat1;
	
	glLoadMatrixf(value_ptr(viewMatrix1));
	glutSolidSphere(0.3, 50, 50);


	glMatrixMode(GL_MODELVIEW);

	glColor3f(1.0f, 0.0f, 0.0f);

	mat4 mat2 = translate(mat4(1.0f), vec3(0,0,0));
	mat4 modelmat2 = scale(mat2, vec3(1.0f, 1.0f, 1.0f));
	mat4 viewMatrix2 = g_cam.viewMat * modelmat2;

	glLoadMatrixf(value_ptr(viewMatrix2));
	vec3 corner1 = vec3(min_point.x, min_point.y, min_point.z);
	vec3 corner2 = vec3(max_point.x, min_point.y, min_point.z);
	vec3 corner3 = vec3(min_point.x, max_point.y, min_point.z);
	vec3 corner4 = vec3(max_point.x, max_point.y, min_point.z);

	glBegin(GL_LINES);
	// Line to corner 1
		glVertex3f(originPosition[0], originPosition[1], originPosition[2]);
		glVertex3f(corner1.x, corner1.y, corner1.z);

		glVertex3f(originPosition[0], originPosition[1], originPosition[2]);
		glVertex3f(corner2.x, corner2.y, corner2.z);

		glVertex3f(originPosition[0], originPosition[1], originPosition[2]);
		glVertex3f(corner3.x, corner3.y, corner3.z);

		glVertex3f(originPosition[0], originPosition[1], originPosition[2]);
		glVertex3f(corner4.x, corner4.y, corner4.z);
	glEnd();

	glMatrixMode(GL_MODELVIEW);

	glColor3f(0.0f, 1.0f, 0.0f);

	mat4 mat3 = translate(mat4(1.0f), vec3(wireframePosition[0], wireframePosition[1], wireframePosition[2]));
	mat4 modelmat3 = scale(mat3, vec3(1.0f, 1.0f, 1.0f));
	mat4 viewMatrix3 = g_cam.viewMat * modelmat3;

	glLoadMatrixf(value_ptr(viewMatrix3));
	glutWireSphere(radius, 30, 30);

	parSys.draw(1.0f, g_cam.viewMat, g_cam.projMat);

	g_cam.drawGrid();
	g_cam.drawCoordinateOnScreen(g_winWidth, g_winHeight);
	g_cam.drawCoordinate();

	// display the text
	if (g_cam.isFocusMode()) {
		string str = "Cam mode: Focus";
		g_text.draw(10, 30, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	}
	else if (g_cam.isFPMode()) {
		string str = "Cam mode: FP";
		g_text.draw(10, 30, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	}

	// display the Resolution
	string str = "Resolution: " + std::to_string(width) + "x" + std::to_string(height);
	g_text.draw(10, 50, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	// display the number of particles
	str = "# of Particles ((w+1)*(h+1)): " + std::to_string((width+1) * (height+1));
	g_text.draw(10, 70, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	glPopMatrix();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_winWidth = w;
	g_winHeight = h;
	if (h == 0) {
		h = 1;
	}
	g_cam.setProjectionMatrix(g_winWidth, g_winHeight);
	g_cam.setViewMatrix(); 
	glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y)
{
	g_cam.mouseClick(button, state, x, y, g_winWidth, g_winHeight);
}

void motion(int x, int y)
{
	g_cam.mouseMotion(x, y, g_winWidth, g_winHeight);
}

void keyup(unsigned char key, int x, int y)
{
	g_keyStates[key] = false;
}

void keyboard(unsigned char key, int x, int y)
{
	g_keyStates[key] = true;
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'c': // switch cam control mode
		g_cam.switchCamMode();
		glutPostRedisplay();
		break;
	case 'w': //light move
		wireframePosition[2] -= 0.1;
		break;
	case 's':
		wireframePosition[2] += 0.1;
		break;
	case 'd':
		wireframePosition[0] += 0.1;
		break;
	case 'a':
		wireframePosition[0] -= 0.1;
		break;
	case 'u':
		wireframePosition[1] += 0.1;
		break;
	case 'j': 
		wireframePosition[1] -= 0.1;
		break;
	case '+':
		if (width < 8192 && height < 4096)
		{
			width *= 2;
			height *= 2;
			//std::cout << "Width" << width << " and Height" << height << std::endl;
			parSys.create(width, height, min_point, max_point, c_shader_file, v_shader_file, f_shader_file);
		}
		break;	
	case '-':
		if (width > 16 && height > 8)
		{
			width /= 2;
			height /= 2;
			//std::cout << "Width" << width << " and Height" << height << std::endl;
			parSys.create(width, height, min_point, max_point, c_shader_file, v_shader_file, f_shader_file);
		}
		break;
	case ' ':
		g_cam.PrintProperty();
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(g_winWidth, g_winHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Compute Shader Example: A Simple particle System");

	glewInit();
	initialGL();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardUpFunc(keyup);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	initialization();

	glutMainLoop();
	return EXIT_SUCCESS;
}