#include "Glyphs.h"
#include "LineFX.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

namespace eqgfx::gpu {

namespace {

constexpr float PI  = 3.14159265358979323846f;
constexpr float TAU = 2.0f * PI;

struct P { float x, y; };
struct Seg { float x0, y0, x1, y1; };

struct Builder
{
	std::vector<Seg> segs;

	void line(P a, P b) { segs.push_back({ a.x, a.y, b.x, b.y }); }

	void poly(std::initializer_list<P> pts, bool closed = false)
	{
		const P* v = pts.begin();
		const size_t n = pts.size();
		for (size_t i = 0; i + 1 < n; ++i)
			line(v[i], v[i + 1]);
		if (closed && n > 2)
			line(v[n - 1], v[0]);
	}

	void poly(const std::vector<P>& v, bool closed = false)
	{
		for (size_t i = 0; i + 1 < v.size(); ++i)
			line(v[i], v[i + 1]);
		if (closed && v.size() > 2)
			line(v.back(), v.front());
	}

	void arc(float cx, float cy, float rx, float ry, int n, float a0 = 0.0f, float a1 = TAU)
	{
		P prev{ cx + rx * std::cos(a0), cy + ry * std::sin(a0) };
		for (int i = 1; i <= n; ++i)
		{
			const float a = a0 + (a1 - a0) * i / n;
			P cur{ cx + rx * std::cos(a), cy + ry * std::sin(a) };
			line(prev, cur);
			prev = cur;
		}
	}

	void mirrorX(std::initializer_list<P> pts)
	{
		std::vector<P> l(pts), r;
		for (P p : l) r.push_back({ -p.x, p.y });
		poly(l);
		poly(r);
	}
};

void DrawGlyph(uint32_t id, Builder& b)
{
	switch (id)
	{
	case Glyph_Cross:
		b.poly({ { -0.3f, 0.9f }, { 0.3f, 0.9f }, { 0.3f, 0.3f }, { 0.9f, 0.3f }, { 0.9f, -0.3f },
			{ 0.3f, -0.3f }, { 0.3f, -0.9f }, { -0.3f, -0.9f }, { -0.3f, -0.3f }, { -0.9f, -0.3f },
			{ -0.9f, 0.3f }, { -0.3f, 0.3f } }, true);
		break;

	case Glyph_Ankh:
		b.arc(0.0f, 0.55f, 0.3f, 0.4f, 14);
		b.line({ 0.0f, 0.15f }, { 0.0f, -0.95f });
		b.line({ -0.6f, 0.08f }, { 0.6f, 0.08f });
		break;

	case Glyph_Heart:
	{
		std::vector<P> v;
		for (int i = 0; i <= 24; ++i)
		{
			const float t = TAU * i / 24.0f;
			const float s = std::sin(t);
			const float x = 16.0f * s * s * s;
			const float y = 13.0f * std::cos(t) - 5.0f * std::cos(2 * t) - 2.0f * std::cos(3 * t) - std::cos(4 * t);
			v.push_back({ x / 17.0f, (y + 2.0f) / 16.0f });
		}
		b.poly(v);
		break;
	}

	case Glyph_Skull:
		b.arc(0.0f, 0.2f, 0.72f, 0.72f, 16, -0.35f, PI + 0.35f);
		b.poly({ { 0.68f, -0.05f }, { 0.48f, -0.45f }, { 0.28f, -0.78f }, { -0.28f, -0.78f },
			{ -0.48f, -0.45f }, { -0.68f, -0.05f } });
		b.arc(-0.28f, 0.08f, 0.18f, 0.2f, 7);
		b.arc(0.28f, 0.08f, 0.18f, 0.2f, 7);
		b.poly({ { 0.0f, -0.12f }, { -0.09f, -0.3f }, { 0.09f, -0.3f } }, true);
		for (float x : { -0.16f, 0.0f, 0.16f })
			b.line({ x, -0.52f }, { x, -0.76f });
		break;

	case Glyph_Eye:
	{
		std::vector<P> up, dn;
		for (int i = 0; i <= 10; ++i)
		{
			const float x = -1.0f + 2.0f * i / 10.0f;
			up.push_back({ x, 0.55f * (1.0f - x * x) });
			dn.push_back({ x, -0.55f * (1.0f - x * x) });
		}
		b.poly(up);
		b.poly(dn);
		b.arc(0.0f, 0.0f, 0.32f, 0.32f, 10);
		b.arc(0.0f, 0.0f, 0.11f, 0.11f, 6);
		break;
	}

	case Glyph_Star:
	{
		std::vector<P> v;
		for (int i = 0; i < 10; ++i)
		{
			const float a = PI / 2 + TAU * i / 10.0f;
			const float r = (i % 2 == 0) ? 1.0f : 0.42f;
			v.push_back({ r * std::cos(a), r * std::sin(a) });
		}
		b.poly(v, true);
		break;
	}

	case Glyph_Z:
		b.poly({ { -0.55f, 0.6f }, { 0.55f, 0.6f }, { -0.55f, -0.6f }, { 0.55f, -0.6f } });
		break;

	case Glyph_Moon:
	{
		std::vector<P> v;
		const float a0 = PI * 0.3f, a1 = PI * 1.7f;
		for (int i = 0; i <= 14; ++i)
		{
			const float a = a0 + (a1 - a0) * i / 14.0f;
			v.push_back({ 0.85f * std::cos(a), 0.85f * std::sin(a) });
		}
		for (int i = 14; i >= 0; --i)
		{
			const float a = a0 + (a1 - a0) * i / 14.0f;
			v.push_back({ 0.35f + 0.62f * std::cos(a) * 0.95f, 0.85f * std::sin(a) * 0.82f });
		}
		b.poly(v, true);
		break;
	}

	case Glyph_Sun:
		b.arc(0.0f, 0.0f, 0.45f, 0.45f, 12);
		for (int i = 0; i < 8; ++i)
		{
			const float a = TAU * i / 8.0f;
			b.line({ 0.6f * std::cos(a), 0.6f * std::sin(a) }, { 0.95f * std::cos(a), 0.95f * std::sin(a) });
		}
		break;

	case Glyph_Hourglass:
		b.line({ -0.65f, 0.9f }, { 0.65f, 0.9f });
		b.line({ -0.65f, -0.9f }, { 0.65f, -0.9f });
		b.poly({ { -0.5f, 0.9f }, { -0.1f, 0.05f }, { -0.1f, -0.05f }, { -0.5f, -0.9f } });
		b.poly({ { 0.5f, 0.9f }, { 0.1f, 0.05f }, { 0.1f, -0.05f }, { 0.5f, -0.9f } });
		b.poly({ { -0.35f, -0.9f }, { 0.0f, -0.55f }, { 0.35f, -0.9f } });
		break;

	case Glyph_Shield:
		b.poly({ { -0.75f, 0.85f }, { 0.75f, 0.85f }, { 0.75f, 0.1f }, { 0.45f, -0.45f },
			{ 0.0f, -0.92f }, { -0.45f, -0.45f }, { -0.75f, 0.1f } }, true);
		b.poly({ { -0.5f, 0.6f }, { 0.5f, 0.6f }, { 0.5f, 0.08f }, { 0.3f, -0.3f },
			{ 0.0f, -0.62f }, { -0.3f, -0.3f }, { -0.5f, 0.08f } }, true);
		break;

	case Glyph_Lightning:
		b.poly({ { 0.15f, 1.0f }, { -0.42f, 0.0f }, { 0.0f, 0.0f }, { -0.22f, -1.0f },
			{ 0.48f, 0.18f }, { 0.06f, 0.18f }, { 0.42f, 1.0f } }, true);
		break;

	case Glyph_Note:
	{
		std::vector<P> head;
		for (int i = 0; i <= 10; ++i)
		{
			const float a = TAU * i / 10.0f;
			const float x = 0.28f * std::cos(a), y = 0.19f * std::sin(a);
			head.push_back({ -0.25f + x * 0.94f - y * 0.34f, -0.62f + x * 0.34f + y * 0.94f });
		}
		b.poly(head);
		b.line({ 0.02f, -0.56f }, { 0.02f, 0.88f });
		b.poly({ { 0.02f, 0.88f }, { 0.45f, 0.58f }, { 0.4f, 0.22f } });
		break;
	}

	case Glyph_Paw:
		b.arc(0.0f, -0.38f, 0.42f, 0.34f, 12);
		for (P t : { P{ -0.55f, 0.15f }, P{ -0.2f, 0.48f }, P{ 0.2f, 0.48f }, P{ 0.55f, 0.15f } })
			b.arc(t.x, t.y, 0.16f, 0.19f, 6);
		break;

	case Glyph_Snowflake:
		for (int i = 0; i < 6; ++i)
		{
			const float a = TAU * i / 6.0f + PI / 2;
			const float c = std::cos(a), s = std::sin(a);
			b.line({ 0.0f, 0.0f }, { c, s });
			const P m{ 0.6f * c, 0.6f * s };
			for (float d : { 0.6f, -0.6f })
			{
				const float ba = a + d;
				b.line(m, { m.x + 0.3f * std::cos(ba), m.y + 0.3f * std::sin(ba) });
			}
		}
		break;

	case Glyph_Leaf:
	{
		std::vector<P> l, r;
		for (int i = 0; i <= 10; ++i)
		{
			const float t = i / 10.0f;
			const float y = -0.8f + 1.75f * t;
			const float w = 0.55f * std::sin(PI * t) * (1.0f - 0.25f * t);
			l.push_back({ -w, y });
			r.push_back({ w, y });
		}
		b.poly(l);
		b.poly(r);
		b.line({ 0.0f, -1.0f }, { 0.0f, 0.8f });
		for (float y : { -0.35f, 0.0f, 0.35f })
		{
			b.line({ 0.0f, y }, { -0.32f, y + 0.25f });
			b.line({ 0.0f, y }, { 0.32f, y + 0.25f });
		}
		break;
	}

	case Glyph_Skeleton:
		b.arc(0.0f, 0.8f, 0.15f, 0.17f, 8);
		b.poly({ { -0.08f, 0.66f }, { -0.05f, 0.6f }, { 0.05f, 0.6f }, { 0.08f, 0.66f } });
		b.line({ -0.06f, 0.82f }, { -0.03f, 0.79f });
		b.line({ 0.06f, 0.82f }, { 0.03f, 0.79f });
		b.line({ 0.0f, 0.6f }, { 0.0f, -0.08f });
		for (float y : { 0.44f, 0.32f, 0.2f })
		{
			b.poly({ { 0.0f, y }, { -0.2f, y - 0.03f }, { -0.18f, y - 0.1f } });
			b.poly({ { 0.0f, y }, { 0.2f, y - 0.03f }, { 0.18f, y - 0.1f } });
		}
		b.line({ -0.3f, 0.54f }, { 0.3f, 0.54f });
		b.poly({ { -0.3f, 0.54f }, { -0.5f, 0.78f }, { -0.55f, 1.0f } });
		b.poly({ { -0.55f, 1.0f }, { -0.62f, 1.06f } });
		b.poly({ { -0.55f, 1.0f }, { -0.52f, 1.08f } });
		b.poly({ { 0.3f, 0.54f }, { 0.42f, 0.16f }, { 0.52f, -0.16f } });
		b.poly({ { 0.52f, -0.16f }, { 0.6f, -0.22f } });
		b.poly({ { 0.52f, -0.16f }, { 0.5f, -0.25f } });
		b.poly({ { -0.18f, -0.06f }, { 0.18f, -0.06f }, { 0.12f, -0.17f }, { -0.12f, -0.17f } }, true);
		b.poly({ { -0.12f, -0.17f }, { -0.15f, -0.56f }, { -0.14f, -0.95f }, { -0.26f, -0.98f } });
		b.poly({ { 0.12f, -0.17f }, { 0.15f, -0.56f }, { 0.18f, -0.95f }, { 0.3f, -0.98f } });
		break;

	case Glyph_Sword:
		b.poly({ { 0.0f, 1.0f }, { 0.08f, 0.8f }, { 0.08f, -0.35f }, { -0.08f, -0.35f }, { -0.08f, 0.8f } }, true);
		b.line({ -0.38f, -0.35f }, { 0.38f, -0.35f });
		b.line({ 0.0f, -0.35f }, { 0.0f, -0.8f });
		b.arc(0.0f, -0.88f, 0.08f, 0.08f, 6);
		break;

	case Glyph_Feather:
	{
		std::vector<P> l, r;
		for (int i = 0; i <= 10; ++i)
		{
			const float t = i / 10.0f;
			const float y = -0.7f + 1.7f * t;
			const float cx = 0.12f * t * t;
			const float w = 0.38f * std::sin(PI * std::min(1.0f, t * 1.05f));
			l.push_back({ cx - w, y - 0.12f * w });
			r.push_back({ cx + w * 0.8f, y - 0.12f * w });
		}
		b.poly(l);
		b.poly(r);
		b.poly({ { 0.0f, -1.0f }, { 0.03f, 0.0f }, { 0.12f, 1.0f } });
		break;
	}

	case Glyph_Link:
		b.arc(0.0f, 0.0f, 0.38f, 0.85f, 14);
		b.arc(0.0f, 0.0f, 0.18f, 0.62f, 10);
		break;

	case Glyph_Spiral:
	{
		std::vector<P> v;
		for (int i = 0; i <= 28; ++i)
		{
			const float t = i / 28.0f;
			const float a = t * TAU * 2.5f;
			v.push_back({ t * std::cos(a), t * std::sin(a) });
		}
		b.poly(v);
		break;
	}

	case Glyph_Drop:
	{
		std::vector<P> v{ { 0.0f, 1.0f } };
		for (int i = 0; i <= 14; ++i)
		{
			const float a = PI * 0.15f - (PI * 1.3f) * i / 14.0f;
			v.push_back({ 0.55f * std::cos(a), -0.3f + 0.55f * std::sin(a) });
		}
		b.poly(v, true);
		break;
	}

	case Glyph_ArrowUp:
		b.poly({ { 0.0f, 1.0f }, { 0.7f, 0.2f }, { 0.28f, 0.2f }, { 0.28f, -0.9f },
			{ -0.28f, -0.9f }, { -0.28f, 0.2f }, { -0.7f, 0.2f } }, true);
		break;

	case Glyph_ArrowDown:
		b.poly({ { 0.0f, -1.0f }, { 0.7f, -0.2f }, { 0.28f, -0.2f }, { 0.28f, 0.9f },
			{ -0.28f, 0.9f }, { -0.28f, -0.2f }, { -0.7f, -0.2f } }, true);
		break;

	case Glyph_Flame:
		b.poly({ { 0.0f, 1.0f }, { 0.3f, 0.45f }, { 0.55f, -0.1f }, { 0.45f, -0.65f }, { 0.0f, -0.92f },
			{ -0.45f, -0.65f }, { -0.55f, -0.1f }, { -0.32f, 0.25f }, { -0.18f, 0.02f }, { -0.12f, 0.55f } }, true);
		b.poly({ { 0.0f, 0.2f }, { 0.2f, -0.25f }, { 0.12f, -0.6f }, { 0.0f, -0.7f },
			{ -0.15f, -0.58f }, { -0.18f, -0.3f } }, true);
		break;

	case Glyph_Pentagram:
		b.arc(0.0f, 0.0f, 1.0f, 1.0f, 20);
		{
			std::vector<P> v;
			for (int i = 0; i < 5; ++i)
			{
				const float a = PI / 2 + TAU * (i * 2 % 5) / 5.0f;
				v.push_back({ 0.95f * std::cos(a), 0.95f * std::sin(a) });
			}
			b.poly(v, true);
		}
		break;

	case Glyph_Rock:
		b.poly({ { -0.8f, -0.55f }, { -0.9f, 0.05f }, { -0.45f, 0.65f }, { 0.25f, 0.8f },
			{ 0.85f, 0.3f }, { 0.75f, -0.5f }, { 0.1f, -0.8f } }, true);
		b.poly({ { -0.45f, 0.65f }, { -0.1f, 0.1f }, { 0.75f, -0.5f } });
		b.line({ -0.1f, 0.1f }, { -0.8f, -0.55f });
		break;

	case Glyph_Wave:
		for (float y0 : { 0.45f, 0.0f, -0.45f })
		{
			std::vector<P> v;
			for (int i = 0; i <= 12; ++i)
			{
				const float x = -1.0f + 2.0f * i / 12.0f;
				v.push_back({ x, y0 + 0.18f * std::sin(x * PI * 1.5f) });
			}
			b.poly(v);
		}
		break;

	case Glyph_Tornado:
		for (int i = 0; i < 6; ++i)
		{
			const float y = 0.9f - i * 0.33f;
			const float w = 0.95f - i * 0.15f;
			const float x = 0.08f * std::sin(i * 1.3f);
			b.line({ x - w, y }, { x + w, y });
		}
		b.poly({ { 0.95f, 0.9f }, { 0.7f, 0.25f }, { 0.35f, -0.4f }, { 0.1f, -0.75f } });
		break;

	case Glyph_Hand:
		for (auto [x, top] : { std::pair{ -0.36f, 0.5f }, std::pair{ -0.12f, 0.78f },
			std::pair{ 0.12f, 0.84f }, std::pair{ 0.36f, 0.62f } })
		{
			b.line({ x - 0.1f, -0.05f }, { x - 0.1f, top });
			b.arc(x, top, 0.1f, 0.1f, 4, PI, 0.0f);
			b.line({ x + 0.1f, top }, { x + 0.1f, -0.05f });
		}
		b.line({ -0.46f, -0.05f }, { -0.4f, -0.95f });
		b.line({ -0.4f, -0.95f }, { 0.35f, -0.95f });
		b.poly({ { 0.46f, -0.05f }, { 0.46f, -0.25f }, { 0.8f, 0.15f }, { 0.92f, 0.05f },
			{ 0.58f, -0.6f }, { 0.35f, -0.95f } });
		break;

	case Glyph_Hammer:
		b.poly({ { -0.06f, -1.0f }, { -0.06f, 0.4f } });
		b.poly({ { 0.06f, -1.0f }, { 0.06f, 0.4f } });
		b.poly({ { -0.75f, 0.4f }, { 0.75f, 0.4f }, { 0.75f, 0.9f }, { -0.75f, 0.9f } }, true);
		b.poly({ { -0.75f, 0.45f }, { -0.9f, 0.55f }, { -0.9f, 0.75f }, { -0.75f, 0.85f } });
		b.poly({ { 0.75f, 0.45f }, { 0.9f, 0.55f }, { 0.9f, 0.75f }, { 0.75f, 0.85f } });
		for (float y : { -0.55f, -0.7f, -0.85f })
			b.line({ -0.1f, y }, { 0.1f, y + 0.08f });
		break;

	case Glyph_Scales:
		b.line({ 0.0f, -0.9f }, { 0.0f, 0.75f });
		b.line({ -0.4f, -0.9f }, { 0.4f, -0.9f });
		b.line({ -0.8f, 0.6f }, { 0.8f, 0.6f });
		b.arc(0.0f, 0.82f, 0.08f, 0.08f, 6);
		for (float sx : { -1.0f, 1.0f })
		{
			b.line({ 0.8f * sx, 0.6f }, { 1.0f * sx, -0.05f });
			b.line({ 0.8f * sx, 0.6f }, { 0.6f * sx, -0.05f });
			b.arc(0.8f * sx, -0.05f, 0.24f, 0.18f, 6, PI, TAU);
			b.line({ 0.56f * sx, -0.05f }, { 1.04f * sx, -0.05f });
		}
		break;

	case Glyph_Crown:
		b.poly({ { -0.8f, -0.5f }, { -0.8f, 0.4f }, { -0.4f, 0.0f }, { 0.0f, 0.7f }, { 0.4f, 0.0f },
			{ 0.8f, 0.4f }, { 0.8f, -0.5f } }, true);
		b.line({ -0.8f, -0.25f }, { 0.8f, -0.25f });
		for (P j : { P{ -0.8f, 0.5f }, P{ 0.0f, 0.8f }, P{ 0.8f, 0.5f } })
			b.arc(j.x, j.y, 0.09f, 0.09f, 5);
		b.poly({ { 0.0f, -0.3f }, { -0.1f, -0.38f }, { 0.0f, -0.46f }, { 0.1f, -0.38f } }, true);
		break;

	case Glyph_Chalice:
		b.arc(0.0f, 0.45f, 0.55f, 0.5f, 10, PI, TAU);
		b.line({ -0.55f, 0.45f }, { 0.55f, 0.45f });
		b.arc(0.0f, 0.45f, 0.55f, 0.1f, 8, 0.0f, PI);
		b.line({ -0.05f, -0.05f }, { -0.05f, -0.7f });
		b.line({ 0.05f, -0.05f }, { 0.05f, -0.7f });
		b.arc(0.0f, -0.8f, 0.4f, 0.12f, 10);
		b.arc(0.0f, -0.3f, 0.1f, 0.06f, 5);
		break;

	case Glyph_Book:
		b.poly({ { 0.0f, -0.6f }, { -0.9f, -0.45f }, { -0.9f, 0.65f }, { 0.0f, 0.5f } });
		b.poly({ { 0.0f, -0.6f }, { 0.9f, -0.45f }, { 0.9f, 0.65f }, { 0.0f, 0.5f } });
		b.line({ 0.0f, -0.6f }, { 0.0f, 0.5f });
		for (float y : { 0.3f, 0.1f, -0.1f, -0.3f })
		{
			b.line({ -0.75f, y + 0.12f }, { -0.15f, y + 0.02f });
			b.line({ 0.15f, y + 0.02f }, { 0.75f, y + 0.12f });
		}
		break;

	case Glyph_Steed:
		b.poly({ { -0.55f, -0.95f }, { 0.55f, -0.95f }, { 0.5f, -0.55f }, { 0.2f, -0.2f }, { 0.55f, -0.15f },
			{ 0.8f, -0.25f }, { 0.92f, 0.05f }, { 0.45f, 0.55f }, { 0.2f, 0.75f }, { 0.05f, 0.95f },
			{ -0.05f, 0.75f }, { -0.4f, 0.55f }, { -0.6f, 0.1f }, { -0.55f, -0.4f } }, true);
		b.arc(0.35f, 0.35f, 0.07f, 0.07f, 5);
		b.poly({ { -0.05f, 0.75f }, { -0.35f, 0.4f }, { -0.3f, 0.05f } });
		b.poly({ { -0.2f, 0.72f }, { -0.5f, 0.3f }, { -0.48f, -0.1f } });
		break;

	case Glyph_Dove:
		b.arc(0.0f, -0.1f, 0.55f, 0.22f, 12);
		b.arc(0.6f, 0.08f, 0.16f, 0.15f, 6);
		b.line({ 0.76f, 0.08f }, { 0.95f, 0.02f });
		b.poly({ { -0.1f, 0.08f }, { -0.55f, 0.85f }, { -0.2f, 0.55f }, { 0.0f, 0.95f }, { 0.18f, 0.1f } });
		b.poly({ { -0.52f, -0.12f }, { -0.95f, -0.38f }, { -0.9f, 0.05f }, { -0.55f, -0.02f } });
		break;

	case Glyph_Anvil:
		b.poly({ { -0.95f, 0.45f }, { 0.65f, 0.45f }, { 0.65f, 0.2f }, { 0.3f, 0.2f }, { 0.3f, -0.3f },
			{ 0.6f, -0.6f }, { 0.6f, -0.8f }, { -0.6f, -0.8f }, { -0.6f, -0.6f }, { -0.3f, -0.3f },
			{ -0.3f, 0.2f }, { -0.55f, 0.2f } }, true);
		b.line({ -0.6f, -0.6f }, { 0.6f, -0.6f });
		break;

	case Glyph_Banner:
		b.line({ -0.6f, -1.0f }, { -0.6f, 0.9f });
		b.arc(-0.6f, 0.95f, 0.07f, 0.07f, 5);
		b.poly({ { -0.6f, 0.85f }, { 0.85f, 0.85f }, { 0.5f, 0.3f }, { 0.85f, -0.25f }, { -0.6f, -0.25f } });
		b.line({ 0.02f, 0.72f }, { 0.02f, -0.12f });
		b.line({ -0.3f, 0.4f }, { 0.34f, 0.4f });
		break;

	case Glyph_Shackle:
		b.arc(-0.55f, 0.0f, 0.36f, 0.36f, 12);
		b.arc(0.55f, 0.0f, 0.36f, 0.36f, 12);
		b.arc(-0.55f, 0.0f, 0.26f, 0.26f, 8);
		b.arc(0.55f, 0.0f, 0.26f, 0.26f, 8);
		b.arc(0.0f, 0.0f, 0.2f, 0.08f, 6);
		break;

	case Glyph_Tower:
		b.poly({ { -0.5f, -0.95f }, { -0.5f, 0.55f }, { -0.65f, 0.55f }, { -0.65f, 0.9f }, { -0.4f, 0.9f },
			{ -0.4f, 0.75f }, { -0.12f, 0.75f }, { -0.12f, 0.9f }, { 0.12f, 0.9f }, { 0.12f, 0.75f },
			{ 0.4f, 0.75f }, { 0.4f, 0.9f }, { 0.65f, 0.9f }, { 0.65f, 0.55f }, { 0.5f, 0.55f },
			{ 0.5f, -0.95f } }, true);
		b.poly({ { -0.2f, -0.95f }, { -0.2f, -0.6f } });
		b.arc(0.0f, -0.6f, 0.2f, 0.22f, 6, PI, 0.0f);
		b.poly({ { 0.2f, -0.6f }, { 0.2f, -0.95f } });
		b.poly({ { -0.08f, 0.2f }, { 0.08f, 0.2f }, { 0.08f, 0.45f }, { -0.08f, 0.45f } }, true);
		break;

	case Glyph_Fist:
		b.poly({ { -0.45f, -0.95f }, { -0.45f, -0.1f }, { -0.6f, 0.1f }, { -0.6f, 0.6f }, { 0.55f, 0.6f },
			{ 0.62f, 0.0f }, { 0.4f, -0.2f }, { 0.4f, -0.95f } });
		b.line({ -0.6f, 0.25f }, { 0.58f, 0.25f });
		for (float x : { -0.3f, 0.0f, 0.3f })
			b.line({ x, 0.25f }, { x, 0.6f });
		b.poly({ { -0.6f, 0.1f }, { 0.2f, 0.08f }, { 0.4f, -0.05f } });
		break;

	case Glyph_Lantern:
		b.arc(0.0f, 0.86f, 0.14f, 0.11f, 6);
		b.poly({ { -0.45f, 0.55f }, { 0.0f, 0.75f }, { 0.45f, 0.55f } });
		b.poly({ { -0.4f, 0.55f }, { 0.4f, 0.55f }, { 0.5f, -0.55f }, { -0.5f, -0.55f } }, true);
		b.line({ -0.58f, -0.72f }, { 0.58f, -0.72f });
		b.line({ -0.5f, -0.55f }, { -0.58f, -0.72f });
		b.line({ 0.5f, -0.55f }, { 0.58f, -0.72f });
		b.poly({ { 0.0f, -0.35f }, { -0.16f, -0.02f }, { 0.0f, 0.35f }, { 0.16f, -0.02f } }, true);
		break;

	case Glyph_Helm:
		b.poly({ { -0.6f, -0.9f }, { -0.66f, 0.3f }, { -0.45f, 0.75f }, { 0.0f, 0.9f }, { 0.45f, 0.75f },
			{ 0.66f, 0.3f }, { 0.6f, -0.9f } }, true);
		b.line({ -0.55f, 0.18f }, { 0.55f, 0.18f });
		b.line({ -0.55f, 0.04f }, { 0.55f, 0.04f });
		b.line({ 0.0f, 0.04f }, { 0.0f, -0.9f });
		for (float y : { -0.3f, -0.5f })
		{
			b.line({ 0.15f, y }, { 0.35f, y });
			b.line({ -0.35f, y }, { -0.15f, y });
		}
		break;

	case Glyph_Horn:
	{
		auto at = [](float t, float side) {
			const float a = PI * (0.95f - 0.8f * t);
			const float w = (0.04f + 0.3f * t * t) * side;
			const float r = 0.75f + w;
			return P{ r * std::cos(a) + 0.05f, r * std::sin(a) - 0.45f };
		};
		std::vector<P> top, bot;
		for (int i = 0; i <= 10; ++i)
		{
			top.push_back(at(i / 10.0f, 1.0f));
			bot.push_back(at(i / 10.0f, -1.0f));
		}
		b.poly(top);
		b.poly(bot);
		const P e0 = at(1.0f, 1.0f), e1 = at(1.0f, -1.0f);
		b.arc((e0.x + e1.x) * 0.5f, (e0.y + e1.y) * 0.5f, (e0.x - e1.x) * 0.5f, 0.1f, 10);
		b.line(at(0.0f, 1.0f), at(0.0f, -1.0f));
		for (float t : { 0.3f, 0.55f })
			b.line(at(t, 1.0f), at(t, -1.0f));
		break;
	}

	case Glyph_Tree:
		b.poly({ { -0.12f, -0.95f }, { -0.1f, -0.15f } });
		b.poly({ { 0.12f, -0.95f }, { 0.1f, -0.15f } });
		b.arc(0.0f, 0.35f, 0.6f, 0.52f, 16);
		b.poly({ { 0.0f, -0.15f }, { -0.3f, 0.25f } });
		b.poly({ { 0.0f, -0.05f }, { 0.28f, 0.35f } });
		b.poly({ { -0.12f, -0.95f }, { -0.4f, -1.0f } });
		b.poly({ { 0.12f, -0.95f }, { 0.4f, -1.0f } });
		break;

	case Glyph_FleurDeLis:
		b.poly({ { 0.0f, 0.95f }, { 0.18f, 0.55f }, { 0.15f, 0.1f }, { 0.0f, -0.05f }, { -0.15f, 0.1f },
			{ -0.18f, 0.55f } }, true);
		b.mirrorX({ { 0.15f, 0.0f }, { 0.35f, 0.3f }, { 0.55f, 0.52f }, { 0.75f, 0.6f }, { 0.9f, 0.5f },
			{ 0.88f, 0.3f }, { 0.74f, 0.22f }, { 0.64f, 0.32f } });
		b.poly({ { -0.45f, -0.05f }, { 0.45f, -0.05f }, { 0.45f, -0.25f }, { -0.45f, -0.25f } }, true);
		b.poly({ { -0.1f, -0.25f }, { 0.0f, -0.8f }, { 0.1f, -0.25f } });
		b.mirrorX({ { 0.15f, -0.25f }, { 0.35f, -0.45f }, { 0.58f, -0.52f }, { 0.7f, -0.42f }, { 0.62f, -0.32f } });
		break;

	case Glyph_Cog:
	{
		std::vector<P> v;
		for (int i = 0; i < 8; ++i)
		{
			const float a = TAU * i / 8.0f;
			for (auto [da, r] : { std::pair{ -0.2f, 0.72f }, std::pair{ -0.13f, 0.95f },
				std::pair{ 0.13f, 0.95f }, std::pair{ 0.2f, 0.72f } })
				v.push_back({ r * std::cos(a + da), r * std::sin(a + da) });
		}
		b.poly(v, true);
		b.arc(0.0f, 0.0f, 0.28f, 0.28f, 10);
		break;
	}

	case Glyph_Coin:
		b.arc(0.0f, 0.0f, 0.85f, 0.85f, 18);
		b.arc(0.0f, 0.0f, 0.66f, 0.66f, 14);
		b.poly({ { 0.0f, 0.4f }, { -0.3f, 0.0f }, { 0.0f, -0.4f }, { 0.3f, 0.0f } }, true);
		break;

	case Glyph_Door:
		b.line({ -0.6f, -0.95f }, { -0.6f, 0.3f });
		b.arc(0.0f, 0.3f, 0.6f, 0.6f, 12, PI, 0.0f);
		b.line({ 0.6f, 0.3f }, { 0.6f, -0.95f });
		b.line({ 0.0f, 0.9f }, { 0.0f, -0.95f });
		b.line({ -0.8f, -0.95f }, { 0.8f, -0.95f });
		b.arc(0.18f, -0.2f, 0.06f, 0.06f, 5);
		b.arc(-0.18f, -0.2f, 0.06f, 0.06f, 5);
		break;

	case Glyph_Gem:
		b.poly({ { -0.9f, 0.35f }, { -0.5f, 0.8f }, { 0.5f, 0.8f }, { 0.9f, 0.35f }, { 0.0f, -0.95f } }, true);
		b.line({ -0.9f, 0.35f }, { 0.9f, 0.35f });
		b.poly({ { -0.5f, 0.8f }, { -0.25f, 0.35f }, { 0.0f, 0.8f }, { 0.25f, 0.35f }, { 0.5f, 0.8f } });
		b.poly({ { -0.25f, 0.35f }, { 0.0f, -0.95f }, { 0.25f, 0.35f } });
		break;

	case Glyph_Mask:
		b.arc(0.0f, 0.2f, 0.72f, 0.95f, 14, PI, TAU);
		b.arc(0.0f, 0.2f, 0.72f, 0.2f, 8, 0.0f, PI);
		b.poly({ { -0.72f, 0.3f }, { -0.95f, 0.95f }, { -0.35f, 0.45f }, { 0.0f, 0.95f }, { 0.35f, 0.45f },
			{ 0.95f, 0.95f }, { 0.72f, 0.3f } });
		b.arc(-0.3f, 0.0f, 0.16f, 0.08f, 6);
		b.arc(0.3f, 0.0f, 0.16f, 0.08f, 6);
		b.arc(0.0f, -0.25f, 0.36f, 0.22f, 8, PI * 1.1f, PI * 1.9f);
		break;

	case Glyph_Fang:
		b.arc(0.0f, 1.25f, 0.95f, 0.5f, 10, PI * 1.15f, PI * 1.85f);
		b.poly({ { -0.58f, 0.66f }, { -0.52f, -0.2f }, { -0.36f, -0.92f }, { -0.26f, -0.12f }, { -0.2f, 0.72f } });
		b.poly({ { 0.58f, 0.66f }, { 0.52f, -0.2f }, { 0.36f, -0.92f }, { 0.26f, -0.12f }, { 0.2f, 0.72f } });
		break;

	case Glyph_Scythe:
		b.poly({ { -0.25f, -1.0f }, { -0.1f, -0.2f }, { 0.05f, 0.5f }, { 0.1f, 0.9f } });
		b.arc(-0.45f, 0.45f, 0.62f, 0.5f, 12, 0.7f, PI * 1.1f);
		b.poly({ { -1.06f, 0.35f }, { -0.7f, 0.55f }, { -0.3f, 0.66f }, { 0.07f, 0.74f } });
		b.line({ -0.2f, -0.45f }, { 0.05f, -0.4f });
		break;

	case Glyph_Claw:
		for (float dx : { -0.45f, 0.0f, 0.45f })
		{
			b.poly({ { dx - 0.25f, 0.9f }, { dx - 0.05f, 0.3f }, { dx + 0.1f, -0.3f }, { dx + 0.15f, -0.9f } });
			b.poly({ { dx - 0.25f, 0.9f }, { dx + 0.05f, 0.35f }, { dx + 0.17f, -0.25f }, { dx + 0.15f, -0.9f } });
		}
		break;

	case Glyph_Lute:
		b.arc(0.0f, -0.35f, 0.5f, 0.55f, 14);
		b.arc(0.0f, -0.3f, 0.14f, 0.14f, 6);
		b.line({ -0.08f, 0.18f }, { -0.08f, 0.85f });
		b.line({ 0.08f, 0.18f }, { 0.08f, 0.85f });
		b.poly({ { -0.08f, 0.85f }, { -0.22f, 1.0f }, { 0.05f, 1.0f }, { 0.08f, 0.85f } });
		for (float x : { -0.04f, 0.0f, 0.04f })
			b.line({ x, 0.85f }, { x, -0.72f });
		b.line({ -0.18f, -0.72f }, { 0.18f, -0.72f });
		break;

	case Glyph_Drum:
	{
		b.arc(0.0f, 0.3f, 0.7f, 0.2f, 14);
		b.arc(0.0f, -0.6f, 0.7f, 0.2f, 8, PI, TAU);
		b.line({ -0.7f, 0.3f }, { -0.7f, -0.6f });
		b.line({ 0.7f, 0.3f }, { 0.7f, -0.6f });
		std::vector<P> lace;
		for (int i = 0; i <= 6; ++i)
		{
			const float x = -0.6f + 1.2f * i / 6.0f;
			lace.push_back({ x, (i % 2 == 0) ? 0.1f - 0.1f * std::fabs(x) : -0.75f + 0.1f * std::fabs(x) });
		}
		b.poly(lace);
		b.line({ -0.8f, 0.95f }, { 0.1f, 0.45f });
		b.line({ 0.8f, 0.95f }, { -0.1f, 0.45f });
		break;
	}

	case Glyph_Totem:
		b.poly({ { -0.35f, -1.0f }, { -0.35f, 0.7f }, { 0.35f, 0.7f }, { 0.35f, -1.0f } });
		b.poly({ { -0.35f, 0.55f }, { -0.9f, 0.8f }, { -0.35f, 0.3f } });
		b.poly({ { 0.35f, 0.55f }, { 0.9f, 0.8f }, { 0.35f, 0.3f } });
		b.poly({ { -0.2f, 0.7f }, { 0.0f, 0.95f }, { 0.2f, 0.7f } });
		for (float y : { 0.12f, -0.45f })
			b.line({ -0.35f, y }, { 0.35f, y });
		for (float y : { 0.42f, -0.15f, -0.72f })
		{
			b.arc(-0.15f, y, 0.08f, 0.06f, 4);
			b.arc(0.15f, y, 0.08f, 0.06f, 4);
			b.line({ -0.12f, y - 0.17f }, { 0.12f, y - 0.17f });
		}
		break;

	case Glyph_Spirit:
		b.arc(0.0f, 0.45f, 0.4f, 0.42f, 10, 0.0f, PI);
		b.poly({ { -0.4f, 0.45f }, { -0.4f, -0.3f }, { -0.25f, -0.55f }, { -0.1f, -0.3f }, { 0.05f, -0.62f },
			{ 0.2f, -0.3f }, { 0.4f, -0.55f }, { 0.4f, 0.45f } });
		b.arc(-0.15f, 0.45f, 0.07f, 0.1f, 4);
		b.arc(0.15f, 0.45f, 0.07f, 0.1f, 4);
		break;

	case Glyph_Snake:
	{
		auto at = [](float t, float side) {
			const float x = -0.9f + 1.5f * t;
			const float k = TAU * 1.1f;
			const float y = 0.35f * std::sin(t * k);
			const float dy = 0.35f * k * std::cos(t * k) / 1.5f;
			const float len = std::sqrt(1.0f + dy * dy);
			const float w = (0.03f + 0.09f * t) * side;
			return P{ x - dy / len * w, y + w / len };
		};
		std::vector<P> top, bot;
		for (int i = 0; i <= 14; ++i)
		{
			top.push_back(at(i / 14.0f, 1.0f));
			bot.push_back(at(i / 14.0f, -1.0f));
		}
		b.poly(top);
		b.poly(bot);
		const P head = at(1.0f, 0.0f);
		b.poly({ at(1.0f, 1.3f), { head.x + 0.25f, head.y + 0.02f }, at(1.0f, -1.3f) });
		b.poly({ { head.x + 0.25f, head.y + 0.02f }, { head.x + 0.4f, head.y + 0.08f } });
		b.poly({ { head.x + 0.25f, head.y + 0.02f }, { head.x + 0.4f, head.y - 0.04f } });
		break;
	}

	case Glyph_Insect:
		b.arc(0.0f, -0.25f, 0.24f, 0.45f, 10);
		b.arc(0.0f, 0.38f, 0.15f, 0.15f, 6);
		b.arc(-0.42f, 0.2f, 0.36f, 0.2f, 8);
		b.arc(0.42f, 0.2f, 0.36f, 0.2f, 8);
		for (float y : { -0.05f, -0.3f, -0.55f })
		{
			b.line({ -0.24f, y }, { -0.6f, y - 0.2f });
			b.line({ 0.24f, y }, { 0.6f, y - 0.2f });
		}
		b.line({ -0.08f, 0.5f }, { -0.25f, 0.85f });
		b.line({ 0.08f, 0.5f }, { 0.25f, 0.85f });
		for (float y : { -0.2f, -0.4f })
			b.line({ -0.22f, y }, { 0.22f, y });
		break;

	case Glyph_Mouth:
		b.arc(0.0f, 0.0f, 0.85f, 0.5f, 16);
		b.arc(0.0f, -0.05f, 0.6f, 0.3f, 12);
		for (float x : { -0.3f, -0.1f, 0.1f, 0.3f })
			b.line({ x, 0.24f }, { x, 0.12f });
		break;

	case Glyph_Bat:
		b.mirrorX({ { 0.06f, 0.12f }, { 0.3f, 0.3f }, { 0.6f, 0.36f }, { 0.95f, 0.55f }, { 0.86f, 0.2f },
			{ 0.7f, 0.04f }, { 0.56f, 0.14f }, { 0.44f, -0.06f }, { 0.3f, 0.04f }, { 0.16f, -0.2f },
			{ 0.06f, -0.22f } });
		b.arc(0.0f, -0.05f, 0.08f, 0.24f, 8);
		b.poly({ { -0.07f, 0.15f }, { -0.1f, 0.35f }, { -0.02f, 0.2f }, { 0.02f, 0.2f }, { 0.1f, 0.35f }, { 0.07f, 0.15f } });
		break;

	case Glyph_Clef:
		b.poly({ { 0.05f, -0.85f }, { -0.1f, -0.95f }, { -0.2f, -0.85f }, { -0.1f, -0.75f }, { 0.05f, -0.8f },
			{ 0.1f, -0.4f }, { 0.05f, 0.3f }, { 0.15f, 0.7f }, { 0.1f, 0.95f }, { -0.08f, 0.78f },
			{ -0.12f, 0.45f }, { 0.25f, 0.0f }, { 0.35f, -0.3f }, { 0.18f, -0.52f }, { -0.18f, -0.5f },
			{ -0.34f, -0.25f }, { -0.2f, 0.05f }, { 0.08f, 0.08f }, { 0.22f, -0.12f } });
		break;

	case Glyph_Wolf:
		b.poly({ { -0.6f, -0.9f }, { -0.5f, -0.2f }, { -0.6f, 0.3f }, { -0.42f, 0.75f }, { -0.3f, 0.95f },
			{ -0.15f, 0.6f }, { 0.1f, 0.55f }, { 0.45f, 0.35f }, { 0.9f, 0.25f }, { 0.85f, 0.05f },
			{ 0.45f, 0.0f }, { 0.6f, -0.15f }, { 0.3f, -0.3f }, { 0.1f, -0.4f }, { 0.2f, -0.9f } });
		b.arc(0.15f, 0.35f, 0.07f, 0.05f, 4);
		b.line({ -0.35f, 0.7f }, { -0.3f, 0.45f });
		break;

	case Glyph_Doll:
		b.arc(0.0f, 0.55f, 0.3f, 0.3f, 10);
		b.poly({ { -0.2f, 0.27f }, { -0.22f, -0.45f }, { 0.22f, -0.45f }, { 0.2f, 0.27f } });
		b.line({ -0.7f, 0.05f }, { 0.7f, 0.05f });
		b.line({ -0.15f, -0.45f }, { -0.35f, -0.95f });
		b.line({ 0.15f, -0.45f }, { 0.35f, -0.95f });
		b.line({ -0.18f, 0.62f }, { -0.06f, 0.5f });
		b.line({ -0.06f, 0.62f }, { -0.18f, 0.5f });
		b.line({ 0.06f, 0.62f }, { 0.18f, 0.5f });
		b.line({ 0.18f, 0.62f }, { 0.06f, 0.5f });
		b.line({ 0.55f, 0.45f }, { -0.1f, -0.15f });
		b.arc(0.6f, 0.5f, 0.06f, 0.06f, 4);
		b.line({ -0.55f, -0.3f }, { 0.1f, -0.05f });
		b.arc(-0.6f, -0.32f, 0.06f, 0.06f, 4);
		break;

	default:
		break;
	}
}

struct Table
{
	std::vector<Seg>      segs;
	std::vector<uint32_t> start, count;
	std::string           hlsl;
};

const Table& GetTable()
{
	static const Table table = [] {
		Table t;
		for (uint32_t id = 0; id < Glyph_Count; ++id)
		{
			Builder b;
			DrawGlyph(id, b);
			if (b.segs.size() > kMaxSegsPerInstance)
				b.segs.resize(kMaxSegsPerInstance);
			t.start.push_back(static_cast<uint32_t>(t.segs.size()));
			t.count.push_back(static_cast<uint32_t>(b.segs.size()));
			t.segs.insert(t.segs.end(), b.segs.begin(), b.segs.end());
		}

		std::string& h = t.hlsl;
		char buf[128];
		std::snprintf(buf, sizeof(buf), "#define GLYPH_COUNT %uu\nstatic const float4 GSEG[%zu] = {\n",
			static_cast<unsigned>(Glyph_Count), t.segs.size());
		h += buf;
		for (const Seg& s : t.segs)
		{
			std::snprintf(buf, sizeof(buf), "float4(%.4f,%.4f,%.4f,%.4f),\n", s.x0, s.y0, s.x1, s.y1);
			h += buf;
		}
		h += "};\n";
		std::snprintf(buf, sizeof(buf), "static const uint2 GRANGE[%u] = {\n", static_cast<unsigned>(Glyph_Count));
		h += buf;
		for (uint32_t id = 0; id < Glyph_Count; ++id)
		{
			std::snprintf(buf, sizeof(buf), "uint2(%uu,%uu),\n", t.start[id], t.count[id]);
			h += buf;
		}
		h += "};\n";
		return t;
	}();
	return table;
}

} // namespace

const char* GlyphName(uint32_t id)
{
	static const char* const kNames[Glyph_Count] = {
		"cross", "ankh", "heart", "skull", "eye", "star", "z", "moon", "sun", "hourglass",
		"shield", "lightning", "note", "paw", "snowflake", "leaf", "skeleton", "sword",
		"feather", "link", "spiral", "drop", "arrowup", "arrowdown", "flame", "pentagram",
		"rock", "wave", "tornado", "hand", "hammer", "scales", "crown", "chalice", "book",
		"steed", "dove", "anvil", "banner", "shackle", "tower", "fist", "lantern", "helm",
		"horn", "tree", "fleurdelis", "cog", "coin", "door", "gem", "mask", "fang", "scythe",
		"claw", "lute", "drum", "totem", "spirit", "snake", "insect", "mouth", "bat", "clef",
		"wolf", "doll",
	};
	return id < Glyph_Count ? kNames[id] : "?";
}

uint32_t GlyphSegments(uint32_t id)
{
	const Table& t = GetTable();
	return id < Glyph_Count ? t.count[id] : 0;
}

const std::string& GlyphHlsl()
{
	return GetTable().hlsl;
}

} // namespace eqgfx::gpu
