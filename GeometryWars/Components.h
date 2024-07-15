#pragma once

#include "Vec2.h"
#include "SFML/Graphics.hpp"

class CTransform
{
public:
	Vec2 pos = { 0.0, 0.0 };
	Vec2 velocity = { 0.0, 0.0 };
	double angle = 0.0;

	CTransform(Vec2 dataPos, Vec2 dataVelocity, float dataAngle)
		: pos(dataPos), velocity(dataVelocity), angle(dataAngle) {}
};
class CShape
{
public:
	sf::CircleShape circle;
	CShape(float radius, int points, const sf::Color fill, const sf::Color outline, float thickness)
		: circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}
};
class CCollision
{
public:
	float radius = 0.f;
	CCollision(float dataRadius)
		: radius(dataRadius) {}
	
};
class CScore
{
public:
	int score = 0;
	CScore(int dataScore)
		: score(dataScore) {}
};
class CLifespan
{
public:
	int remaining = 0;
	int total = 0;
	CLifespan(int dataTotal)
		: total(dataTotal), remaining(dataTotal) {}
};

class CInput
{
public:
	bool up = false; 
	bool down = false;
	bool left = false;
	bool right = false;

	CInput() {}
};