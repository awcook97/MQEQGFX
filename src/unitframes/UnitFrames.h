#pragma once

#include <mq/Plugin.h>

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace eqgfx::unitframes {

enum class Window
{
	Player,
	Target,
	Focus,
	Group,
	Raid,
	Pet,
	XTarget,

	Count,
};

constexpr int kWindowCount = static_cast<int>(Window::Count);

enum class Action
{
	Hotkey,
	Left,
	Right,

	Count,
};

constexpr int kActionCount = static_cast<int>(Action::Count);
constexpr int kMaxRotationGems = 8;

constexpr const char* kSettingsSection = "UnitFrames";

namespace Anon
{
enum Mode { Off, ClassName, ClassShort, Scramble, Stars, Stars8, FirstLast, Count };
}
extern const char* const kAnonLabels[Anon::Count];

namespace Align
{
enum Mode { Left, Center, Right, Count };
}
extern const char* const kAlignLabels[Align::Count];

namespace HpText
{
enum Mode { Off, Percent, Current, CurrentMax, Deficit, Count };
}
extern const char* const kHpTextLabels[HpText::Count];

namespace BarColor
{
enum Mode { Health, Class, Fixed, Con, Count };
}
extern const char* const kBarColorLabels[BarColor::Count];

namespace BarFill
{
enum Mode { Flat, Gradient, Glass, Stripes, Segments, Count };
}
extern const char* const kBarFillLabels[BarFill::Count];

namespace BuffFilter
{
enum Mode { All, Beneficial, Detrimental, Count };
}
extern const char* const kBuffFilterLabels[BuffFilter::Count];

namespace Sort
{
enum Mode { None, Distance, HealthLow, HealthHigh, Name, Class, Count };
}
extern const char* const kSortLabels[Sort::Count];

namespace Layout
{
enum Mode { Rows, Columns, Count };
}
extern const char* const kLayoutLabels[Layout::Count];

namespace CastText
{
enum Mode { Off, Inside, Above, Below, Count };
}
extern const char* const kCastTextLabels[CastText::Count];

namespace TextEdge
{
enum Mode { None, Shadow, Outline, Count };
}
extern const char* const kTextEdgeLabels[TextEdge::Count];

struct SettingDef
{
	enum class Kind { Header, Bool, Int, Float, Color, Enum };

	Kind               kind = Kind::Bool;
	const char*        key = nullptr;
	const char*        label = nullptr;
	const char*        tip = nullptr;
	void*              ptr = nullptr;
	float              lo = 0.0f;
	float              hi = 1.0f;
	const char* const* names = nullptr;
	int                nameCount = 0;
	bool               sameLine = false;
	const char*        fmt = nullptr;
};

class SettingsPanel
{
public:
	void Header(const char* label);
	void Bool(const char* key, const char* label, bool* v, const char* tip = nullptr,
		bool sameLine = false);
	void Int(const char* key, const char* label, int* v, int lo, int hi,
		const char* tip = nullptr);
	void Float(const char* key, const char* label, float* v, float lo, float hi,
		const char* tip = nullptr, const char* fmt = "%.1f");
	void Color(const char* key, const char* label, ImVec4* v, const char* tip = nullptr,
		bool sameLine = false);
	void Enum(const char* key, const char* label, int* v, const char* const* names,
		int count, const char* tip = nullptr);

	bool Draw(const char* filter);

	void Load(const char* prefix);
	void Save(const char* prefix) const;

	bool empty() const { return m_defs.empty(); }

private:
	std::vector<SettingDef> m_defs;
};

struct Rotation
{
	int gems[kMaxRotationGems] = { 0 };
	int count = 0;

	bool empty() const { return count == 0; }
	void clear() { count = 0; }
};

struct FrameColors
{
	ImVec4 bg            { 0.08f, 0.08f, 0.10f, 0.85f };
	ImVec4 hpHigh        { 0.16f, 0.85f, 0.16f, 0.90f };
	ImVec4 hpMid         { 0.95f, 0.85f, 0.16f, 0.90f };
	ImVec4 hpLow         { 0.85f, 0.16f, 0.16f, 0.90f };
	ImVec4 hpFixed       { 0.20f, 0.70f, 0.25f, 0.90f };
	ImVec4 mana          { 0.25f, 0.45f, 0.95f, 0.90f };
	ImVec4 endurance     { 0.90f, 0.75f, 0.20f, 0.90f };
	ImVec4 petHp         { 0.25f, 0.80f, 0.30f, 0.90f };
	ImVec4 auxBack       { 0.00f, 0.00f, 0.00f, 0.55f };
	ImVec4 name          { 1.00f, 1.00f, 1.00f, 0.95f };
	ImVec4 nameShadow    { 0.00f, 0.00f, 0.00f, 0.90f };
	ImVec4 hpTextColor   { 1.00f, 1.00f, 1.00f, 0.90f };
	ImVec4 castFill      { 0.95f, 0.72f, 0.18f, 0.95f };
	ImVec4 castBack      { 0.00f, 0.00f, 0.00f, 0.60f };
	ImVec4 castText      { 1.00f, 1.00f, 1.00f, 0.90f };
	ImVec4 hoverBorder   { 1.00f, 1.00f, 1.00f, 0.95f };
	ImVec4 targetBorder  { 1.00f, 0.85f, 0.25f, 0.90f };
	ImVec4 focusBorder   { 0.85f, 0.35f, 1.00f, 0.95f };
	ImVec4 idleBorder    { 0.00f, 0.00f, 0.00f, 0.60f };
	ImVec4 flash         { 1.00f, 1.00f, 1.00f, 0.65f };
	ImVec4 lowHp         { 1.00f, 0.10f, 0.10f, 0.90f };
	ImVec4 dead          { 0.30f, 0.06f, 0.06f, 0.85f };
	ImVec4 aggroHigh     { 1.00f, 0.35f, 0.35f, 1.00f };
	ImVec4 aggroMid      { 1.00f, 0.75f, 0.30f, 1.00f };
	ImVec4 aggroLow      { 0.85f, 0.85f, 0.85f, 0.90f };
	ImVec4 buffBorderBen { 0.10f, 0.90f, 0.10f, 0.90f };
	ImVec4 buffBorderDet { 1.00f, 0.15f, 0.15f, 0.90f };
	ImVec4 buffBorderMine{ 1.00f, 0.85f, 0.20f, 1.00f };
	ImVec4 tag           { 0.70f, 0.85f, 1.00f, 1.00f };
};

struct BarStyle
{
	float height          = 18.0f;
	float auxHeight       = 4.0f;
	float auxGap          = 1.0f;
	float rounding        = 3.0f;
	float rowSpacing      = 2.0f;
	float classStrip      = 4.0f;
	float inset           = 1.0f;
	float auxInset        = 5.0f;
	int   colorMode       = BarColor::Health;
	int   fill            = BarFill::Flat;
	bool  reverse         = false;
	float borderThickness = 1.5f;
	bool  borderAlways    = false;
	bool  hpBackdrop      = true;
};

struct TextStyle
{
	bool  name          = true;
	int   nameAlign     = Align::Left;
	int   edge          = TextEdge::Shadow;
	float edgeOffset    = 1.0f;

	bool  pixelSnap     = true;
	bool  classColorName = false;
	bool  conColorName  = false;
	float scale         = 1.0f;
	int   hpText        = HpText::Percent;
	int   hpAlign       = Align::Right;
	bool  showClass     = false;
	bool  showLevel     = false;
	bool  showDistance  = false;
	bool  showGuild     = false;
	bool  showTag       = true;
	int   anonMode      = Anon::Off;
	bool  anonNPCs      = false;
	int   maxNameChars  = 0;
};

struct CastBarStyle
{
	bool  show       = true;
	float height     = 8.0f;
	bool  spellName  = true;
	bool  timeText   = false;
	bool  onlyTarget = false;
	bool  overlay    = true;
	int   textPlace  = CastText::Inside;
	float textScale  = 0.85f;
};

struct BuffStyle
{
	bool  show      = false;
	float iconSize  = 18.0f;
	float spacing   = 2.0f;
	int   maxIcons  = 20;
	int   perRow    = 8;
	bool  borders   = true;
	bool  tooltip   = true;
	bool  timers    = false;
	bool  mineOnly  = false;
	bool  dimOthers = false;
	int   filter    = BuffFilter::All;

	bool  petFromWindow = true;
	int   refreshMs = 250;
};

struct TooltipStyle
{
	bool enabled    = true;
	bool name       = true;
	bool realName   = true;
	bool classLevel = true;
	bool race       = true;
	bool con        = true;
	bool hp         = true;
	bool distance   = true;
	bool guild      = true;
	bool aggro      = true;
	bool pet        = true;
	bool targetOf   = true;
	bool role       = true;
	bool casting    = true;
	bool binds      = false;
};

struct AnimStyle
{
	bool  hpSmooth       = true;

	bool  useImAnim      = true;
	float hpSpeed        = 10.0f;
	bool  damageFlash    = true;
	float flashThreshold = 0.03f;
	float flashDur       = 0.30f;
	bool  healFlash      = false;
	bool  lowHpPulse     = true;
	float lowHpThreshold = 0.30f;
	float lowHpSpeed     = 1.5f;
	bool  targetGlow     = false;
	float glowSpeed      = 1.0f;
};

struct WindowSettings
{
	bool  show          = false;
	bool  locked        = false;
	bool  noTitleBar    = false;
	bool  noBackground  = false;
	bool  scrollbar     = false;
	float opacity       = 0.85f;

	bool  autoResize    = true;
	float width         = 280.0f;
	float height        = 120.0f;

	bool  hideWhenEmpty = false;

	bool  clickThrough  = true;
	float hitPad        = 2.0f;

	bool  showHitRects  = false;

	BarStyle     bar;
	TextStyle    text;
	CastBarStyle cast;
	BuffStyle    buffs;
	TooltipStyle tip;
	AnimStyle    anim;
	FrameColors  colors;

	Rotation binds[kActionCount];
};

struct RowOptions
{
	bool        mana = false;
	bool        endurance = false;
	bool        pet = false;
	bool        aggro = false;
	bool        buffs = false;
	bool        castBar = true;
	int         aggroPct = -1;
	const char* tag = nullptr;
	const char* role = nullptr;
	float       height = 0.0f;
	bool        nameOnly = false;
};

struct RowResult
{
	bool hovered = false;
	bool menuRequested = false;
};

const char* WindowName(Window w);
const char* ActionName(Action a);
bool        ParseWindow(const char* name, Window& out);
bool        ParseAction(const char* name, Action& out);

std::string FormatRotation(const Rotation& r);
bool        ParseRotation(const char* text, Rotation& out);

int  PickRotationGem(const Rotation& r);

bool IsGemReady(int gem);

bool FireRotation(const Rotation& r, int spawnId);

int  HoverWindowIndex();
void SetHoverWindow(Window w);
void ClearHoverWindow();

using MenuItems = std::function<void(PlayerClient*)>;

RowResult DrawUnitRow(PlayerClient* sp, Window src, const RowOptions& opts, float width,
	const MenuItems& extra = nullptr);

void DrawBuffIcons(PlayerClient* sp, const WindowSettings& cfg);

ImVec2 MousePos();

void                       BeginRowCapture(Window w);
const std::vector<ImVec4>& RowRects(Window w);

void NoteHotRect(Window w, const ImVec2& lo, const ImVec2& hi);

void NoteHotItem(Window w);

int MenuOpenWindow();

void DebugDrawHotRects(Window w, const ImVec4& colour);

bool ShouldDraw();

bool OnlyWhenForeground();
void SetOnlyWhenForeground(bool on);

std::string DisplayName(const PlayerClient* sp, const TextStyle& text);

float PctHP(const PlayerClient* sp);
float PctMana(const PlayerClient* sp);
float PctEndurance(const PlayerClient* sp);

ImU32       ClassColor(const PlayerClient* sp, float alpha = 1.0f);
ImU32       ConColorOf(const PlayerClient* sp, float alpha = 1.0f);
const char* ClassShort(const PlayerClient* sp);

int GroupAggroPct(int groupIndex);
int XTargetAggroPct(int slotIndex);

CGroupMember* GroupMemberFor(const PlayerClient* sp);
void GroupRoleItems(CGroupMember* member);

void ForgetUnits();
void ForgetUnit(int spawnId);
void ApplyForgets();

PlayerClient* ValidateSpawn(PlayerClient* sp);

void ShutdownIcons();

void BeginAnimContext();
void EndAnimContext();

void DrawSharedOptions(Window w, const char* filter);

void BuildSharedPanel(SettingsPanel& panel, WindowSettings& cfg);

void ShowPluginOptions(bool on);
bool PluginOptionsShown();
void DrawPluginOptions();

void Draw();

void Pulse();

bool Command(const char* szCmd, const char* szLine);

void PrintHelp();

void LoadSettings();

bool        SettingGetBool(const char* key, bool fallback);
int         SettingGetInt(const char* key, int fallback);
float       SettingGetFloat(const char* key, float fallback);
std::string SettingGetString(const char* key, const char* fallback);

void SettingPutBool(const char* key, bool value);
void SettingPutInt(const char* key, int value);
void SettingPutFloat(const char* key, float value);
void SettingPutString(const char* key, const char* value);

void SaveSettings();
void MarkSettingsDirty();
void FlushSettings();

void ShutdownWindows();

} // namespace eqgfx::unitframes
