#include "../../MQEQGFX.h"

namespace {

ParticleGpuStats s_counting;
ParticleGpuStats s_last;

} // namespace

void NoteParticleGpuFrame(bool gated, const char* ungatedReason)
{
	if (gated)
		++s_counting.gatedFrames;
	else
		s_counting.ungatedReason = ungatedReason;
}

void NoteParticleGpuDraw(uint32_t particles, uint32_t emitters, uint32_t draws)
{
	s_counting.particles += particles;
	s_counting.emitters += emitters;
	s_counting.draws += draws;
}

void SampleParticleGpuStats()
{
	if (s_counting.gatedFrames || s_counting.draws || *s_counting.ungatedReason)
		s_last = s_counting;
	s_counting = {};
}

ParticleGpuStats GetParticleGpuStats() { return s_last; }
