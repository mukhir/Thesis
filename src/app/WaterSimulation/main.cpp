#include "WaterScene.h"
#include "glew/glew.h"
#include "glut/glut.h"

WaterScene* water;

void display()
{
	water->update();
	water->renderScene();
}

void reshape(int width, int height)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(height == 0)
		height = 1;
	float ratio = 1.0* width / height;

	water->m_windowHeight = height;
	water->m_windowWidth = width;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

        // Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, width, height);

	// Set the correct perspective.
	gluPerspective(45,ratio,1,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}



void pressNormalKeys(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	else
		water->pressNormalKey(key);
}

void releaseNormalKeys(unsigned char key, int x, int y)
{
	water->releaseNormalKey(key);
}

void pressKey(int key, int x, int y)
{
	water->pressKey(key);
}

void releaseKey(int key, int x, int y)
{
	water->releaseKey(key);
}

void processMousePassiveMotion(int x, int y)
{
	water->mouseMoved(x,y);
}

void mouseButton(int button, int state, int x, int y)
{
	water->mouseButton(button, state, x, y);
}

void mouseMove(int x, int y)
{
	//water->mouseMove(x, y);
}

void InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

int main(int argc, char**argv)
{
	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1360,768);
	glutCreateWindow("Water Simulation");

	// register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutKeyboardFunc(pressNormalKeys);
	glutSpecialFunc(pressKey);

	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(releaseKey);
	glutKeyboardUpFunc(releaseNormalKeys);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutPassiveMotionFunc(processMousePassiveMotion);

	glewInit();	

	InitGL();

	water = new WaterScene();

	// enter GLUT event processing cycle
	glutMainLoop();

	return 0;
}



/*bool yes = false;
int rotation = 0;

void mouseButton(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON) {
		if (yes == true)
			yes = false;
		else
			yes = true;
		
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_QUADS);                      // Drawing Using Triangles
		glVertex2f(-1.0f, 1.0f); // vertex 1
        glVertex2f(1.0f, 1.0f); // vertex 2
		glVertex2f(1.0f, -1.0f); // vertex 3
		glVertex2f(-1.0f, -1.0f); // vertex 4
	glEnd(); 

	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);                      // Drawing Using Triangles
		glVertex2f(0.55f, 0.15f); // vertex 1
        glVertex2f(0.56f, 0.16f); // vertex 2
		glVertex2f(0.70f, -0.15f); // vertex 3
		glVertex2f(0.69f, -0.16f); // vertex 4
	glEnd(); 

	glPushMatrix();

	if(yes)
		rotation = rotation + 1;
	
	glRotatef(rotation, 0.0f, 0.0f, 1.0f);

	glBegin(GL_QUADS);                      // Drawing Using Triangles
		glVertex2f(-0.005f, 0.15f); // vertex 1
        glVertex2f(0.005f, 0.15f); // vertex 2
		glVertex2f(0.005f, -0.15f); // vertex 3
		glVertex2f(-0.005f, -0.15f); // vertex 4
	glEnd(); 

	glPopMatrix();

    glutSwapBuffers();
}

int main(int argc, char**argv)
{

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800,600);
    glutCreateWindow("Hello World");

    glutMouseFunc(mouseButton);
    glutDisplayFunc(display);
	glutIdleFunc(display);

    glutMainLoop();

	return 0;
}*/
