#include "OpenGL.h"

#include "GL/glut.h"
#include <cmath>
#include <map>
#include <cstdio>
#include "SOIL/SOIL.h"

using namespace std;

void(*drawFunc)();
void(*mouseFunc)(float x, float y);
void(*keyboardFunc)(int key);

int matrixLvl = 0;
float rotation = 0;

int width = 640;
int height = 480;

bool* keyHandle[256] = { 0 };

void AttachKeyHandle(char key, bool* value) {
	keyHandle[key] = value;
}


void KeyUp(unsigned char key, int x, int y) {
	if (keyHandle[key])
		*keyHandle[key] = false;
}

void KeyDown(unsigned char key, int x, int y) {
	if (keyHandle[key])
		*keyHandle[key] = true;
}

GLuint loadTexture(string name) {
	GLuint tex;
	tex = SOIL_load_OGL_texture(
		name.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA
	);
	if (tex == 0) {
		printf("Error loading texture %s", name.c_str());
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	return tex;
}

GLuint getTexture(string name) {
	static map<string, GLuint> loadedTextures;
	if (loadedTextures.find(name) == loadedTextures.end()) {
		loadedTextures[name] = loadTexture(name);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	return loadedTextures[name];
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	matrixLvl = 1;
	drawFunc();
	Reset();
	glutSwapBuffers();
}

void Rotate(float degrees) {
	rotation += degrees;
}

void Reset() {
	while (matrixLvl > 0) {
		matrixLvl--;
		glPopMatrix();
	}
	rotation = 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	height = h;
	width = w;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-.3, 1.3, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

void SetColor(int r, int g, int b, int a) {
	glColor4ub(r, g, b, a);
}

void idle() {
	glutPostRedisplay();
}

void MotionFunc(int x, int y) {
	if(mouseFunc)
		mouseFunc(2.0 * x / width - 1, -2.0 * y / height + 1);
}

void KeyboardFunc(unsigned char key, int x, int y) {
	KeyDown(key, x, y);
	if(keyboardFunc)
		keyboardFunc(key);
}

void RunOpenGL(int &argc, char** argv, void (Draw)(), void (Mouse)(float x, float y), void (Keyboard)(int key)) {
	drawFunc = Draw;
	mouseFunc = Mouse;
	keyboardFunc = Keyboard;
	
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("OpenGL playground");
	glEnable(GL_TEXTURE_2D);
	glutDisplayFunc(Display);
	glutKeyboardFunc(KeyboardFunc);
	glutKeyboardUpFunc(KeyUp);
	glutMotionFunc(MotionFunc);
	glutPassiveMotionFunc(MotionFunc);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glClearColor(0, 0, 0, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}

void DrawRectangle(float x1, float y1, float x2, float y2) {
	float cx = (x1 + x2) / 2.0f;
	float cy = (y1 + y2) / 2.0f;
	x1 -= cx;
	x2 -= cx;
	y1 -= cy;
	y2 -= cy;
	glPushMatrix();
	glTranslatef(cx, cy, 0);
	glRotatef(rotation, 0, 0, 1);
	glBegin(GL_QUADS);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
	glPopMatrix();
}

void DrawLetter(string fontImg, char letter, float centerX, float centerY, float scale = 1.0) {
	int index = -1;
	if (letter >= 'a' && letter <= 'z') index = 10 + letter - 'a';
	if (letter >= 'A' && letter <= 'Z') index = 10 + letter - 'A';
	if (letter >= '0' && letter <= '9') index = letter - '0';
	if (index == -1) return;

	GLuint textId = getTexture(fontImg);
	glBindTexture(GL_TEXTURE_2D, textId);
	glPushMatrix();
	glBegin(GL_QUADS);
	float off = -.0045;
	float width = .0281;
	glTexCoord2f(off + width * index,       0);  glVertex2f(centerX-.03*scale, -.036*scale);
	glTexCoord2f(off + width * (index + .9), 0); glVertex2f(centerX+.03*scale, -.036*scale);
	glTexCoord2f(off + width * (index + .9), 1); glVertex2f(centerX+.03*scale,  .036*scale);
	glTexCoord2f(off + width * index,       1);  glVertex2f(centerX -.03*scale,  .036*scale);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
}

void PrintText(string font, string text, float centerX, float centerY, float scale) {
	glPushMatrix();
	float spaceing = .06;
	float offset = scale * spaceing * (text.length()-1) / 2.0f;
	glTranslatef(centerX, centerY, 0);
	glRotatef(rotation, 0, 0, 1);
	for (int i = 0; i < text.length(); i++) {
		DrawLetter(font, text.at(i), - offset + spaceing * i * scale, 0, scale);
	}
	glPopMatrix();
}

void DrawRectangle(
		float x1, float y1,
		float x2, float y2,
		string texture, 
		bool useColor, 
		float tx1, float ty1, 
		float tx2, float ty2) 
{
	GLuint textId = getTexture(texture);
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	if(!useColor)
		glColor3ub(255, 255, 255);
	glBindTexture(GL_TEXTURE_2D, textId);

	float cx = (x1 + x2) / 2.0f;
	float cy = (y1 + y2) / 2.0f;
	x1 -= cx;
	x2 -= cx;
	y1 -= cy;
	y2 -= cy;
	glPushMatrix();
	glTranslatef(cx, cy, 0);
	glRotatef(rotation, 0, 0, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(tx1, ty1); glVertex2f(x1, y1);
	glTexCoord2f(tx2, ty1); glVertex2f(x2, y1);
	glTexCoord2f(tx2, ty2); glVertex2f(x2, y2);
	glTexCoord2f(tx1, ty2); glVertex2f(x1, y2);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	glPopAttrib();
}

void DrawTriangle(float x1, float y1,
				  float x2, float y2,
				  float x3, float y3) {

	float cx = (x1 + x2 + x3) / 3.0f;
	float cy = (y1 + y2 + y3) / 3.0f;
	x1 -= cx;
	y1 -= cy;
	x2 -= cx;
	y2 -= cy;
	x3 -= cx;
	y3 -= cy;
	glPushMatrix();
	glTranslatef(cx, cy, 0);
	glRotatef(rotation, 0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glVertex2f(x3, y3);
	glEnd();
	glPopMatrix();
}

void DrawCircle(float centerX, float centerY, float radius, int segments) {
	glPushMatrix();
	glTranslatef(centerX, centerY, 0);
	glRotatef(rotation, 0, 0, 1);
	glBegin(GL_POLYGON);
	for (int i = 0; i < segments; i++) {
		glVertex2f(cos(i * 3.14159 * 2 / segments)*radius, sin(i * 3.14159 * 2 / segments)*radius);
	}
	glVertex2f(radius, 0.0f);
	glEnd();
	glPopMatrix();
}