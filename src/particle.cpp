#include "pch.h"
#include "particle.h"

constexpr Vector2i DrawSize = { 8, 8 };
constexpr Vector2i HDrawSize = { DrawSize.x >> 1, DrawSize.y >> 1 };


FORCEINLINE constexpr byte lerp(byte a, byte b, float l)
{
	return a + ((b - a) * l);
}

FORCEINLINE constexpr byte lerp3(byte a, byte b, byte c, float l)
{
	if (l < .5f)
		return lerp(a, b, l * 2.0f);
	[[unlikely]] if (l == .5f) return b;
	return lerp(b, c, (l - 0.5f) * 2.0f);
}

FORCEINLINE constexpr Colorb lerp(Colorb a, Colorb b, float l)
{
	return { lerp(a.r, b.r, l), lerp(a.g, b.g, l), lerp(a.b, b.b, l), lerp(a.a, b.a, l) };
}

FORCEINLINE constexpr Colorb lerp3(Colorb a, Colorb b, Colorb c, float l)
{
	return { lerp3(a.r, b.r, c.r, l), lerp3(a.g, b.g, c.g, l), lerp3(a.b, b.b, c.b, l), lerp3(a.a, b.a, c.a, l) };
}

Particle::Particle(Vector2f pos)
	: m_pos{ pos }
{
}

void Particle::draw(ig::Context2D &c) const
{
	c.rect(
		m_pos - HDrawSize,
		m_pos + HDrawSize,
		lerp3({ 60, 60, 255 }, { 60, 255, 60 }, { 255, 60, 60 }, std::min(m_vel.length() / 360.f, 1.f))
	);
}
