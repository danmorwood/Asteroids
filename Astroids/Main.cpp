#include "OpenGL.h"
#include "GL/glut.h"
#include "GL/GL.h"
#include "Ship.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;


void draw() {
	manager.Update();
	static float t = 0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	manager.Draw();
}

int main(int argc, char** argv) {
	srand(time(0));
	//SceneManager::AddObject(&SceneManager::Player2);
	AttachKeyHandle('w', &manager.Player.accelCommand);
	AttachKeyHandle('a', &manager.Player.leftCommand);
	AttachKeyHandle('d', &manager.Player.rightCommand);
	AttachKeyHandle(' ', &manager.Player.shootCommand);
	/*AttachKeyHandle('8', &manager.AIPlayer.accelCommand);
	AttachKeyHandle('4', &manager.AIPlayer.leftCommand);
	AttachKeyHandle('6', &manager.AIPlayer.rightCommand);
	AttachKeyHandle('0', &manager.AIPlayer.shootCommand);*/
	RunOpenGL(argc, argv, draw);
}
