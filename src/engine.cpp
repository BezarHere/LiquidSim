#include "pch.h"
#include "engine.h"

//! GO GLOBAL VARIABLES HELL! GO!

constexpr int ChunkSizeBitshift = 5;
constexpr Vector2i ChunkSize = { 1 << ChunkSizeBitshift, 1 << ChunkSizeBitshift };

constexpr Vector2i ParticleMapSize = { 64, 64 };
constexpr size_t ParticleCount = ParticleMapSize.area();

constexpr float PressureCurveRadius = 32.0f;
constexpr float PressureCurveRadiusSquared = PressureCurveRadius * PressureCurveRadius;
constexpr Vector2i PressureHalfAABBSize = Vector2i(int(PressureCurveRadius) / 2, int(PressureCurveRadius) / 2);

constexpr Recti WorldRect{ 0, 0, 1200, 900 };
constexpr Vector2i ChunksMapSize = (WorldRect.size() / ChunkSize) + Vector2i{ 1, 1 };
constexpr size_t ChunksCount = (size_t)ChunksMapSize.area();


float g_StiffnessMultiplair = 0.2;
float g_CollisionDampingFactor = 0.1;
Vector2f g_Gravity = { 0.f, 98.f };

bool g_StartedEngine = false;

typedef std::shared_ptr<Particle> SPParticle_t;
typedef unsigned particle_id_t;

std::array<SPParticle_t, ParticleCount> g_Particles{};

struct Chunk
{
#ifdef LOW_MEM
	static constexpr size_t IdsPreallocateSize = (size_t)(ChunkSize.area() / (int)(PressureCurveRadius * PressureCurveRadius)) * 2;
#else
	static constexpr size_t IdsPreallocateSize = ParticleCount + 64;
#endif
	FORCEINLINE Chunk()
	{
		if constexpr (IdsPreallocateSize > 2)
			ids.reserve(IdsPreallocateSize);
	}


	std::vector<particle_id_t> ids;
};

std::array<Chunk, ChunksCount> g_ParticleChunks;



FORCEINLINE float pressure_curve_sq(float distance_squared)
{
	constexpr float PressureEffectArea = Pi * PressureCurveRadiusSquared / 4.0f;
	 const float v = std::max(0.0f, PressureCurveRadiusSquared - distance_squared);
	 return v * v * v / PressureEffectArea;
}

FORCEINLINE float pressure_curve(float distance)
{
	return pressure_curve_sq(distance * distance);
}

Recti Engine::get_world_rect()
{
	return WorldRect;
}

FORCEINLINE SPParticle_t get_particle(const size_t index)
{
	return g_Particles[ index ];
}

FORCEINLINE Particle *get_particle_ptr(const size_t index)
{
	return g_Particles[ index ].get();
}

FORCEINLINE constexpr particle_id_t to_particle_index(const Vector2i index)
{
	return index.x + (index.y * ParticleMapSize.x);
}

FORCEINLINE constexpr bool is_chunk_pos_valid(Vector2i pos) noexcept
{
	return pos.x >= 0 && pos.y >= 0 && pos.x < ChunksMapSize.x && pos.y < ChunksMapSize.y;
}

FORCEINLINE Vector2i to_chunk_position(Vector2i pos)
{
	return pos >> ChunkSizeBitshift;
}

FORCEINLINE Chunk &get_chunk(Vector2i chunk_pos)
{
	return g_ParticleChunks[ (size_t)(chunk_pos.x + (chunk_pos.y * ChunksMapSize.x)) ];
}

FORCEINLINE Chunk &get_chunk_containing(Vector2i pos)
{
	return get_chunk(to_chunk_position(pos));
}


FORCEINLINE Vector2f _calc_pressure_force_from_chunk(Vector2i chunk, Vector2f from_pos)
{
	Vector2f force{};
	for (const particle_id_t p : get_chunk(chunk).ids)
	{
		Particle *particle = get_particle_ptr(p);
		const float distance = from_pos.distance_squared(particle->position());

		if (distance >= PressureCurveRadiusSquared)
			continue;

		const Vector2f f = (from_pos - particle->position()) * pressure_curve_sq(distance);
		particle->velocity(particle->velocity() - f);

		force += f;
	}
	return force * g_StiffnessMultiplair;
}

FORCEINLINE Vector2f _calc_pressure_force_from_chunk_safe(Vector2i chunk, Vector2f from_pos)
{
	if (!is_chunk_pos_valid(chunk))
		return { 0.f, 0.f };
	return _calc_pressure_force_from_chunk(chunk, from_pos);
}

FORCEINLINE Vector2f calc_pressure(particle_id_t index)
{
	const Vector2f pos = get_particle(index)->position();

	const Vector2i topleft_chunk_pos = to_chunk_position(pos - PressureHalfAABBSize);
	const Vector2i bottomright_chunk_pos = to_chunk_position(pos + PressureHalfAABBSize);

	

	// pressure effect is only on one chunk
	// *-----------*
	// \	+------+ \
	// \  \      \ \
	// \	\			 \ \
	// \	+------+ \
	// *-----------*
	if (topleft_chunk_pos == bottomright_chunk_pos)
	{
		return _calc_pressure_force_from_chunk_safe(topleft_chunk_pos, pos);
	}

	const Vector2i bottomleft_chunk_pos = { topleft_chunk_pos.x, bottomright_chunk_pos.y };
	const Vector2i topright_chunk_pos = { bottomright_chunk_pos.x, topleft_chunk_pos.y };
	
	// *-----------*-----------*
	// \	+--------\----+			 \
	// \  \				 \		\			 \
	// \	\				 \		\			 \
	// \	+--------\----+			 \
	// *-----------*-----------*
	//# commented becuse this state if very unlikely, but may be uncommented in the future
	//# reason it's very unlikely is that the pressure virtual AABB is always squared and 'limited in size' thus it can only effect one or 4 chunk
	//if (bottomleft_chunk_pos == topleft_chunk_pos || topright_chunk_pos == bottomright_chunk_pos)
	//	return _calc_pressure_force_from_chunk(topleft_chunk_pos, pos) + _calc_pressure_force_from_chunk(bottomright_chunk_pos, pos);
	
	// *-----------*-----------*
	// \	+--------\---------+ \
	// \  \				 \		 		 \ \
	// \	\				 \		 		 \ \
	// \	\  			 \         \ \
	// *--\--------*---------\-*
	// \	\        \     		 \ \
	// \  \				 \		 		 \ \
	// \	\				 \				 \ \
	// \	+--------\---------+ \
	// *-----------*-----------*

	return
		_calc_pressure_force_from_chunk_safe(bottomright_chunk_pos, pos) + _calc_pressure_force_from_chunk_safe(bottomleft_chunk_pos, pos)
		+ _calc_pressure_force_from_chunk_safe(topleft_chunk_pos, pos) + _calc_pressure_force_from_chunk_safe(topright_chunk_pos, pos);
}

FORCEINLINE void repack_chunks()
{
	for (auto &c : g_ParticleChunks)
		c.ids.clear();

	for (particle_id_t i = 0; i < ParticleCount; i++)
	{
		const Vector2i chunk_pos = to_chunk_position(get_particle(i)->position());

		get_chunk(chunk_pos).ids.push_back(i);
	}
}

void Engine::start()
{
	assert(!g_StartedEngine);
	g_StartedEngine = true;

	for (int x = 0; x < ParticleMapSize.x; x++)
	{
		for (int y = 0; y < ParticleMapSize.y; y++)
		{
			const particle_id_t i = to_particle_index({ x, y });
			g_Particles[ i ] = SPParticle_t(new Particle{ Vector2f{ 32.f + (float)(x * 10), 32.f + (float)(y * 10) } });
		}
	}
}

void Engine::update()
{
	constexpr float dt = 1.0f / 60.0f;
	repack_chunks();

#pragma omp for parrala
	for (size_t i = 0; i < ParticleCount; i++)
	{
		g_Particles[ i ]->m_vel += calc_pressure(i);
		g_Particles[ i ]->m_vel *= 0.97f;
	}

	for (const SPParticle_t &p : g_Particles)
	{
		p->m_pos += (p->m_vel + g_Gravity) * dt;

		if (p->m_pos.x < WorldRect.x)
		{
			p->m_pos.x = WorldRect.x;
			p->m_vel.x *= -g_CollisionDampingFactor;
		}
		else if (p->m_pos.x >= WorldRect.w)
		{
			p->m_pos.x = WorldRect.w - Epsilon;
			p->m_vel.x *= -g_CollisionDampingFactor;
		}

		if (p->m_pos.y < WorldRect.y)
		{
			p->m_pos.y = WorldRect.y;
			p->m_vel.y *= -g_CollisionDampingFactor;
		}
		else if (p->m_pos.y >= WorldRect.h)
		{
			p->m_pos.y = WorldRect.h - Epsilon;
			p->m_vel.y *= -g_CollisionDampingFactor;
		}
	}

}

void Engine::draw(ig::Context2D c)
{

	for (const auto &p : g_Particles)
		p->draw(c);

}
