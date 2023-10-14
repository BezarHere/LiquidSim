#pragma once
#include "pch.h"

class Particle
{
	friend class Engine;
public:
	Particle(Vector2f pos);

	void draw(ig::Context2D &c) const;

	FORCEINLINE Vector2f position() const noexcept
	{
		return m_pos;
	}

	FORCEINLINE void position(Vector2f val) noexcept
	{
		m_pos = val;
	}

	FORCEINLINE Vector2f velocity() const noexcept
	{
		return m_vel;
	}

	FORCEINLINE void velocity(Vector2f val) noexcept
	{
		m_vel = val;
	}

private:
	Vector2f m_pos;
	Vector2f m_vel;
};
