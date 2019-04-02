#pragma once
#include <string>

#ifndef NULL
#define NULL 0
#endif

void RunOpenGL(int &argc, char** argv, 
	void (Draw)(), 
	void (Mouse)(float x, float y) = NULL,
	void (Keyboard)(int key) = NULL
);

void AttachKeyHandle(char key, bool* value);

void PrintText(std::string fontImg, std::string text, float centerX, float centerY, float scale = 1.0f);
void DrawRectangle(float x1, float y1, float x2, float y2);
void DrawRectangle(float x1, float y1, float x2, float y2, std::string texture, bool useColor=false, float tx1=0.0f, float ty1=0.0f, float tx2 = 1.0f, float ty2 = 1.0f);
void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
void DrawCircle(float centerX, float centerY, float radius, int segments = 24);

void SetColor(int r, int g, int b, int a = 255);
void Reset();
void Rotate(float deg);