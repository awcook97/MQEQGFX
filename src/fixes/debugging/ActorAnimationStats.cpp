#include "../../MQEQGFX.h"

namespace {

ActorAnimationStats s_counting;
ActorAnimationStats s_last;

} // namespace

void NoteActorAnimation(bool distant, bool skipped)
{
	if (skipped)
		++s_counting.skipped;
	else
		++s_counting.animated;
	if (distant)
		++s_counting.distant;
}

void SampleActorAnimationStats()
{
	if (s_counting.animated || s_counting.skipped)
		s_last = s_counting;
	s_counting = {};
}

ActorAnimationStats GetActorAnimationStats() { return s_last; }
