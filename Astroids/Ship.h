#pragma once
#include <list>
#include <set>
#include <string>

#include "rapidjson/document.h"

using namespace rapidjson;


const double M_PI = 3.14159;
const double RAD_TO_DEG = 180 / M_PI;


struct Point {
	Point() : x(0), y(0) {}
	Point(double heading) : x(cos(heading)), y(sin(heading)) {}
	Point(double X, double Y) : x(X), y(Y) {}
	inline double dir() { return atan2(y, x); }
	inline double mag() { return sqrt(x*x + y*y); }
	inline double dot(const Point& other) const { return x*other.x + y*other.y; }
	inline Point unit() { return *this / mag(); }
	inline Point operator+(const Point& rhs) const { return Point(x + rhs.x, y + rhs.y); }
	inline Point operator-(const Point& rhs) const { return Point(x - rhs.x, y - rhs.y); }
	inline Point operator*(const double& rhs) const { return Point(x * rhs, y * rhs); }
	inline Point operator/(const double& rhs) const { return Point(x / rhs, y / rhs); }
	inline void operator+=(const Point& rhs) { x += rhs.x; y += rhs.y; }
	inline void operator-=(const Point& rhs) { x -= rhs.x; y -= rhs.y; }
	inline void operator*=(const double& rhs) { x *= rhs; y *= rhs; }
	inline void operator/=(const double& rhs) { x /= rhs; y /= rhs; }
	rapidjson::Value encode(rapidjson::MemoryPoolAllocator<CrtAllocator>& allocator);
	double x;
	double y;
};
typedef Point Vector;


double getAngleBetween(double angle1, double angle2); 

struct Rect {
	Rect() :
		p1(0, 0), p2(1, 1) {}
	Rect(Point P1, Point P2) :
		p1(P1), p2(P2) {}
	Point p1;
	Point p2;
};

struct Sprite {
	Sprite() : size(.1,.1) {}
	Sprite(Vector Size, std::string Image, Rect SubTexture) :
		size(Size),
		image(Image),
		subTexture(SubTexture) {}
	Vector size;
	std::string image;
	Rect subTexture;
};

enum objType {
	ShipType,
	BulletType,
	AstroidType,
};
class Object {
public:
	Object();
	Object(Sprite Sprite, Point Position, Vector Delta, double Heading, double DeltaHeading);
	virtual double getRadius() const;
	virtual Point getPosition() const;
	virtual Point getDelta() const;
	virtual double getHeading() const;
	virtual rapidjson::Value encode(rapidjson::MemoryPoolAllocator<>& allocator) = 0;
	virtual void Update(double dt);
	virtual void Collide(const Object* object);
	virtual void Object::Draw();
	virtual objType getType() const = 0;

protected:
	Sprite sprite;
	Point position;
	Vector delta;
	double heading;
	double deltaHeading;
};
class Bullet : public Object {
public:
	Bullet(Point position, Point delta);
	virtual void Collide(const Object* other);
	virtual rapidjson::Value encode(rapidjson::MemoryPoolAllocator<>& allocator);
	virtual void Update(double dt);
	virtual objType getType() const;
private:
	double timeToLive;
};
class Ship : public Object {
public:
	Ship(int type, bool player1);
	virtual void Collide(const Object* other);
	virtual rapidjson::Value encode(rapidjson::MemoryPoolAllocator<>& allocator);
	virtual void Update(double dt);
	virtual objType getType() const;
	virtual void Draw();
	bool accelCommand;
	bool leftCommand;
	bool rightCommand;
	bool shootCommand;
private:
	bool isPlayer1;
	double timeAlive;
	double bullet_speed;
	double accel_rate;
	double turn_rate;
	double weapon_cooldown;
	double weapon_damage;
	double life;
	double cooldown;
};
class Astroid : public Object {
public:
	Astroid(Point position, Point delta, double size);
	virtual void Update(double dt);
	virtual void Collide(const Object* other);
	virtual rapidjson::Value encode(rapidjson::MemoryPoolAllocator<>& allocator);
	virtual objType getType() const;
private:
	double size;
};

class SceneManager {
public:
	SceneManager();
	std::string GetGameState(bool isPlayer1);
	void AddObject(Object* o);
	void DeleteObject(Object* o);
	void Update();
	void Draw();
	std::list<Object*> scene;
	std::set<Object*> toDelete;
	Ship Player;
	Ship AIPlayer;
	int Player1Socket;
	int Player2Socket;
};	

extern SceneManager manager;