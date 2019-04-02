#include "OpenGL.h"
#include "GL/glut.h"
#include "GL/GL.h"
#include "Ship.h"
#include "AI.h"
#include "Network.h"
#include <cstdio>

#include "rapidjson\stringbuffer.h"
#include "rapidjson\writer.h"

SceneManager manager;

Value Point::encode(MemoryPoolAllocator<CrtAllocator>& allocator) {
	Value value(kObjectType);
	value.AddMember("x", x, allocator);
	value.AddMember("y", y, allocator);
	return value;
}
Object::Object() {}
Object::Object(Sprite Sprite, Point Position, Vector Delta, double Heading, double DeltaHeading) :
	sprite(Sprite),
	position(Position),
	delta(Delta),
	heading(Heading),
	deltaHeading(DeltaHeading) {
}
double Object::getRadius() const {
	return (sprite.size.x + sprite.size.y) / 4.0;
}
Point Object::getPosition() const {
	return position;
}
Point Object::getDelta() const { return delta; }
double Object::getHeading() const
{
	return heading;
}
void Object::Update(double dt) {
	position += delta * dt;
	if (position.x > 1) position.x -= 1;
	if (position.y > 1) position.y -= 1;
	if (position.x < 0) position.x += 1;
	if (position.y < 0) position.y += 1;
	heading += deltaHeading * dt;
	heading = fmod(heading + 2 * M_PI, 2 * M_PI);
	if (heading > M_PI) heading -= 2 * M_PI;
}
void Object::Collide(const Object* object) {
	manager.DeleteObject(this);
}
void Object::Draw() {
	glPushMatrix();
	glTranslated(position.x, position.y, 0);
	glRotated(heading * RAD_TO_DEG - 90, 0, 0, 1);
	DrawRectangle(
		-sprite.size.x / 2, -sprite.size.y / 2,
		sprite.size.x / 2, sprite.size.y / 2,
		sprite.image, true,
		sprite.subTexture.p1.x, sprite.subTexture.p1.y,
		sprite.subTexture.p2.x, sprite.subTexture.p2.y);
	glPopMatrix();
}

Bullet::Bullet(Point position, Point delta) :
	Object(Sprite(Vector(.004, .008), "img/shipsall.jpg", Rect(Point(.667, .333), Point(.777, .667))), position, delta, delta.dir(), 0),
	timeToLive(.5) {}
void Bullet::Collide(const Object* other) {
	Object::Collide(other);
}
Value Bullet::encode(MemoryPoolAllocator<>& allocator) {
	Value value(kObjectType);
	value.AddMember("pos", position.encode(allocator), allocator);
	value.AddMember("pos_delta", delta.encode(allocator), allocator);
	value.AddMember("ttl", timeToLive, allocator);
	return value;
}
void Bullet::Update(double dt) {
	Object::Update(dt);
	timeToLive -= dt;
	if (timeToLive <= 0) {
		manager.DeleteObject(this);
	}
}
objType Bullet::getType() const { return BulletType; }

Ship::Ship(int type, bool player1) : isPlayer1(player1) {
	switch (type) {
	default:
	case 0: //Sting ship
		accel_rate = 1;
		turn_rate = 1.5;
		weapon_cooldown = .1;
		weapon_damage = .09;
		life = .5;
		break;
	case 1: //Base Ship
		accel_rate = 1/3.0;
		turn_rate = 2;
		weapon_cooldown = .5;
		weapon_damage = 1;
		life = 1;
		break;
	case 2: //Gun boat
		accel_rate = .5 / 3.0;
		turn_rate = 1.6;
		weapon_cooldown = .1;
		weapon_damage = .15;
		life = 1.25;
		break;
	case 3: //TripleFleet
		accel_rate = 2 / 3.0;
		turn_rate = 11.6;
		weapon_cooldown = .05;
		weapon_damage = .025;
		life = .5;
		break;
	case 4: //Goliath
		accel_rate = .2 / 3.0;
		turn_rate = 1.0;
		weapon_cooldown = 3;
		weapon_damage = 10;
		life = 5;
		break;
	case 5: //Support
		accel_rate = 1 / 3.0;
		turn_rate = 1.0;
		weapon_cooldown = .5;
		weapon_damage = .5;
		life = 2;
		break;
	case 6: //BattleCruiser
		accel_rate = .5 / 3.0;
		turn_rate = 1.0;
		weapon_cooldown = 1.2;
		weapon_damage = 2;
		life = 3.5;
		break;
	case 7: //Pair
		accel_rate = 1.25 / 3.0;
		turn_rate = 1.8;
		weapon_cooldown = .33;
		weapon_damage = .75;
		life = .85;
		break;
	case 8: //Alt Ship
		accel_rate = 1.25 / 3.0;
		turn_rate = 1.5;
		weapon_cooldown = .75;
		weapon_damage = 1.33;
		life = 1.1;
		break;
	}
	//Apply base stats

	//Setup sprite
	sprite.image = "img/shipsall.jpg";
	sprite.size = Point(.075 / 3.0, .075 / 3.0);
	sprite.subTexture.p1.x = (int)(type % 3) / 3.0;
	sprite.subTexture.p1.y = (int)(type / 3) / 3.0;
	sprite.subTexture.p2 = sprite.subTexture.p1 + Vector(1 / 3.0, 1 / 3.0);
	cooldown = 0;
	deltaHeading = 0;
	bullet_speed = 2 / 3.0;
	position.x = rand() / (double)RAND_MAX;
	position.y = rand() / (double)RAND_MAX;
}
void Ship::Collide(const Object* other) {
	position = Point(rand()/(double)RAND_MAX, rand() / (double)RAND_MAX);
	delta = Point(0,0);
	cooldown = 0;
	timeAlive = 0;
}
Value Ship::encode(MemoryPoolAllocator<>& allocator) {
	Value value(kObjectType);
	value.AddMember(StringRef("pos"), position.encode(allocator), allocator);
	value.AddMember(StringRef("pos_delta"), delta.encode(allocator), allocator);
	value.AddMember(StringRef("heading"), heading, allocator);
	value.AddMember(StringRef("acceleration"), accelCommand ? accel_rate : 0.0, allocator);
	value.AddMember(StringRef("score"), timeAlive, allocator);
	return value;
}
void Ship::Update(double dt) {
	if (dt > .1) dt = .1;
	Object::Update(dt);
	if (accelCommand) {
		delta += Vector(heading) * accel_rate * dt;
		double speed = delta.mag();
		if (speed > accel_rate) {
			delta /= (speed / accel_rate);
		}
	}
	//deltaHeading *= 0;
	if (leftCommand) {
		deltaHeading += turn_rate * dt * 500;
		deltaHeading = deltaHeading > turn_rate ? turn_rate : deltaHeading;
	}
	if (rightCommand && deltaHeading > -turn_rate) {
		deltaHeading -= turn_rate * dt * 500;
		deltaHeading = deltaHeading < -turn_rate ? -turn_rate : deltaHeading;
	}
	if (!leftCommand && !rightCommand)
		deltaHeading *= 0;
	if (shootCommand && cooldown <= 0) {
		//TODO: Create a bullet
		manager.AddObject(new Bullet(position + (Vector(heading) * .075), delta + (Vector(heading) * bullet_speed)));
		cooldown = weapon_cooldown;
	}
	cooldown -= dt;
	timeAlive += dt;
}
objType Ship::getType() const { return ShipType; }
void Ship::Draw() {
	if(isPlayer1)
		glColor3f(1, .75, .75);
	else
		glColor3f(.75, .75, 1);
	Object::Draw();
	glColor3f(1, 1, 1);
}

Astroid::Astroid(Point position, Point delta, double size) :
	Object(Sprite(Vector(size, size), "img/astroid.png", Rect()), position, delta, delta.dir(), .001),
	size(size) {}
void Astroid::Collide(const Object* other) {
	if (other->getType() != BulletType) return;
	if (size > .05) {
		manager.AddObject(new Astroid(position, other->getDelta()*.06125 + delta + Point(rand() / (double)RAND_MAX * 2.0 - 1, rand() / (double)RAND_MAX * 2.0 - 1)*.1, 3 * size / 4));
		manager.AddObject(new Astroid(position, other->getDelta()*.06125 + delta + Point(rand() / (double)RAND_MAX * 2.0 - 1, rand() / (double)RAND_MAX * 2.0 - 1)*.1, 3 * size / 4));
	}
	Object::Collide(other);
}
void Astroid::Update(double dt) {
	Object::Update(dt);
}
Value Astroid::encode(MemoryPoolAllocator<>& allocator) {
	Value value(kObjectType);
	value.AddMember(StringRef("pos"), position.encode(allocator), allocator);
	value.AddMember(StringRef("pos_delta"), delta.encode(allocator), allocator);
	value.AddMember(StringRef("size"), size, allocator);
	return value;
}
objType Astroid::getType() const { return AstroidType; }

SceneManager::SceneManager() : Player(PlayerShipType, true), AIPlayer(AiShipType,false) {
	for (int i = 0; i < initailAstroids; i++) {
		SceneManager::AddObject(new Astroid(Point(rand() / (double)RAND_MAX * 2.0 - 1, rand() / (double)RAND_MAX * 2.0 - 1),
			Point(rand() / (double)RAND_MAX * 2.0 - 1, rand() / (double)RAND_MAX * 2.0 - 1)*.1, .125));
	}
	manager.AddObject(&Player);
	manager.AddObject(&AIPlayer);
	/*printf("Waiting for Player 1 to connect...\n");
	Player1Socket = network.CreateConnection("30000");
	printf("Player 1 Connected!\n");
	printf("Waiting for Player 2 to connect...\n");
	Player2Socket = network.CreateConnection("30001");
	printf("Player 2 Connected!\nStarting...");*/
}
std::string SceneManager::GetGameState(bool isPlayer1) {
	Document doc;
	doc.SetObject();
	if (isPlayer1) {
		doc.AddMember("myShip", Player.encode(doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("enemy", AIPlayer.encode(doc.GetAllocator()), doc.GetAllocator());
	}
	else {
		doc.AddMember("myShip", AIPlayer.encode(doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("enemy", Player.encode(doc.GetAllocator()), doc.GetAllocator());
	}
	Value astroids(kArrayType);
	Value bullets(kArrayType);

	for (Object* o : scene) {
		if (o->getType() == AstroidType && astroids.Size() < 20) {
			astroids.PushBack(o->encode(doc.GetAllocator()), doc.GetAllocator());
		}
		if (o->getType() == BulletType && bullets.Size() < 20) {
			bullets.PushBack(o->encode(doc.GetAllocator()), doc.GetAllocator());
		}
	}
	doc.AddMember("bullets", bullets, doc.GetAllocator());
	doc.AddMember("asteroids", astroids, doc.GetAllocator());

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);
	return std::string(buffer.GetString()) + "\n";
}
void SceneManager::AddObject(Object* o) {
	scene.push_front(o);
}
void SceneManager::DeleteObject(Object* o) {
	toDelete.insert(o);
}
void SceneManager::Update() {
	static int lastTime = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	double dt = (time - lastTime) / 1000.0;
	lastTime = time;
	DoAI(Player, AIPlayer);
	for (Object* obj : scene) {
		obj->Update(dt);
	}

	for (Object* obj : scene) {
		for (Object* other : scene) {
			if (obj <= other) continue;
			if ((obj->getPosition() - other->getPosition()).mag() < (obj->getRadius() + other->getRadius())) {
				obj->Collide(other);
				other->Collide(obj);
			}
		}
	}
	for (Object* dead : toDelete) {
		scene.remove(dead);
		delete dead;
	}
	toDelete.clear();

	/*
	network.SendTo(Player1Socket, GetGameState(true));
	network.SendTo(Player2Socket, GetGameState(false));
	
	Document doc;
	std::string msg = network.Recv(Player1Socket);
	doc.Parse(msg.c_str());
	Player1.accelCommand = doc["accel"].GetBool();
	Player1.rightCommand = doc["right"].GetBool();
	Player1.leftCommand = doc["left"].GetBool();
	Player1.shootCommand = doc["shoot"].GetBool();
	msg = network.Recv(Player2Socket);
	doc.Parse(msg.c_str());
	Player2.accelCommand = doc["accel"].GetBool();
	Player2.rightCommand = doc["right"].GetBool();
	Player2.leftCommand = doc["left"].GetBool();
	Player2.shootCommand = doc["shoot"].GetBool();*/

}
void SceneManager::Draw()
{
	for (Object* obj : scene) {
		obj->Draw();
	}
}

double getAngleBetween(double angle1, double angle2) {
	double delta = angle1 - angle2;

	delta = fmod(delta + 2 * M_PI, 2 * M_PI);
	if (delta > M_PI)
		delta -= 2 * M_PI;

	return delta;
}