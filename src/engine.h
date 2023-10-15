#pragma once
#include "pch.h"
#include "particle.h"

constexpr int ChunkSizeBitshift = 4;
constexpr Vector2i ChunkSize = { 1 << ChunkSizeBitshift, 1 << ChunkSizeBitshift };

constexpr Vector2i ParticleMapSize = { 64, 64 };
constexpr size_t ParticleCount = ParticleMapSize.area();

constexpr float PressureCurveRadius = 16.0f;
constexpr float PressureCurveRadiusSquared = PressureCurveRadius * PressureCurveRadius;
constexpr Vector2i PressureHalfAABBSize = Vector2i(int(PressureCurveRadius) / 2, int(PressureCurveRadius) / 2);

constexpr Recti WorldRect{ 0, 0, 600, 400 };
constexpr Vector2i ChunksMapSize = (WorldRect.size() / ChunkSize) + Vector2i{ 1, 1 };
constexpr size_t ChunksCount = (size_t)ChunksMapSize.area();

class Engine
{
public:
	static Recti get_world_rect();

	static void start();
	static void update();
	static void draw(ig::Context2D c);


};
