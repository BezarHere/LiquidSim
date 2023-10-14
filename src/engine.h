#pragma once
#include "pch.h"
#include "particle.h"

class Engine
{
public:
	static Recti get_world_rect();

	static void start();
	static void update();
	static void draw(ig::Context2D c);


};
