#include <iostream>
#include "includes.h"
#include "menu.h"
#include "FNTool.h"
#include "spoof_call.h"

AFortPawn* AcknowledgedPawn;

#include "detours.h"

#pragma comment(lib, "detours.lib")

//#include "SpeedHack.h"
// TODO: put in another file, and rename to something better
template<class DataType>
class SpeedHack {
	DataType time_offset;
	DataType time_last_update;

	double speed_;

public:
	SpeedHack(DataType currentRealTime, double initialSpeed) {
		time_offset = currentRealTime;
		time_last_update = currentRealTime;

		speed_ = initialSpeed;
	}

	// TODO: put lock around for thread safety
	void setSpeed(DataType currentRealTime, double speed) {
		time_offset = getCurrentTime(currentRealTime);
		time_last_update = currentRealTime;

		speed_ = speed;
	}

	// TODO: put lock around for thread safety
	DataType getCurrentTime(DataType currentRealTime) {
		DataType difference = currentRealTime - time_last_update;

		return (DataType)(speed_ * difference) + time_offset;
	}
};


// function signature typedefs
typedef DWORD(WINAPI* GetTickCountType)(void);
typedef ULONGLONG(WINAPI* GetTickCount64Type)(void);

typedef BOOL(WINAPI* QueryPerformanceCounterType)(LARGE_INTEGER* lpPerformanceCount);

// globals
GetTickCountType   g_GetTickCountOriginal;
GetTickCount64Type g_GetTickCount64Original;
GetTickCountType   g_TimeGetTimeOriginal;    // Same function signature as GetTickCount

QueryPerformanceCounterType g_QueryPerformanceCounterOriginal;


const double kInitialSpeed = 1.0; // initial speed hack speed

//                                  (initialTime,      initialSpeed)
SpeedHack<DWORD>     g_speedHack(GetTickCount(), kInitialSpeed);
SpeedHack<ULONGLONG> g_speedHackULL(GetTickCount64(), kInitialSpeed);
SpeedHack<LONGLONG>  g_speedHackLL(0, kInitialSpeed); // Gets set properly in DllMain

// function prototypes

DWORD     WINAPI GetTickCountHacked(void);
ULONGLONG WINAPI GetTickCount64Hacked(void);

BOOL      WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount);

DWORD     WINAPI KeysThread(LPVOID lpThreadParameter);

// functions

void setAllToSpeed(double speed) {
	g_speedHack.setSpeed(g_GetTickCountOriginal(), speed);

	g_speedHackULL.setSpeed(g_GetTickCount64Original(), speed);

	LARGE_INTEGER performanceCounter;
	g_QueryPerformanceCounterOriginal(&performanceCounter);

	g_speedHackLL.setSpeed(performanceCounter.QuadPart, speed);
}

DWORD WINAPI GetTickCountHacked(void) {
	return g_speedHack.getCurrentTime(g_GetTickCountOriginal());
}

ULONGLONG WINAPI GetTickCount64Hacked(void) {
	return g_speedHackULL.getCurrentTime(g_GetTickCount64Original());
}

BOOL WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount) {
	LARGE_INTEGER performanceCounter;

	BOOL result = g_QueryPerformanceCounterOriginal(&performanceCounter);

	lpPerformanceCount->QuadPart = g_speedHackLL.getCurrentTime(performanceCounter.QuadPart);

	return result;
}

void DrawMenu() {
	ZeroGUI::SetupCanvas(static_canvas);
	ZeroGUI::Input::Handle();

	static int tab = 1;
	static Vector2 pos = Vector2(300, 300);
	Vector2 size = Vector2(600, 475);
	if (tab == 3)
	{
		size = Vector2(600, 900);
	}

	if (ZeroGUI::Window(xorstr("Mokobake Modded (INSERT) Mokobake Backup#3757"), &pos, size, ShowMenu))
	{
		static bool text_check = false;
		ZeroGUI::Text(xorstr(" "));
		ZeroGUI::NextColumn(10.0f);
		if (ZeroGUI::ButtonTab(xorstr("Aimbot"), Vector2{ 135, 25 }, tab == 1)) tab = 1;
		ZeroGUI::NextColumn(155.0f);
		if (ZeroGUI::ButtonTab(xorstr("Visuals"), Vector2{ 135, 25 }, tab == 2)) tab = 2;
		ZeroGUI::NextColumn(300.0f);
		if (ZeroGUI::ButtonTab(xorstr("Mods"), Vector2{ 135, 25 }, tab == 3)) tab = 3;
		ZeroGUI::NextColumn(445.0f);
		if (ZeroGUI::ButtonTab(xorstr("Misc"), Vector2{ 135, 25 }, tab == 4)) tab = 4;

		if (tab == 1)
		{
			ZeroGUI::NextColumn(18.0f);
			ZeroGUI::Text(xorstr(" "));
			ZeroGUI::Text(xorstr("Aimbot Mode"));
			ZeroGUI::Checkbox(xorstr("Mouse Aim"), &Settings::mouseaim);
			if (Settings::mouseaim)
			{
				Settings::MemoryAim = false;
				Settings::SilentAim = false;
			}
			ZeroGUI::Checkbox(xorstr("Memory Aim"), &Settings::MemoryAim);
			if (Settings::MemoryAim)
			{
				Settings::mouseaim = false;
				Settings::SilentAim = false;
			}
			ZeroGUI::Checkbox(xorstr("Silent Aim"), &Settings::SilentAim);
			if (Settings::SilentAim)
			{
				Settings::mouseaim = false;
				Settings::MemoryAim = false;
			}
			ZeroGUI::Text(xorstr("Aim Bone"));
			ZeroGUI::Checkbox(xorstr("Head"), &Settings::head);
			if (Settings::head == true)
			{
				Settings::AimBone = 68;
				Settings::chest = false;
				Settings::pelvis = false;
			}
			if (Settings::AimBone == 68)
			{
				Settings::head = true;
			}
			ZeroGUI::Checkbox(xorstr("Chest"), &Settings::chest);
			if (Settings::chest == true)
			{
				Settings::AimBone = 6;
				Settings::head = false;
				Settings::pelvis = false;
			}
			if (Settings::AimBone == 6)
			{
				Settings::chest = true;
			}
			ZeroGUI::Checkbox(xorstr("Pelvis"), &Settings::pelvis);
			if (Settings::pelvis == true)
			{
				Settings::AimBone = 2;
				Settings::head = false;
				Settings::chest = false;
			}
			if (Settings::AimBone == 2)
			{
				Settings::pelvis = true;
			}
			ZeroGUI::Checkbox(xorstr("Fov Circle"), &Settings::FovCircle);
			ZeroGUI::SliderFloat(xorstr("Fov:"), &Settings::FovSize, 5.0f, 3400.f);
			ZeroGUI::slidermouse(xorstr("Mouse Aim Smooth:"), &Settings::mousesmooth, 1, 50);
			if (Settings::SilentAim)
			{
				ZeroGUI::Checkbox(xorstr("Spinbot"), &Settings::spinbot);
			}
		}
		if (tab == 2)
		{
			ZeroGUI::NextColumn(18.0f);
			ZeroGUI::Text(xorstr(" "));
			ZeroGUI::Checkbox(xorstr("Box"), &Settings::BoxESP);
			if (Settings::BoxESP)
			{
				Settings::box = false;
			}
			ZeroGUI::Checkbox(xorstr("Corner Box"), &Settings::box);
			if (Settings::box)
			{
				Settings::BoxESP = false;
			}
			ZeroGUI::Checkbox(xorstr("Skeleton"), &Settings::SkeletonESP);
			ZeroGUI::Checkbox(xorstr("Snaplines"), &Settings::LinesESP);
			ZeroGUI::Checkbox(xorstr("PlayerName"), &Settings::PlayerNameESP);
			ZeroGUI::Checkbox(xorstr("Player Distance"), &Settings::DistanceESP);
			ZeroGUI::Checkbox(xorstr("Platform"), &Settings::PlatformESP);
			ZeroGUI::Checkbox(xorstr("Weapon/Active item"), &Settings::EnemyWeaponESP);

			ZeroGUI::NextColumn(200);
			ZeroGUI::Text(xorstr(" "));
			ZeroGUI::Checkbox(xorstr("Chest"), &Settings::chestesp);
			ZeroGUI::Checkbox(xorstr("AmmoBox"), &Settings::ammobox);
			ZeroGUI::Checkbox(xorstr("Vehicle"), &Settings::vehicleespnerds);
			ZeroGUI::Checkbox(xorstr("Loot"), &Settings::LootESP);
			ZeroGUI::Checkbox(xorstr("Projectile"), &Settings::ProjectileESP);
			ZeroGUI::Checkbox(xorstr("Projectile ObjectName"), &Settings::ProjectileObjectNameESP);
			ZeroGUI::Checkbox(xorstr("DMR Seller"), &Settings::DMRSeller);
			ZeroGUI::Checkbox(xorstr("Actor ObjectName (10m)"), &Settings::ActorObjectName);
		}
		if (tab == 3)
		{
			ZeroGUI::NextColumn(18.0f);
			ZeroGUI::Text(xorstr(" "));
			ZeroGUI::Checkbox(xorstr("Fov Changer"), &Settings::FovChanger);
			ZeroGUI::SliderFloat(xorstr("Fov Amount"), &Settings::FovChanger_Value, 60, 150);
			ZeroGUI::Checkbox(xorstr("Projectile Bullet Teleport"), &Settings::SniperTp);
			ZeroGUI::Checkbox(xorstr("Rapid Fire"), &Settings::RapidFire);
			ZeroGUI::Checkbox(xorstr("Full Auto Weapons"), &Settings::FullAutoWeapons);
			ZeroGUI::Checkbox(xorstr("CurrentWeapon Speed"), &Settings::CurrentWeaponSpeed);
			ZeroGUI::Checkbox(xorstr("No Recoil"), &Settings::NoRecoil);
			ZeroGUI::Checkbox(xorstr("Instant Reload"), &Settings::InstantReload);
			ZeroGUI::Checkbox(xorstr("No Weapon CoolDown"), &Settings::NoWeaponCoolDown);
			ZeroGUI::Checkbox(xorstr("No Spread TEST"), &Settings::NoSpread);
			ZeroGUI::Checkbox(xorstr("Projectile Destroyer (F1)"), &Settings::ProjectileDestroyer);
			ZeroGUI::Checkbox(xorstr("Magic Bullet (In Vehicle) TEST"), &Settings::MagicBullet);
			ZeroGUI::Checkbox(xorstr("Vehicle Fly (and recommend to use Vehicle No Gravity)"), &Settings::VehicleFly);
			ZeroGUI::SliderFloat(xorstr("Vehicle Fly Speed"), &Settings::VehicleFlySpeed, 0.1f, 1000.0f);
			ZeroGUI::Checkbox(xorstr("Sync Camera Rotation"), &Settings::SyncCameraRotation);
			ZeroGUI::Checkbox(xorstr("Vehicle TP (SHIFT)"), &Settings::VehicleTP);
			ZeroGUI::Checkbox(xorstr("Vehicle Map Marker To Teleport (ENTER)"), &Settings::VehicleMapMarkerToTeleport);
			ZeroGUI::Checkbox(xorstr("Vehicle No Gravity"), &Settings::VehicleNoGravity);
			ZeroGUI::Checkbox(xorstr("Player Fly (Initialization Key: RIGHTCONTROL)"), &Settings::PlayerFly);
			ZeroGUI::SliderFloat(xorstr("Player Fly Speed"), &Settings::PlayerFlySpeed, 0.1f, 1000.0f);
			ZeroGUI::Checkbox(xorstr("Disable Get Current Location (Bypass Key: RIGHTMOUSE)"), &Settings::DisableGetCurrentLocation);
			ZeroGUI::Checkbox(xorstr("Zipline Teleport (F1, F2, F3, F5, F6)"), &Settings::ZiplineTeleport);
			ZeroGUI::Checkbox(xorstr("LocalPawn Speed (F)"), &Settings::LocalPawnSpeed);
			ZeroGUI::Checkbox(xorstr("Quick Fly (RIGHTSHIFT)"), &Settings::QuickFly);
			ZeroGUI::Checkbox(xorstr("Speedhack (F)"), &Settings::Speedhack);
			ZeroGUI::SliderFloat(xorstr("Speedhack Value"), &Settings::SpeedhackValue, 0.01, 50);
			ZeroGUI::Checkbox(xorstr("Unreal Engine Speedhack (F)"), &Settings::UnrealEngineSpeedhack);
			ZeroGUI::SliderFloat(xorstr("Unreal Engine Speedhack Value"), &Settings::UnrealEngineSpeedhackValue, 0.01, 50);
		}	
		if (tab == 4)
		{
			ZeroGUI::NextColumn(18.0f);
			ZeroGUI::Text(xorstr(" "));
			ZeroGUI::Text(xorstr("DTC Pasta"));
			ZeroGUI::Text(xorstr("Discord: discord.gg/eGdRsEv2G8"));
			ZeroGUI::Text(xorstr("Super Thanks: ritz, Tuyome, Blueberry, UnknownCheats, Void, Epic Games"));
		}
	}
	ZeroGUI::Draw_Cursor(true);
	ZeroGUI::Render();
}




static BOOL IsNotInScreen(int SizeX, int SizeY, Vector2 Pos) {
	if (((Pos.x <= 0 or Pos.x > SizeX) and (Pos.y <= 0 or Pos.y > SizeY)) or ((Pos.x <= 0 or Pos.x > SizeX) or (Pos.y <= 0 or Pos.y > SizeY))) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

AFortPawn* TargetPawno;
AFortPawn* LocalPawno;
APlayerCameraManager* PlayerCameraManagero;
Vector3 o_CamRot;
Vector3 o_CamLoc;
uintptr_t GlobalPlayerState;
bool initspeed = true;
bool Debuggytest = true;
ue::FString playeraname;
void __forceinline AnsiToWide(char* inAnsi, wchar_t* outWide)
{
	int i = 0;
	for (; inAnsi[i] != '\0'; i++)
		outWide[i] = (wchar_t)(inAnsi)[i];
	outWide[i] = L'\0';
}
#include <cmath>
float color_red;
float color_blue;
float color_green;
float color_speed = -10.0;
static ue::FLinearColor FromHSB(float hue, float saturation, float brightness)
{
	float h = hue == 1.0f ? 0 : hue * 6.0f;
	float f = h - (int)h;
	float p = brightness * (1.0f - saturation);
	float q = brightness * (1.0f - saturation * f);
	float t = brightness * (1.0f - (saturation * (1.0f - f)));

	if (h < 1)
	{
		return ue::FLinearColor(
			(float)(brightness),
			(float)(t),
			(float)(p),
			(float)(0.7f)
		);
	}
	else if (h < 2)
	{
		return ue::FLinearColor(
			(float)(q),
			(float)(brightness),
			(float)(p),
			(float)(0.7f)
		);
	}
	else if (h < 3)
	{
		return ue::FLinearColor(
			(float)(p),
			(float)(brightness),
			(float)(t),
			(float)(0.7f)
		);
	}
	else if (h < 4)
	{
		return ue::FLinearColor(
			(float)(p),
			(float)(q),
			(float)(brightness),
			(float)(0.7f)
		);
	}
	else if (h < 5)
	{
		return ue::FLinearColor(
			(float)(t),
			(float)(p),
			(float)(brightness),
			(float)(0.7f)
		);
	}
	else
	{
		return ue::FLinearColor(
			(float)(brightness),
			(float)(p),
			(float)(q),
			(float)(0.7f)
		);
	}
}
ue::FLinearColor GetItemColor(EFortItemTier tier)
{
	if (tier == EFortItemTier::I)
		return ue::FLinearColor(123 / 255.0f, 123 / 255.0f, 123 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::II)
		return ue::FLinearColor(58 / 255.0f, 121 / 255.0f, 19 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::III)
		return ue::FLinearColor(18 / 255.0f, 88 / 255.0f, 162 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::IV)
		return ue::FLinearColor(189 / 255.0f, 63 / 255.0f, 250 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::V)
		return ue::FLinearColor(255 / 255.0f, 118 / 255.0f, 5 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::VI)
		return ue::FLinearColor(220 / 255.0f, 160 / 255.0f, 30 / 255.0f, 0.95f);
	else if (tier == EFortItemTier::VII)
		return ue::FLinearColor(0 / 255.0f, 225 / 255.0f, 252 / 255.0f, 0.95f);

	return ue::FLinearColor(123, 123, 123, 0.95f);
}

EFortItemTier GetItemEFortTier(EFortItemTier tier)
{
	int tierI = (int)tier;
	switch (tierI)
	{
	case 1:
		return EFortItemTier::I;
	case 2:
		return EFortItemTier::II;
	case 3:
		return EFortItemTier::III;
	case 4:
		return EFortItemTier::IV;
	case 5:
		return EFortItemTier::V;
	case 6:
		return EFortItemTier::VI;
	case 7:
		return EFortItemTier::VII;
	default:
		return EFortItemTier::I;
	}
}

std::string GetItemTierName(EFortItemTier tier)
{
	switch (tier)
	{
	case EFortItemTier::I:
		return xorstr("Common");
	case EFortItemTier::II:
		return xorstr("Uncommon");
	case EFortItemTier::III:
		return xorstr("Rare");
	case EFortItemTier::IV:
		return xorstr("Epic");
	case EFortItemTier::V:
		return xorstr("Legendary");
	case EFortItemTier::VI:
		return xorstr("Mythic");
	case EFortItemTier::VII:
		return xorstr("Exotic");
	default:
		return xorstr("Common");
	}
}

static void freenamenerds(__int64 address)
{
	auto func = reinterpret_cast<__int64(__fastcall*)(__int64 a1)>(ue::cached::signatures::FreeFn);

	(func, address);
}

static const char* getwierdnames(uintptr_t Object)
{
	if (Object == NULL)
		return ("");

	auto fGetObjName = reinterpret_cast<ue::FString * (__fastcall*)(int* index, ue::FString * res)>(ue::cached::signatures::GetNameByIndex);

	int index = *(int*)(Object + 0x18);

	ue::FString result;
	(fGetObjName, &index, &result);

	if (result.c_str() == NULL)
		return ("");

	auto result_str = result.ToString();

	if (result.c_str() != NULL)
		freenamenerds((__int64)result.c_str());

	return result_str.c_str();
}


void DrawFilledRect(UCanvas* pCanvas, Vector2 initial_pos, float width, float height, ue::FLinearColor color)
{
	for (float i = 0.0f; i < height; i += 1.0f)
		pCanvas->K2_DrawLine(Vector2(initial_pos.x, initial_pos.y + i), Vector2(initial_pos.x + width, initial_pos.y + i), 1.0f, color);

}
#define mpe 3.14159265358979323846264338327950288419716939937510582f

void DrawCircle(UCanvas* canvas, int x, int y, int radius, int numsides, ue::FLinearColor color)
{
	float Step = mpe * 2.0 / numsides;
	int Count = 0;
	Vector2 V[128];
	for (float a = 0; a < mpe * 2.0; a += Step)
	{
		float X1 = radius * sdkUssage->STATIC_Cos(a) + x;
		float Y1 = radius * sdkUssage->STATIC_Sin(a) + y;
		float X2 = radius * sdkUssage->STATIC_Cos(a + Step) + x;
		float Y2 = radius * sdkUssage->STATIC_Sin(a + Step) + y;
		V[Count].x = X1;
		V[Count].y = Y1;
		V[Count + 1].x = X2;
		V[Count + 1].y = Y2;
		canvas->K2_DrawLine(Vector2({ V[Count].x, V[Count].y }), Vector2({ X2, Y2 }), 1.5f, color);
	}
}

void DrawTriangle(UCanvas* pCanvas, const Vector2& point1, const Vector2& point2, const Vector2& point3, const ue::FLinearColor& color, bool isFilled)
{
	pCanvas->K2_DrawLine(point1, point2, 1.0f, color);
	pCanvas->K2_DrawLine(point1, point3, 1.0f, color);
	pCanvas->K2_DrawLine(point2, point3, 1.0f, color);
	// Fill it
	if (isFilled) {
		float addX = 0.5f;
		float addY = 1;
		for (float i = point1.y; i < point3.y; i++) {
			// draw line
			pCanvas->K2_DrawLine(Vector2(point1.x + addX, point1.y + addY), Vector2(point2.x - addX, point2.y + addY), 1.0f, color);
			addY++;
			addX += 0.5f;
		}
	}
}
ue::FLinearColor getColorFromTier(BYTE Tier) {
	ue::FLinearColor color = { 255.f, 255.f, 255.f, 1 };

	if (Tier == 1)
		color = { 255.f, 255.f, 255.f, 1 };
	else if (Tier == 2)
		color = { 0.0f, 255.f, 0.0f, 1 };
	else if (Tier == 3)
		color = { 0.f, 1.f, 1.f, 1 };
	else if (Tier == 4)
		color = { 0.8f, 0.f, 0.8f, 1.f };
	else if (Tier == 5)
		color = { 1.f, 1.f, 0.f, 1 };

	return color;
}
void DrawLine(UCanvas* Canvas, Vector2 position1, Vector2 position2, ue::FLinearColor color, int thickness = 1, bool screenCheck = false)
{
		Canvas->K2_DrawLine(position1, position2, thickness, color);
}

void DrawCorneredBox(UCanvas* Canvas, Vector2 position, Vector2 size, int thickness, ue::FLinearColor colora) {

	float lineW = (size.x / 4);
	float lineH = (size.y / 4);

	//corners
	DrawLine(Canvas, Vector2(position.x, position.y), Vector2(position.x, position.y + lineH), colora, 2);
	DrawLine(Canvas, Vector2(position.x, position.y), Vector2(position.x + lineW, position.y), colora, 2);
	DrawLine(Canvas, Vector2(position.x + size.x - lineW, position.y), Vector2(position.x + size.x, position.y), colora, 2);
	DrawLine(Canvas, Vector2(position.x + size.x, position.y), Vector2(position.x + size.x, position.y + lineH), colora, 2);
	DrawLine(Canvas, Vector2(position.x, position.y + size.y - lineH), Vector2(position.x, position.y + size.y), colora, 2);
	DrawLine(Canvas, Vector2(position.x, position.y + size.y), Vector2(position.x + lineW, position.y + size.y), colora, 2);
	DrawLine(Canvas, Vector2(position.x + size.x - lineW, position.y + size.y), Vector2(position.x + size.x, position.y + size.y), colora, 2);
	DrawLine(Canvas, Vector2(position.x + size.x, position.y + size.y - lineH), Vector2(position.x + size.x, position.y + size.y), colora, 2);
}

auto redcol = ue::FLinearColor(1.f, 1.f, 1.f, 1.f);	
auto bluecol = ue::FLinearColor(1.f, 1.f, 1.f, 1.f);
auto redcol2 = ue::FLinearColor(1.f, 0.8f, 0.f, 1.f);
auto bluecol2 = ue::FLinearColor(0.f, 2.55f, 2.55f, 1.f);
auto bluecol3 = ue::FLinearColor(0, 1, 1, 1);

auto gold = ue::FLinearColor(1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f);
auto lightgreen = ue::FLinearColor(0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f);

void SetMouseAbsPosition(DWORD x, DWORD y)
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = x;
	input.mi.dy = y;
	SendInput(1, &input, sizeof(input));
}
void aimbot(float x, float y)
{
	float ScreenCenterX = (width / 2);
	float ScreenCenterY = (height / 2);
	int AimSpeed = Settings::mousesmooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	SetMouseAbsPosition(TargetX, TargetY);

	return;
}


void actorloop() {
	AFortPawn* targetpawn = nullptr;
	float closestTocenter = FLT_MAX;
	width = static_canvas->SizeX();
	height = static_canvas->SizeY();
	auto gworld = ((GWorld*)(*(uintptr_t*)(ue::cached::signatures::GWorld)));
	auto OwningGameInstance = gworld->OwningGameInstance();
	PlayerController = OwningGameInstance->LocalPlayers()->LocalPlayer()->PlayerController();
	auto PlayerCameraManager = PlayerController->PlayerCameraManager();
	auto LocalPawn = (AFortPawn*)PlayerController->LocalPawn();
	auto levels = gworld->Levels();
	auto CurrentVehicle = (AFortPawn*)LocalPawn->GetVehicle();
	auto CurrentWeapon = (AFortPawn*)LocalPawn->CurrentWeapon();
	static_canvas->K2_DrawText(Vector2(width / 2, 50), redcol2, xorstr(L"Mokobake Modded Private"), true, true);

	if (Settings::FovChanger)
	{
		PlayerController->Fov(Settings::FovChanger_Value);
	}

	if (Settings::VehicleFly)
	{
		if (CurrentVehicle)
		{
			Vector3 Location = CurrentVehicle->RootComponent()->RelativeLocation();

			if (Settings::VehicleTP)
			{
				if (!safe_call(GetAsyncKeyState)(VK_LSHIFT))
				{
					if (Settings::SyncCameraRotation)
					{
						Vector3 current_rotation = PlayerCameraManager->GetCameraRotation();
						bool pushing;

						if (safe_call(GetAsyncKeyState)(0x57)) //w
						{
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x53)) //s
						{
							current_rotation = { -current_rotation.x, current_rotation.y + 180, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x41)) //a
						{
							current_rotation = { 0, current_rotation.y + 270, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x44)) //d
						{
							current_rotation = { 0, current_rotation.y + 90, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x57) && safe_call(GetAsyncKeyState)(0x44)) //w d
						{
							current_rotation = { current_rotation.x, current_rotation.y + 45, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x57) && safe_call(GetAsyncKeyState)(0x41)) //w a
						{
							current_rotation = { current_rotation.x, current_rotation.y + 315, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x53) && safe_call(GetAsyncKeyState)(0x44)) //s d
						{
							current_rotation = { -current_rotation.x, current_rotation.y + 135, 0 };
							pushing = true;
						}
						else if (safe_call(GetAsyncKeyState)(0x53) && safe_call(GetAsyncKeyState)(0x41)) //s a
						{
							current_rotation = { -current_rotation.x, current_rotation.y + 225, 0 };
							pushing = true;
						}
						else
						{
							pushing = false;
						}

						double angle;

						angle = current_rotation.y * (M_PI / 180.0);
						double sy = sinf(angle);
						double cy = cosf(angle);

						angle = -current_rotation.x * (M_PI / 180.0);
						double sp = sinf(angle);
						double cp = cosf(angle);

						if (pushing)
						{
							CurrentVehicle->K2_TeleportTo(Location + Vector3{ cp * cy , cp * sy, -sp } *Settings::VehicleFlySpeed, current_rotation);
						}
						else
						{
							CurrentVehicle->K2_TeleportTo(Location, current_rotation);
						}
					}
					else
					{
						if (safe_call(GetAsyncKeyState)(0x57)) //w
						{
							Location.x += Settings::VehicleFlySpeed;
						}

						if (safe_call(GetAsyncKeyState)(0x53)) //s
						{
							Location.x -= Settings::VehicleFlySpeed;
						}

						if (safe_call(GetAsyncKeyState)(0x41)) //a
						{
							Location.y -= Settings::VehicleFlySpeed;
						}

						if (safe_call(GetAsyncKeyState)(0x44)) //d
						{
							Location.y += Settings::VehicleFlySpeed;
						}

						if (safe_call(GetAsyncKeyState)(VK_SPACE))
						{
							Location.z += Settings::VehicleFlySpeed;
						}

						CurrentVehicle->K2_TeleportTo(Location, Vector3(NULL, NULL, NULL));
					}
				}
			}
			else
			{
				if (Settings::SyncCameraRotation)
				{
					Vector3 current_rotation = PlayerCameraManager->GetCameraRotation();
					bool pushing;

					if (safe_call(GetAsyncKeyState)(0x57)) //w
					{
						pushing = true;
					}
					else if (safe_call(GetAsyncKeyState)(0x53)) //s
					{
						current_rotation = { -current_rotation.x, current_rotation.y + 180, 0 };
						pushing = true;
					}
					else if (safe_call(GetAsyncKeyState)(0x41)) //a
					{
						current_rotation = { 0, current_rotation.y + 270, 0 };
						pushing = true;
					}
					else if (safe_call(GetAsyncKeyState)(0x44)) //d
					{
						current_rotation = { 0, current_rotation.y + 90, 0 };
						pushing = true;
					}
					else
					{
						pushing = false;
					}

					double angle;

					angle = current_rotation.y * (M_PI / 180.0);
					double sy = sinf(angle);
					double cy = cosf(angle);

					angle = -current_rotation.x * (M_PI / 180.0);
					double sp = sinf(angle);
					double cp = cosf(angle);

					if (pushing)
					{
						CurrentVehicle->K2_TeleportTo(Location + Vector3{ cp * cy , cp * sy, -sp } *Settings::VehicleFlySpeed, current_rotation);
					}
					else
					{
						CurrentVehicle->K2_TeleportTo(Location, current_rotation);
					}
				}
				else
				{
					if (safe_call(GetAsyncKeyState)(0x57)) //w
					{
						Location.x += Settings::VehicleFlySpeed;
					}

					if (safe_call(GetAsyncKeyState)(0x53)) //s
					{
						Location.x -= Settings::VehicleFlySpeed;
					}

					if (safe_call(GetAsyncKeyState)(0x41)) //a
					{
						Location.y -= Settings::VehicleFlySpeed;
					}

					if (safe_call(GetAsyncKeyState)(0x44)) //d
					{
						Location.y += Settings::VehicleFlySpeed;
					}

					if (safe_call(GetAsyncKeyState)(VK_SPACE))
					{
						Location.z += Settings::VehicleFlySpeed;
					}

					if (safe_call(GetAsyncKeyState)(VK_LSHIFT))
					{
						Location.z -= Settings::VehicleFlySpeed;
					}

					CurrentVehicle->K2_TeleportTo(Location, Vector3(NULL, NULL, NULL));
				}
			}
		}
	}

	if (safe_call(GetAsyncKeyState)(VK_RCONTROL))
	{
		Settings::AddPlayerFlyLocation = Vector3(0, 0, 0);

		static_canvas->K2_DrawText(Vector2(950, 20), gold, xorstr(L"Player Fly Location Initialized!"), true, true);
	}

	if (Settings::PlayerFly)
	{
		if (LocalPawn)
		{
			if (safe_call(GetAsyncKeyState)(0x57)) //w
			{
				Settings::AddPlayerFlyLocation.x += Settings::PlayerFlySpeed;
			}

			if (safe_call(GetAsyncKeyState)(0x53)) //s
			{
				Settings::AddPlayerFlyLocation.x -= Settings::PlayerFlySpeed;
			}

			if (safe_call(GetAsyncKeyState)(0x41)) //a
			{
				Settings::AddPlayerFlyLocation.y -= Settings::PlayerFlySpeed;
			}

			if (safe_call(GetAsyncKeyState)(0x44)) //d
			{
				Settings::AddPlayerFlyLocation.y += Settings::PlayerFlySpeed;
			}

			if (safe_call(GetAsyncKeyState)(VK_SPACE))
			{
				Settings::AddPlayerFlyLocation.z += Settings::PlayerFlySpeed;
			}

			if (safe_call(GetAsyncKeyState)(VK_LSHIFT))
			{
				Settings::AddPlayerFlyLocation.z -= Settings::PlayerFlySpeed;
			}

			Vector3 FlyLocation = Settings::SetLocation + Settings::AddPlayerFlyLocation;
			Vector3 camrotation = PlayerCameraManager->GetCameraRotation();

			LocalPawn->K2_SetActorLocation(FlyLocation, false, true);
		}
	}
	else
	{
		Settings::SetLocation = LocalPawn->RootComponent()->RelativeLocation();
	}

	if (Settings::DisableGetCurrentLocation)
	{
		if (LocalPawn)
		{
			*(float*)((uintptr_t)LocalPawn + 0x64) = 0;

			if (safe_call(GetAsyncKeyState)(VK_RBUTTON))
			{
				*(float*)((uintptr_t)LocalPawn + 0x64) = 1;
			}
		}
	}

	if (Settings::LocalPawnSpeed)
	{
		if (LocalPawn)
		{
			if (safe_call(GetAsyncKeyState)(0x46)) //f
			{
				*(float*)((uintptr_t)LocalPawn + 0x64) = 500;
			}
			else
			{
				*(float*)((uintptr_t)LocalPawn + 0x64) = 1;
			}
		}
	}

	if (Settings::CurrentWeaponSpeed)
	{
		if (CurrentWeapon)
		{
			*(float*)(CurrentWeapon + 0x64) = FLT_MAX;
		}
	}

	if (Settings::NoWeaponCoolDown)
	{
		if (CurrentWeapon)
		{
			*(float*)(CurrentWeapon + 0x1550) = 0;
		}
	}

	if (Settings::FullAutoWeapons)
	{
		if (CurrentWeapon)
		{
			*(uint8_t*)(LocalPawn->CurrentWeapon()->WeaponData() + ue::cached::offsets::TriggerType) = 1;
		}
	}

	if (safe_call(GetAsyncKeyState)(VK_HOME))
	{
		AllocConsole();
		freopen(xorstr("CONIN$"), xorstr("r"), stdin);
		freopen(xorstr("CONOUT$"), xorstr("w"), stdout);
		freopen(xorstr("CONOUT$"), xorstr("w"), stderr);
	}

	if (safe_call(GetAsyncKeyState)(VK_END))
	{
		FreeConsole();
	}

	if (Settings::RapidFire)
	{
		if (CurrentWeapon)
		{
			LocalPawn->CurrentWeapon()->RapidFire();
		}
	}

	if (Settings::NoRecoil)
	{
		*(float*)(PlayerController + 0x64) = -1;
	}

	if (Settings::InstantReload)
	{
		if (CurrentWeapon)
		{
			bool bIsReloadingWeapon = *(bool*)(CurrentWeapon + ue::cached::offsets::bIsReloadingWeapon);
			auto Mesh = *(uintptr_t*)(LocalPawn + ue::cached::offsets::Mesh);
	
			if (bIsReloadingWeapon)
			{
				*(float*)(Mesh + ue::cached::offsets::GlobalAnimRateScale) = 999;
			}
			else
			{
				*(float*)(Mesh + ue::cached::offsets::GlobalAnimRateScale) = 1;
			}
		}
	}

	if (Settings::VehicleNoGravity)
	{
		if (CurrentVehicle)
		{
			*(float*)(CurrentVehicle + ue::cached::offsets::VehicleAttributes + 0x18) = 0.01;
		}
	}

	if (Settings::Speedhack)
	{
		if (safe_call(GetAsyncKeyState)(0x46)) //f
		{
			setAllToSpeed(Settings::SpeedhackValue);
		}
		else
		{
			setAllToSpeed(1);
		}
	}

	if (Settings::UnrealEngineSpeedhack)
	{
		if (LocalPawn)
		{
			auto WorldSettings = (GWorld*)gworld->GetWorldSettings();

			if (safe_call(GetAsyncKeyState)(0x46)) //f
			{
				*(float*)(WorldSettings + ue::cached::offsets::TimeDilation) = Settings::UnrealEngineSpeedhackValue;
			}
			else
			{
				*(float*)(WorldSettings + ue::cached::offsets::TimeDilation) = 1;
			}
		}
	}

	if (Settings::ZiplineTeleport)
	{
		if (LocalPawn)
		{
			float CustomTimeDilation = *(float*)(LocalPawn + 0x64);

			if (safe_call(GetAsyncKeyState)(VK_F1))
			{
				LocalPawn->K2_SetActorLocation(Vector3(24198.2, -10233.5, 4316.96), false, true);
				*(float*)(LocalPawn + 0x64) = 0;
			}
			else if (safe_call(GetAsyncKeyState)(VK_F2))
			{
				LocalPawn->K2_SetActorLocation(Vector3(72066.6, 23895.7, 7790.23), false, true);
				*(float*)(LocalPawn + 0x64) = 0;
			}
			else if (safe_call(GetAsyncKeyState)(VK_F3))
			{
				LocalPawn->K2_SetActorLocation(Vector3(57793.7, -62427, 5629), false, true);
				*(float*)(LocalPawn + 0x64) = 0;
			}
			else if (safe_call(GetAsyncKeyState)(VK_F5))
			{
				LocalPawn->K2_SetActorLocation(Vector3(5902.17, 28763.5, 2804.1), false, true);
				*(float*)(LocalPawn + 0x64) = 0;
			}
			else if (safe_call(GetAsyncKeyState)(VK_F6))
			{
				LocalPawn->K2_SetActorLocation(Vector3(-51238.5, 19975, 1093.84), false, true);
				*(float*)(LocalPawn + 0x64) = 0;
			}
			else if (CustomTimeDilation != 1 && !Settings::DisableGetCurrentLocation)
			{
				*(float*)(LocalPawn + 0x64) = 1;
			}
		}
	}

	if (Settings::QuickFly)
	{
		if (LocalPawn)
		{
			if (safe_call(GetAsyncKeyState)(VK_RSHIFT))
			{
				*(bool*)(LocalPawn + ue::cached::offsets::ZiplineState + 0x18) = true;
				*(float*)(LocalPawn + ue::cached::offsets::ZiplineSpeedFactor) = 1000;
			}
		}
	}

	if (Settings::VehicleMapMarkerToTeleport)
	{
		if (CurrentVehicle)
		{
			if (PlayerController->WasInputKeyJustPressed(EnterKey))
			{
				if (Settings::SyncCameraRotation)
				{
					Vector3 camrotation = PlayerCameraManager->GetCameraRotation();

					CurrentVehicle->K2_TeleportTo(Settings::MarkerLocation, camrotation);
				}
				else
				{
					CurrentVehicle->K2_TeleportTo(Settings::MarkerLocation, Vector3(NULL, NULL, NULL));
				}
			}
		}
	}

	if (Settings::spinbot)
	{
		if (LocalPawn && Settings::SilentAim)
		{
			uintptr_t ViewTarget = *(uintptr_t*)(PlayerCameraManager + ue::cached::offsets::ViewTarget);
			static Vector3 Rotation;

			if (PlayerController->IsInputKeyDown(RBKey))
			{
				Rotation = PlayerCameraManager->GetCameraRotation();
				*(uintptr_t*)(PlayerCameraManager + ue::cached::offsets::ViewTarget) = 0;
				Settings::Rotation.y += 100;
				PlayerController->ClientSetRotation(Settings::Rotation, false);
			}
			else if (ViewTarget != NULL && PlayerController->WasInputKeyJustReleased(RBKey))
			{
				PlayerController->ClientSetRotation(Rotation, false);
				*(uintptr_t*)(PlayerCameraManager + ue::cached::offsets::ViewTarget) = (uintptr_t)LocalPawn;
			}
		}
	}

	for (int a = 0; a < levels.Num(); a++)
	{
		auto level = levels[a];
		if (!level) continue;
		auto actors = levels[a]->ActorArray();

		for (int i = 0; i < actors.Num(); i++)
		{
			auto actor = actors[i];
			Beep(500, 500);
			if (!actor || actor == LocalPawn)
				continue;

			if (Settings::VehicleTP)
			{
				if (CurrentVehicle)
				{
					if (safe_call(GetAsyncKeyState)(VK_LSHIFT))
					{
						if (targetpawn)
						{
							CurrentVehicle->K2_SetActorLocation(targetpawn->GetBone(5), false, false);
						}
					}
				}
			}

			if (targetpawn)
			{
				Vector3 tergetloc = targetpawn->GetBone(Settings::AimBone);
				if (actor->IsA(ue::cached::objects::FortProjectileBase) && Settings::SniperTp)
				{
					actor->K2_SetActorLocation(tergetloc, false, false);
				}
			}

			if (actor->IsA(ue::cached::objects::FortProjectileBase) && Settings::ProjectileDestroyer && safe_call(GetAsyncKeyState)(VK_F1))
			{
				actor->K2_DestroyActor();
			}

			if (Settings::VehicleMapMarkerToTeleport && wcsstr(actor->ObjectName(), xorstr(L"AthenaPlayerMarker_WithCustomization")))
			{
				Settings::MarkerLocation = Vector3(actor->RootComponent()->RelativeLocation().x, actor->RootComponent()->RelativeLocation().y, actor->RootComponent()->RelativeLocation().z + 500);
			}


			if (Settings::chestesp && wcsstr(actor->ObjectName(), xorstr(L"Tiered_Chest")))
			{
				if (!actor->bAlreadySearched())
				{
					static char distanceesp[128];
					static wchar_t wdistanceesp[128];
					sprintf(distanceesp, xorstr("Chest [%dm]"), (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100));
					AnsiToWide(distanceesp, wdistanceesp);
					auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
					if (IsNotInScreen(width, height, ItemPos)) continue;

					static_canvas->K2_DrawText(ItemPos, gold, wdistanceesp, true, true);
				}
			}

			if (Settings::ProjectileESP && actor->IsA(ue::cached::objects::FortProjectileBase))
			{
				static char distanceesp[128];
				static wchar_t wdistanceesp[128];
				sprintf(distanceesp, xorstr("Projectile [%dm]"), (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100));
				AnsiToWide(distanceesp, wdistanceesp);
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, ue::FLinearColor(1.f, 1.f, 1.f, 1.f), wdistanceesp, true, false);
			}

			if (Settings::ProjectileObjectNameESP && actor->IsA(ue::cached::objects::FortProjectileBase))
			{
				auto objectname = actor->ObjectName();
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, ue::FLinearColor(1.f, 1.f, 1.f, 1.f), objectname, true, false);
			}

			if (Settings::vehicleespnerds && wcsstr(actor->ObjectName(), xorstr(L"Vehicle")))
			{
				static char distanceesp[128];
				static wchar_t wdistanceesp[128];
				sprintf(distanceesp, xorstr("Vehicle [%dm]"), (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100));
				AnsiToWide(distanceesp, wdistanceesp);
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, bluecol, wdistanceesp, true, true);
			}

			if (Settings::LootESP && actor->IsA(ue::cached::objects::actor::FortPickupAthena))
			{
				ue::FString DisplayName = actor->ItemDefinition()->DisplayName().Get();
				EFortItemTier Tier = GetItemEFortTier(actor->ItemDefinition()->Tier());
				ue::FLinearColor ItemColor = GetItemColor(Tier);
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, ItemColor, DisplayName, true, true);
			}

			if (Settings::DMRSeller && wcsstr(actor->ObjectName(), xorstr(L"DMR")) && !actor->IsA(ue::cached::objects::FortProjectileBase))
			{
				static char distanceesp[128];
				static wchar_t wdistanceesp[128];
				sprintf(distanceesp, xorstr("Selling DMR [%dm]"), (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100));
				AnsiToWide(distanceesp, wdistanceesp);
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, gold, wdistanceesp, true, true);
			}

			if (Settings::ammobox && wcsstr(actor->ObjectName(), xorstr(L"Tiered_Ammo_Athena")))
			{
				if (!actor->bAlreadySearched())
				{
					static char distanceesp[128];
					static wchar_t wdistanceesp[128];
					sprintf(distanceesp, xorstr("AmmoBox [%dm]"), (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100));
					AnsiToWide(distanceesp, wdistanceesp);
					auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
					if (IsNotInScreen(width, height, ItemPos)) continue;

					static_canvas->K2_DrawText(ItemPos, lightgreen, wdistanceesp, true, true);
				}
			}

			if (Settings::ActorObjectName && (int)(PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100) < 10)
			{
				auto objectname = actor->ObjectName();
				auto ItemPos = PlayerController->WorldToScreen(actor->RootComponent()->RelativeLocation());
				if (IsNotInScreen(width, height, ItemPos)) continue;

				static_canvas->K2_DrawText(ItemPos, ue::FLinearColor(1.f, 1.f, 1.f, 1.f), objectname, true, false);
			}


			if (actor->IsA(ue::cached::objects::actor::FortPlayerPawnAthena))
			{
				if (actor->PlayerState()->TeamIndex() == LocalPawn->PlayerState()->TeamIndex())
					continue;

				char bIsDying = actor->bIsDying();
				std::string bIsDyingStr(1, bIsDying);
				if (bIsDyingStr == "0")
				{
					continue;
				}

				Vector3 head = actor->GetBone(68);
				Vector2 head_w2s = PlayerController->WorldToScreen(head);


				if (actor != LocalPawn)
				{
					auto dx = head_w2s.x - (width / 2);
					auto dy = head_w2s.y - (height / 2);
					auto dist = crt::sqrtf(dx * dx + dy * dy);

					if (dist < Settings::FovSize && dist < closestTocenter)
					{
						closestTocenter = dist;
						targetpawn = actor;
					}
					else if (!dist < Settings::FovSize && !dist < closestTocenter)
					{
						TargetPawno = 0;
					}

				}
				TargetPawno = targetpawn;
				LocalPawno = LocalPawn;
				PlayerCameraManagero = PlayerCameraManager;


				if (Settings::mouseaim and targetpawn and PlayerController->IsInputKeyDown(RBKey))
				{
					if (PlayerController->LineOfSightTo(targetpawn))
					{
						Vector3 headmarket = targetpawn->GetBone(Settings::AimBone);
						Vector2 headlarketn = PlayerController->WorldToScreen(headmarket);
						aimbot(headlarketn.x, headlarketn.y);
					}
				}

				if (Settings::MemoryAim and targetpawn and PlayerController->IsInputKeyDown(RBKey))
				{
					if (PlayerController->LineOfSightTo(targetpawn))
					{
						Vector3 headtarget = targetpawn->GetBone(Settings::AimBone);
						Vector2 headtarget_w2s = PlayerCameraManager->ProjectWorldLocationToScreen(width, height, headtarget);
						auto camloc = PlayerCameraManager->GetCameraLocation();
						auto NewRotation = galgan(camloc, headtarget);
						PlayerController->ClientSetRotation(NewRotation, false); //never got around to making ud memory none of my attemps to hook get roation shit worked :shrug:
					}
				}


				Vector2 head2_w2s = PlayerCameraManager->ProjectWorldLocationToScreen(width, height, { head.x, head.y, head.z + 20 });
				Vector3 bottom = actor->GetBone(0);

				Vector2 bottom_w2s = PlayerCameraManager->ProjectWorldLocationToScreen(width, height, { bottom.x, bottom.y, bottom.z });
				ue::FLinearColor col = ue::FLinearColor(1.f, 1.f, 1.f, 1.f);
				float TextOffset_Top;
				auto WorldHead = actor->GetBone(68);
				auto Head = PlayerController->WorldToScreen(WorldHead);
				Vector2 Top;
				Top = PlayerController->WorldToScreen({ WorldHead.x, WorldHead.y, WorldHead.z + 20 });
				auto Bottom = PlayerController->WorldToScreen(actor->GetBone(0));
				TextOffset_Top = Top.y;
				float TextOffset_Bottom = Bottom.y;

				if (IsNotInScreen(width, height, head_w2s)) continue;
				float BoxHeight = (float)(bottom_w2s.y - head2_w2s.y);
				float BoxWidth = BoxHeight * 0.50f;

				float BottomLeftX = (float)head2_w2s.x - BoxWidth / 2;
				float BottomLeftY = (float)bottom_w2s.y;

				float BottomRightX = (float)head2_w2s.x + BoxWidth / 2;
				float BottomRightY = (float)bottom_w2s.y;

				float TopRightX = (float)head2_w2s.x + BoxWidth / 2;
				float TopRightY = (float)head2_w2s.y;

				float TopLeftX = (float)head2_w2s.x - BoxWidth / 2;
				float TopLeftY = (float)head2_w2s.y;

				if (Settings::BoxESP)
				{
					if (PlayerController->LineOfSightTo(actor))
					{
						static_canvas->K2_DrawLine(Vector2(BottomLeftX, BottomLeftY), Vector2(BottomRightX, BottomRightY), 2.f, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
						static_canvas->K2_DrawLine(Vector2(BottomRightX, BottomRightY), Vector2(TopRightX, TopRightY), 2.f, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
						static_canvas->K2_DrawLine(Vector2(TopRightX, TopRightY), Vector2(TopLeftX, TopLeftY), 2.f, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
						static_canvas->K2_DrawLine(Vector2(TopLeftX, TopLeftY), Vector2(BottomLeftX, BottomLeftY), 2.f, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
					}
					else
					{
						static_canvas->K2_DrawLine(Vector2(BottomLeftX, BottomLeftY), Vector2(BottomRightX, BottomRightY), 2.f, col);
						static_canvas->K2_DrawLine(Vector2(BottomRightX, BottomRightY), Vector2(TopRightX, TopRightY), 2.f, col);
						static_canvas->K2_DrawLine(Vector2(TopRightX, TopRightY), Vector2(TopLeftX, TopLeftY), 2.f, col);
						static_canvas->K2_DrawLine(Vector2(TopLeftX, TopLeftY), Vector2(BottomLeftX, BottomLeftY), 2.f, col);
					}
				}

				if (Settings::box)
				{
					float Height = Bottom.y - Top.y;
					float Width = Height * 0.50;
					Vector2 min;
					min.x = Bottom.x + Width / 2;
					min.y = Bottom.y;
					Vector2 max;
					max.x = Top.x - Width / 2;
					max.y = Top.y;
					Vector2 size;
					size.x = min.x - max.x;
					size.y = min.y - max.y;
					if (PlayerController->LineOfSightTo(actor))
					{
						DrawCorneredBox(static_canvas, max, size, 1.f, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
					}
					else
					{
						DrawCorneredBox(static_canvas, max, size, 1.f, col);
					}
				}

				float TextOffset_Y = head2_w2s.y;
				float TextOffset_X = TopRightX + 5;
				Vector2 calculation;
				if (Settings::PlayerNameESP)
				{
					int ifespison = 30;
					if (sdkUssage->isValidPointer((uintptr_t)LocalPawn))
					{
						if (Settings::EnemyWeaponESP)
						{
							int ifespison = 15;
						}
					}

					auto PlayerState = actor->PlayerState();
					if (PlayerState)
					{
						TextOffset_Top -= static_canvas->K2_TextSize(PlayerState->GetPlayerName()).y + ifespison - 28;
						Vector2 calculation = Vector2(Top.x, TextOffset_Top);
						static_canvas->K2_DrawText(calculation, redcol2, PlayerState->GetPlayerName().c_str(), true, false);
					}
				}

				if (Settings::DistanceESP)
				{
					int dist = PlayerCameraManager->GetCameraLocation().Distance(actor->RootComponent()->RelativeLocation()) / 100;
					static char distanceesp[128];
					static wchar_t wdistanceesp[128];
					sprintf(distanceesp, xorstr("%d m"), dist);
					AnsiToWide(distanceesp, wdistanceesp);
					TextOffset_Y += static_canvas->K2_TextSize(ue::FString(wdistanceesp)).y + 10;
					Vector2 calculation = Vector2(Top.x, TextOffset_Top - 12);
					static_canvas->K2_DrawText(calculation, bluecol, wdistanceesp, true, false);
				}

				if (Settings::PlatformESP)
				{
					auto platform = actor->PlayerState()->GetPlatform();
					if (platform.IsValid())
					{
						auto platform_name = platform.c_str();
						TextOffset_Top += static_canvas->K2_TextSize(ue::FString(platform_name)).y + 10;
						Vector2 calculation = Vector2(Top.x - 25, TextOffset_Top);

						if (math::custom_wcsstr(platform_name, xorstr(L"PSN")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"PS4"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"PS5")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"PS5"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"XBL")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Xbox One"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"XSS")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Xbox Series S"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"XSX")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Xbox Series X"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"WIN")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Windows"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"MAC")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Mac"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"LNX")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Linux"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"IOS")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"iOS"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"AND")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Android"), false, false);
						}
						else if (math::custom_wcsstr(platform_name, xorstr(L"SWT")))
						{
							static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Switch"), false, false);
						}
						else
						{
							static_canvas->K2_DrawText(calculation, redcol2, platform_name, false, false);
						}
					}
					else
					{
						TextOffset_Top += static_canvas->K2_TextSize(ue::FString(L"Bot")).y + 10;
						Vector2 calculation = Vector2(Top.x - 25, TextOffset_Top);
						static_canvas->K2_DrawText(calculation, redcol2, xorstr(L"Bot"), false, false);
					}
				}

				Vector2 ChestAtLeft = PlayerController->WorldToScreen(actor->GetBone(37));
				Vector2 ChestAtRight = PlayerController->WorldToScreen(actor->GetBone(8));
				Vector2 Chest = { ChestAtLeft.x + (ChestAtRight.x - ChestAtLeft.x) / 2, ChestAtLeft.y };
				ue::FLinearColor Color = ue::FLinearColor(1.f, 1.f, 0.f, 1.f);

				if (Settings::SkeletonESP)
				{
					Vector2 Neck = PlayerController->WorldToScreen(actor->GetBone(66));

					Vector2 LeftShoulder = PlayerController->WorldToScreen(actor->GetBone(64));
					Vector2 RightShoulder = PlayerController->WorldToScreen(actor->GetBone(35));
					Vector2 LeftElbow = PlayerController->WorldToScreen(actor->GetBone(39));
					Vector2 RightElbow = PlayerController->WorldToScreen(actor->GetBone(10));
					Vector2 LeftHand = PlayerController->WorldToScreen(actor->GetBone(40));
					Vector2 RightHand = PlayerController->WorldToScreen(actor->GetBone(11));
					Vector2 LeftLeg = PlayerController->WorldToScreen(actor->GetBone(78));
					Vector2 RightLeg = PlayerController->WorldToScreen(actor->GetBone(71));
					Vector2 LeftThigh = PlayerController->WorldToScreen(actor->GetBone(79));
					Vector2 RightThigh = PlayerController->WorldToScreen(actor->GetBone(72));
					Vector2 LeftFoot = PlayerController->WorldToScreen(actor->GetBone(80));
					Vector2 RightFoot = PlayerController->WorldToScreen(actor->GetBone(73));
					Vector2 LeftFeet = PlayerController->WorldToScreen(actor->GetBone(82));
					Vector2 RightFeet = PlayerController->WorldToScreen(actor->GetBone(86));
					Vector2 LeftFeetFinger = PlayerController->WorldToScreen(actor->GetBone(83));
					Vector2 RightFeetFinger = PlayerController->WorldToScreen(actor->GetBone(76));
					Vector2 Bottom = PlayerController->WorldToScreen(actor->GetBone(0));
					Vector2 Pelvis = PlayerController->WorldToScreen(actor->GetBone(2));

					static_canvas->K2_DrawLine(head_w2s, Neck, 2, bluecol2);
					static_canvas->K2_DrawLine(Neck, Chest, 2, bluecol2);
					static_canvas->K2_DrawLine(Chest, Pelvis, 2, bluecol2);
					static_canvas->K2_DrawLine(Chest, LeftShoulder, 2, bluecol2);
					static_canvas->K2_DrawLine(Chest, RightShoulder, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftShoulder, LeftElbow, 2, bluecol2);
					static_canvas->K2_DrawLine(RightShoulder, RightElbow, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftElbow, LeftHand, 2, bluecol2);
					static_canvas->K2_DrawLine(RightElbow, RightHand, 2, bluecol2);
					static_canvas->K2_DrawLine(Pelvis, LeftLeg, 2, bluecol2);
					static_canvas->K2_DrawLine(Pelvis, RightLeg, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftLeg, LeftThigh, 2, bluecol2);
					static_canvas->K2_DrawLine(RightLeg, RightThigh, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftThigh, LeftFoot, 2, bluecol2);
					static_canvas->K2_DrawLine(RightThigh, RightFoot, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftFoot, LeftFeet, 2, bluecol2);
					static_canvas->K2_DrawLine(RightFoot, RightFeet, 2, bluecol2);
					static_canvas->K2_DrawLine(LeftFeet, LeftFeetFinger, 2, bluecol2);
					static_canvas->K2_DrawLine(RightFeet, RightFeetFinger, 2, bluecol2);
				}

				if (Settings::EnemyWeaponESP and LocalPawn)
				{
					if (actor->CurrentWeapon()->WeaponData())
					{
						auto weaponname = actor->CurrentWeapon()->WeaponData()->DisplayName().Get();
						if (weaponname)
						{
							TextOffset_Y += static_canvas->K2_TextSize(weaponname).y + 10;
							calculation = Vector2(Bottom.x, Bottom.y + 8);
							static_canvas->K2_DrawText(calculation, bluecol, weaponname, true, false);
						}
					}
				}

				if (Settings::LinesESP)
				{
					if (PlayerController->LineOfSightTo(actor))
					{
						static_canvas->K2_DrawLine(Vector2(width / 2, height), bottom_w2s, 1, ue::FLinearColor(1.f, 0.f, 0.f, 1.f));
					}
					else
					{
						static_canvas->K2_DrawLine(Vector2(width / 2, height), bottom_w2s, 1, col);
					}
				}
			}
		}
	}
}

Vector3 OriginalLocation;
Vector3 OriginalRotation;
Vector3 rot;

void(*o_GetViewPoint)(uintptr_t, ue::FMinimalViewInfo*, BYTE) = nullptr;
void hk_GetViewPoint(uintptr_t this_LocalPlayer, ue::FMinimalViewInfo* OutViewInfo, BYTE StereoPass)
{
	o_GetViewPoint(this_LocalPlayer, OutViewInfo, StereoPass);

	if (sdkUssage->isValidPointer(uintptr_t(LocalPawno)))
	{
		if (Settings::SilentAim)
		{
			OutViewInfo->Rotation = OriginalRotation;
			OutViewInfo->Location = OriginalLocation;
		}
	}
}

void(*o_GetPlayerViewPoint)(uintptr_t, Vector3*, Vector3*) = nullptr;
void hk_GetPlayerViewPoint(uintptr_t this_PlayerController, Vector3* Location, Vector3* Rotation)
{
	o_GetPlayerViewPoint(this_PlayerController, Location, Rotation);
	OriginalLocation = *Location;
	OriginalRotation = *Rotation;
	if (sdkUssage->isValidPointer(uintptr_t(LocalPawno)))
	{
		if (Settings::SilentAim)
		{
			auto rootHead = TargetPawno->GetBone(Settings::AimBone);
			if (PlayerController->LineOfSightTo(TargetPawno))
			{
				Vector3 camloc = *Location;
				Vector3 VectorPos;
				VectorPos.x = rootHead.x - camloc.x;
				VectorPos.y = rootHead.y - camloc.y;
				VectorPos.z = rootHead.z - camloc.z;
				float distance = (double)(sqrtf(VectorPos.x * VectorPos.x + VectorPos.y * VectorPos.y + VectorPos.z * VectorPos.z));

				rot.x = -((acosf(VectorPos.z / distance) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510)) - 90.f);
				rot.y = atan2f(VectorPos.y, VectorPos.x) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510);
				rot.z = 0;
				*Rotation = rot;
			}
		}
	}
}

BOOL(*Spread)(PVOID, float*, float*) = nullptr;
BOOL SpreadHook(PVOID a1, float* a2, float* a3)
{
	if (Settings::NoSpread && safe_call(GetAsyncKeyState)(VK_LBUTTON) || safe_call(GetAsyncKeyState)(VK_RBUTTON))
	{
		return 0;
	}

	return Spread(a1, a2, a3);
}

float OriginalFiringRate;

void(*FireSingleBulletOriginal)(AWeapon*) = nullptr;
auto FireSingleBulletHook(AWeapon* Weapon)
{
	Beep(300, 100);
	if (Settings::RapidFire && Weapon != nullptr)
	{
		auto WeaponStatsRow = Weapon->GetWeaponStats();

		if (WeaponStatsRow && OriginalFiringRate != 0.0f)
		{
			*(float*)(WeaponStatsRow + 0x1c8) = OriginalFiringRate, OriginalFiringRate = 0.0f;
		}
	}

	FireSingleBulletOriginal(Weapon);

	if (Settings::RapidFire && Weapon != nullptr)
	{
		auto WeaponStatsRow = Weapon->GetWeaponStats();

		if (WeaponStatsRow)
		{
			auto& FiringRate = *(float*)(WeaponStatsRow + 0x1c8);

			auto MaxFiringRate = Weapon->GetMaxFiringRate();

			if (FiringRate != MaxFiringRate && MaxFiringRate != 0.0f)
			{
				OriginalFiringRate = FiringRate;
				FiringRate = MaxFiringRate;
			}
		}
	}
}

float* (*CalculateShot)(PVOID, PVOID, PVOID) = nullptr;
float* CalculateShotHook(PVOID arg0, PVOID arg1, PVOID arg2)
{
	auto ret = CalculateShot(arg0, arg1, arg2);

	Beep(200, 100);

	if (ret && Settings::MagicBullet || Settings::SilentAim && TargetPawno && LocalPawno)
	{

		Vector3 headvec3;
		headvec3 = TargetPawno->GetBone(Settings::AimBone);
		ue::FVector head = { headvec3.x, headvec3.y , headvec3.z };

		Vector3 RootCompLocationvec3 = LocalPawno->RootComponent()->RelativeLocation();
		ue::FVector RootCompLocation = { RootCompLocationvec3.x, RootCompLocationvec3.y , RootCompLocationvec3.z };
		ue::FVector* RootCompLocation_check = &RootCompLocation;
		if (!RootCompLocation_check) return ret;
		auto root = RootCompLocation;

		auto dx = head.X - root.X;
		auto dy = head.Y - root.Y;
		auto dz = head.Z - root.Z;

		if (Settings::MagicBullet)
		{
			ret[4] = head.X;
			ret[5] = head.Y;
			ret[6] = head.Z;
			head.Z -= 16.0f;
			root.Z += 45.0f;

			auto y = atan2f(head.Y - root.Y, head.X - root.X);

			root.X += cosf(y + 1.5708f) * 32.0f;
			root.Y += sinf(y + 1.5708f) * 32.0f;

			auto length = sqrtf(powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
			auto x = -atan2f(head.Z - root.Z, length);
			y = atan2f(head.Y - root.Y, head.X - root.X);

			x /= 2.0f;
			y /= 2.0f;

			ret[0] = -(sinf(x) * sinf(y));
			ret[1] = sinf(x) * cosf(y);
			ret[2] = cosf(x) * sinf(y);
			ret[3] = cosf(x) * cosf(y);
		}

		if (Settings::SilentAim)
		{
			if (dx * dx + dy * dy + dz * dz < 125000.0f)
			{
				ret[4] = head.X;
				ret[5] = head.Y;
				ret[6] = head.Z;
			}
			else
			{
				head.Z -= 16.0f;
				root.Z += 45.0f;

				auto y = atan2f(head.Y - root.Y, head.X - root.X);

				root.X += cosf(y + 1.5708f) * 32.0f;
				root.Y += sinf(y + 1.5708f) * 32.0f;

				auto length = sqrtf(powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
				auto x = -atan2f(head.Z - root.Z, length);
				y = atan2f(head.Y - root.Y, head.X - root.X);

				x /= 2.0f;
				y /= 2.0f;

				ret[0] = -(sinf(x) * sinf(y));
				ret[1] = sinf(x) * cosf(y);
				ret[2] = cosf(x) * sinf(y);
				ret[3] = cosf(x) * cosf(y);
			}
		}

	}

	return ret;
}

static uintptr_t storedPLocalPlayer;
static uintptr_t storedPController;
static uintptr_t storedPCurrentWeapon;

bool hook = true;
bool hook2 = true;
bool hook3 = false;

void VirtualTableFunctionSwap(void* VTable, void* FunctionToSwap, void** pOriginal, int Index)
{
	DWORD Old;

	void* pVTableFunction = (void*)((uint64_t)VTable + Index);
	*pOriginal = *(PVOID*)(pVTableFunction);

	safe_call(VirtualProtectEx)((HANDLE)-1, pVTableFunction, 8, PAGE_EXECUTE_READWRITE, &Old);
	*(PVOID*)pVTableFunction = FunctionToSwap;
	safe_call(VirtualProtectEx)((HANDLE)-1, pVTableFunction, 8, Old, &Old);
}

void DrawTransition(uintptr_t this_, uintptr_t Canvas)
{
	if (!Canvas)
	{
		return DrawTransition(this_, Canvas);
	}
	static_canvas = (UCanvas*)Canvas;

	actorloop();

	auto uworld = ((GWorld*)(*(uintptr_t*)(ue::cached::signatures::GWorld)));
	auto LocalPlayer = uworld->OwningGameInstance()->LocalPlayers()->LocalPlayer();
	
	//GetViewPoint
	if (storedPLocalPlayer != (uintptr_t)LocalPlayer)
	{
		storedPLocalPlayer = (uintptr_t)LocalPlayer;
		hook = true;
	}
	if (storedPLocalPlayer == (uintptr_t)LocalPlayer && hook)
	{
		if (LocalPlayer)
		{
			void** LocalPlayer_VTable = *(void***)(LocalPlayer);
			DWORD OldProtection;
			safe_call(VirtualProtect)(&LocalPlayer_VTable[88], 8, PAGE_EXECUTE_READWRITE, &OldProtection);
			o_GetViewPoint = decltype(o_GetViewPoint)(LocalPlayer_VTable[88]);
			LocalPlayer_VTable[88] = &hk_GetViewPoint;
			safe_call(VirtualProtect)(&LocalPlayer_VTable[88], 8, OldProtection, &OldProtection);
		}
		hook = false;
	}

	//GetPlayerViewPoint
	if (storedPController != (uintptr_t)LocalPlayer->PlayerController())
	{
		storedPController = (uintptr_t)LocalPlayer->PlayerController();
		hook2 = true;
	}
	if (Settings::SilentAim && storedPController == (uintptr_t)LocalPlayer->PlayerController() && hook2)
	{	
		void** PlayerController_VTable = *(void***)(LocalPlayer->PlayerController());
		DWORD OldProtection;
		safe_call(VirtualProtect)(&PlayerController_VTable[247], 8, PAGE_EXECUTE_READWRITE, &OldProtection);
		o_GetPlayerViewPoint = decltype(o_GetPlayerViewPoint)(PlayerController_VTable[247]);
		PlayerController_VTable[247] = &hk_GetPlayerViewPoint;
		safe_call(VirtualProtect)(&PlayerController_VTable[247], 8, OldProtection, &OldProtection);
		hook2 = false;
	}

	if (storedPCurrentWeapon != (uintptr_t)LocalPawno->CurrentWeapon())
	{
		storedPCurrentWeapon = (uintptr_t)LocalPawno->CurrentWeapon();
		hook3 = true;
	}
	if (Settings::RapidFire && storedPCurrentWeapon == (uintptr_t)LocalPawno->CurrentWeapon() && hook3)
	{
		void** CurrentWeapon_VTable = *(void***)(LocalPawno->CurrentWeapon());
		DWORD OldProtection;
		safe_call(VirtualProtect)(&CurrentWeapon_VTable[345], 8, PAGE_EXECUTE_READWRITE, &OldProtection);
		FireSingleBulletOriginal = decltype(FireSingleBulletOriginal)(CurrentWeapon_VTable[345]);
		CurrentWeapon_VTable[345] = &FireSingleBulletHook;
		safe_call(VirtualProtect)(&CurrentWeapon_VTable[345], 8, OldProtection, &OldProtection);
		hook3 = false;
	}

	if (PlayerController->WasInputKeyJustPressed(InsertKey))
	{
		ShowMenu = !ShowMenu;
	}

	//static_canvas->K2_DrawLine(Vector2(width / 2 + 5, height / 2), Vector2(width / 2 + 10.f, height / 2), 3, { 0.6f, 0.6f, 0.6f, 0.6f });
	//static_canvas->K2_DrawLine(Vector2(width / 2 - 5, height / 2), Vector2(wi/dth / 2 - 10.f, height / 2), 3, { 0.6f, 0.6f, 0.6f, 0.6f });
	//static_canvas->K2_DrawLine(Vector2(width / 2, height / 2 + 5), Vector2(width / 2, height / 2 + 10.f), 3, { 0.6f, 0.6f, 0.6f, 0.6f });
	//static_canvas->K2_DrawLine(Vector2(width / 2, height / 2 - 5), Vector2(width / 2, height / 2 - 10.f), 3, { 0.6f, 0.6f, 0.6f, 0.6f });

	if (Settings::FovCircle)
	{
		Pasted_CircleOutline(Vector2(width / 2, height / 2), Settings::FovSize, { 0.6f, 0.6f, 0.6f, 0.6f });
		//DrawCircle(static_canvas, width / 2, height / 2, FovSize, 1450, ue::FLinearColor(0, 0, 1, 1));
	}

	if (ShowMenu)
	{
		DrawMenu();
	}


	return DrawTransitionOriginal(this_, Canvas);
}

bool dumper = false;

VOID Main()
{
	ue::cached::Base = sdkUssage->GetGameBase();

	ue::cached::signatures::GWorld = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 89 05 ?? ?? ?? ?? 0F 28 D7"), 7, true); if (!ue::cached::signatures::GWorld) safe_call(MessageBox)(0, xorstr(L"Failed to find UWorld"), 0, 0);
	ue::cached::signatures::GetBoneMatrix = sdkUssage->PatternScan(ue::cached::Base, xorstr("E8 ? ? ? ? 0F 10 40 68"), 5, true); if (!ue::cached::signatures::GetBoneMatrix) safe_call(MessageBox)(0, xorstr(L"Failed to find GetBoneMatrix"), 0, 0);
	ue::cached::signatures::ProjectWorldToScreen = sdkUssage->PatternScan(ue::cached::Base, xorstr("40 53 55 56 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 33 DB 45 8A F9 49 8B F8 48 8B EA 4C 8B F1 48 85 C9 0F 84 ? ? ? ? 48 8D B1 ? ? ? ? 48 39 1E 74 19 E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 84 C0 74 05 48 8B 0E EB 03"), 5, true); if (!ue::cached::signatures::ProjectWorldToScreen) safe_call(MessageBox)(0, xorstr(L"Failed to find w2s"), 0, 0); //true -> false xD
	ue::cached::signatures::FreeFn = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 89 D1 48 FF 25 ? ? ? ? C3"), 0, false); if (!ue::cached::signatures::FreeFn) safe_call(MessageBox)(0, xorstr(L"Failed to find FreeFn"), 0, 0);
	ue::cached::signatures::GetNameByIndex = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B 19 48 8B FA E8 ? ? ? ? 0F B7 F3 8B D3 48 C1 EA 10 C1 E6"), 0, false); if (!ue::cached::signatures::GetNameByIndex) safe_call(MessageBox)(0, xorstr(L"Failed to find GetNameByIndex"), 0, 0);
	ue::cached::signatures::CalculateSpreadAddress = sdkUssage->PatternScan(ue::cached::Base, xorstr("E8 ? ? ? ? 48 8D 4B 28 E8 ? ? ? ? 48 8B C8"), 5, true); if (!ue::cached::signatures::CalculateSpreadAddress) safe_call(MessageBox)(0, xorstr(L"Failed to find CalculateSpreadAddress"), 0, 0);
	ue::cached::signatures::LineOfSightTo = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 8B C4 48 89 58 20 55 56 57 41 54 41 56 48 8D 68 B8 48 81 EC ?? ?? ?? ??"), 0, false); if (!ue::cached::signatures::LineOfSightTo) safe_call(MessageBox)(0, xorstr(L"Failed to find LineOfSightTo"), 0, 0);
	ue::cached::signatures::CalculateShot = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 44 0F 29 40 ? 44 0F 29 48 ? 44 0F 29 90 ? ? ? ? 44 0F 29 98 ? ? ? ? 44 0F 29 A0 ? ? ? ? 44 0F 29 A8 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ?"), 0, false); if (!ue::cached::signatures::CalculateShot) safe_call(MessageBox)(0, xorstr(L"Failed to find CalculateShot"), 0, 0);
	ue::cached::signatures::GetWeaponStats = sdkUssage->PatternScan(ue::cached::Base, xorstr("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 33 FF 4D 8B F0 48 89 7C 24 ? 48 8B DA 89 7C 24 28 E8 ? ? ? ? 48 8B 43 20 48 85 C0 74 25 0F B6 08 4C 8D 0D ? ? ? ? 48 FF C0 4C 8D 44 24 ? 48 89 43 20 48 8B D3 8B C1 48 8B 4B 18 41 FF 14 C1 EB 1F"), 6, true); if (!ue::cached::signatures::GetWeaponStats) safe_call(MessageBox)(0, xorstr(L"Failed to find GetWeaponStats"), 0, 0);
	ue::classes::objects = (ue::classes::GObjects*)sdkUssage->PatternScan(ue::cached::Base, xorstr("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1"), 7, true); if (!ue::classes::objects) safe_call(MessageBox)(0, xorstr(L"Failed to find GObjects"), 0, 0);

	ue::cached::objects::camera::GetCameraLocation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerCameraManager.GetCameraLocation")); if (!ue::cached::objects::camera::GetCameraLocation) return;
	ue::cached::objects::camera::GetCameraRotation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerCameraManager.GetCameraRotation")); if (!ue::cached::objects::camera::GetCameraRotation) return;
	ue::cached::objects::camera::GetFOVAngle = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerCameraManager.GetFOVAngle")); if (!ue::cached::objects::camera::GetCameraRotation) return;

	ue::cached::objects::render::Font = (ue::classes::UObject*)find::FindObject(xorstr(L"/Engine/EngineFonts/Roboto.Roboto")); if (!ue::cached::objects::render::Font) return;
	ue::cached::objects::render::K2_DrawLine = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Canvas.K2_DrawLine")); if (!ue::cached::objects::render::K2_DrawLine) return;
	ue::cached::objects::render::K2_DrawBox = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Canvas.K2_DrawBox")); if (!ue::cached::objects::render::K2_DrawBox) return;
	ue::cached::objects::render::K2_DrawText = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Canvas.K2_DrawText")); if (!ue::cached::objects::render::K2_DrawText) return;
	ue::cached::objects::render::K2_TextSize = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Canvas.K2_TextSize")); if (!ue::cached::objects::render::K2_DrawBox) return;

	ue::cached::objects::actor::FortPlayerPawnAthena = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortPlayerPawnAthena")); if (!ue::cached::objects::actor::FortPlayerPawnAthena) return;
	ue::cached::objects::actor::ClientSetRotation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Controller.ClientSetRotation")); if (!ue::cached::objects::actor::ClientSetRotation) return;
	ue::cached::objects::actor::GetPlayerName = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerState.GetPlayerName")); if (!ue::cached::objects::actor::GetPlayerName) return;
	ue::cached::objects::actor::ApplyPawnHighlight = (ue::classes::UObject*)find::FindObject(xorstr(L"PlayerPawn_Athena.PlayerPawn_Athena_C.ApplyPawnHighlight")); if (!ue::cached::objects::actor::ApplyPawnHighlight) return;
	ue::cached::objects::actor::Fov = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerController.FOV")); if (!ue::cached::objects::actor::Fov) return;
	ue::cached::objects::actor::SetMouseLocation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerController.SetMouseLocation")); if (!ue::cached::objects::actor::SetMouseLocation) return;
	ue::cached::objects::actor::FortPickupAthena = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortPickupAthena")); if (!ue::cached::objects::actor::FortPickupAthena) return;

	ue::cached::objects::FortProjectileBase = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortProjectileBase")); if (!ue::cached::objects::FortProjectileBase) return;
	ue::cached::objects::FortRangedWeaponStats = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortRangedWeaponStats")); if (!ue::cached::objects::FortRangedWeaponStats) return;

	ue::cached::objects::K2_GetActorRotation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Actor.K2_GetActorRotation")); if (!ue::cached::objects::K2_GetActorRotation) return;

	//ue::cached::objects::actor::isHiddingInProp = (ue::classes::UObject*)find::FindObject(xorstr(L"PlayerPawn_Athena.PlayerPawn_Athena_C.isHiddingInProp")); if (!ue::cached::objects::actor::isHiddingInProp) return;
	//ue::cached::objects::actor::SetCharacterBodyVisibilityForPossession = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortPlayerPawnAthena.SetCharacterBodyVisibilityForPossession")); if (!ue::cached::objects::actor::SetCharacterBodyVisibilityForPossession) return;
	//ue::cached::FortKismetLibrary = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortKismetLibrary")); if (!ue::cached::FortKismetLibrary) return;
	ue::cached::KismetStringLibrary = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.KismetStringLibrary")); if (!ue::cached::KismetStringLibrary) return;
	//ue::cached::KismetMathLibrary = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.KismetMathLibrary")); if (!ue::cached::KismetMathLibrary) return;
	ue::cached::Conv_StringToName = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.KismetStringLibrary.Conv_StringToName")); if (!ue::cached::Conv_StringToName) return;
	//ue::cached::Cos = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.KismetMathLibrary.Cos")); if (!ue::cached::Cos) return;
	//ue::cached::Sin = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.KismetMathLibrary.Sin")); if (!ue::cached::Sin) return;
	ue::cached::GetRangedWeaponStatsRow = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortKismetLibrary.GetRangedWeaponStatsRow")); if (!ue::cached::GetRangedWeaponStatsRow) return;

	ue::cached::objects::actor::K2_PayBuildingResourceCost = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortKismetLibrary.K2_PayBuildingResourceCost")); if (!ue::cached::objects::actor::K2_PayBuildingResourceCost) return;
	ue::cached::objects::actor::GetVehicle = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortPlayerPawn.GetVehicle")); if (!ue::cached::objects::actor::GetVehicle) return;
	//ue::cached::objects::actor::IsInVehicle = (ue::classes::UObject*)find::FindObject(xorstr(L"FortniteGame.FortPlayerPawn.IsInVehicle")); if (!ue::cached::objects::actor::IsInVehicle) return;
	ue::cached::objects::K2_TeleportTo = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Actor.K2_TeleportTo")); if (!ue::cached::objects::K2_TeleportTo) return;
	//ue::cached::objects::SetMouseLoc = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.PlayerController.SetMouseLocation")); if (!ue::cached::objects::SetMouseLoc) return;
	ue::cached::objects::K2_SetActorLocation = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Actor.K2_SetActorLocation")); if (!ue::cached::objects::K2_SetActorLocation) return;
	ue::cached::objects::K2_GetWorldSettings = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.World.K2_GetWorldSettings")); if (!ue::cached::objects::K2_GetWorldSettings) return;
	ue::cached::objects::K2_DestroyActor = (ue::classes::UObject*)find::FindObject(xorstr(L"Engine.Actor.K2_DestroyActor")); if (!ue::cached::objects::K2_DestroyActor) return;

	InsertKey = ue::FKey{ ue::FName{ sdkUssage->Conv_StringToName(xorstr(L"Insert")) }, 0 };
	RBKey = ue::FKey{ ue::FName{ sdkUssage->Conv_StringToName(xorstr(L"RightMouseButton")) }, 0 };
	EnterKey = ue::FKey{ ue::FName{ sdkUssage->Conv_StringToName(xorstr(L"Enter")) }, 0 };

	FNTool* sdk = new FNTool();
	sdk->Initialize((uintptr_t)ue::classes::objects, ue::cached::signatures::GetNameByIndex, ue::cached::signatures::FreeFn);

	ue::cached::offsets::OwningGameInstance = sdk->FindOffset(xorstr("World"), xorstr("OwningGameInstance"));
	ue::cached::offsets::TimeDilation = sdk->FindOffset(xorstr("WorldSettings"), xorstr("TimeDilation"));
	ue::cached::offsets::Levels = sdk->FindOffset(xorstr("World"), xorstr("Levels"));
	ue::cached::offsets::AcknowledgedPawn = sdk->FindOffset(xorstr("PlayerController"), xorstr("AcknowledgedPawn"));
	ue::cached::offsets::PlayerCameraManager = sdk->FindOffset(xorstr("PlayerController"), xorstr("PlayerCameraManager"));
	ue::cached::offsets::PlayerState = sdk->FindOffset(xorstr("Pawn"), xorstr("PlayerState"));
	ue::cached::offsets::bADSWhileNotOnGround = sdk->FindOffset(xorstr("FortPlayerPawnAthena"), xorstr("bADSWhileNotOnGround"));
	ue::cached::offsets::CurrentWeapon = sdk->FindOffset(xorstr("FortPawn"), xorstr("CurrentWeapon"));
	ue::cached::offsets::TeamIndex = sdk->FindOffset(xorstr("FortPlayerStateAthena"), xorstr("TeamIndex"));
	ue::cached::offsets::WeaponData = sdk->FindOffset(xorstr("FortWeapon"), xorstr("WeaponData"));
	ue::cached::offsets::DisplayName = sdk->FindOffset(xorstr("FortItemDefinition"), xorstr("DisplayName"));
	ue::cached::offsets::Mesh = sdk->FindOffset(xorstr("Character"), xorstr("Mesh"));
	ue::cached::offsets::RootComponent = sdk->FindOffset(xorstr("Actor"), xorstr("RootComponent"));
	ue::cached::offsets::RelativeLocation = sdk->FindOffset(xorstr("SceneComponent"), xorstr("RelativeLocation"));
	ue::cached::offsets::LastFireTime = sdk->FindOffset(xorstr("FortWeapon"), ("LastFireTime"));
	ue::cached::offsets::AmmoCount = sdk->FindOffset(xorstr("FortWeapon"), xorstr("AmmoCount"));
	ue::cached::offsets::LastFireTimeVerified = sdk->FindOffset(xorstr("FortWeapon"), xorstr("LastFireTimeVerified"));
	ue::cached::offsets::Tier = sdk->FindOffset(xorstr("FortItemDefinition"), xorstr("Tier"));
	ue::cached::offsets::TriggerType = sdk->FindOffset(xorstr("FortWeaponItemDefinition"), xorstr("TriggerType"));
	ue::cached::offsets::bIsReloadingWeapon = sdk->FindOffset(xorstr("FortWeapon"), xorstr("bIsReloadingWeapon"));
	ue::cached::offsets::VehicleAttributes = sdk->FindOffset(xorstr("FortAthenaVehicle"), xorstr("VehicleAttributes"));
	ue::cached::offsets::ZiplineState = sdk->FindOffset(xorstr("FortPlayerPawn"), xorstr("ZiplineState"));
	ue::cached::offsets::ZiplineSpeedFactor = sdk->FindOffset(xorstr("FortPlayerPawn"), xorstr("ZiplineSpeedFactor"));
	ue::cached::offsets::bIsDying = sdk->FindOffset(xorstr("FortPawn"), xorstr("bIsDying"));
	ue::cached::offsets::ViewTarget = sdk->FindOffset(xorstr("PlayerCameraManager"), xorstr("ViewTarget"));
	ue::cached::offsets::bAlreadySearched = sdk->FindOffset(xorstr("BuildingContainer"), xorstr("bAlreadySearched"));
	ue::cached::offsets::GlobalAnimRateScale = sdk->FindOffset(xorstr("SkeletalMeshComponent"), xorstr("GlobalAnimRateScale"));

	if (dumper)
	{
		AllocConsole();
		freopen(xorstr("CONIN$"), xorstr("r"), stdin);
		freopen(xorstr("CONOUT$"), xorstr("w"), stdout);
		freopen(xorstr("CONOUT$"), xorstr("w"), stderr);

		std::cout << xorstr("GWorld: 0x") << std::hex << std::uppercase << ue::cached::signatures::GWorld - ue::cached::Base << std::endl;

		std::cout << xorstr("OwningGameInstance: 0x") << std::hex << std::uppercase << ue::cached::offsets::OwningGameInstance << std::endl;
		std::cout << xorstr("AcknowledgedPawn: 0x") << std::hex << std::uppercase << ue::cached::offsets::AcknowledgedPawn << std::endl;
		std::cout << xorstr("PlayerState: 0x") << std::hex << std::uppercase << ue::cached::offsets::PlayerState << std::endl;
		std::cout << xorstr("RootComponent: 0x") << std::hex << std::uppercase << ue::cached::offsets::RootComponent << std::endl;
		std::cout << xorstr("Mesh: 0x") << std::hex << std::uppercase << ue::cached::offsets::Mesh << std::endl;
		std::cout << xorstr("TeamIndex: 0x") << std::hex << std::uppercase << ue::cached::offsets::TeamIndex << std::endl;
		std::cout << xorstr("RelativeLocation: 0x") << std::hex << std::uppercase << ue::cached::offsets::RelativeLocation << std::endl;
		std::cout << xorstr("bIsDBNO: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("FortPawn"), xorstr("bIsDBNO")) << std::endl;
		std::cout << xorstr("bIsDying: 0x") << std::hex << std::uppercase << ue::cached::offsets::bIsDying << std::endl;
		std::cout << xorstr("ReviveFromDBNOTime: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("FortPlayerPawnAthena"), xorstr("ReviveFromDBNOTime")) << std::endl;
		std::cout << xorstr("PlayerCameraManager: 0x") << std::hex << std::uppercase << ue::cached::offsets::PlayerCameraManager << std::endl;
		std::cout << xorstr("CustomTimeDilation: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("Actor"), xorstr("CustomTimeDilation")) << std::endl;
		std::cout << xorstr("RelativeRotation: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("SceneComponent"), xorstr("RelativeRotation")) << std::endl;
		std::cout << xorstr("RelativeScale3D: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("SceneComponent"), xorstr("RelativeScale3D")) << std::endl;
		std::cout << xorstr("bComponentToWorldUpdated: 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("SceneComponent"), xorstr("bComponentToWorldUpdated")) << std::endl;
		std::cout << xorstr("DefaultFov(incorrect): 0x") << std::hex << std::uppercase << sdk->FindOffset(xorstr("APlayerCameraManager"), xorstr("DefaultFov")) << std::endl;
		std::cout << xorstr("TimeDilation: 0x") << std::hex << std::uppercase << ue::cached::offsets::TimeDilation << std::endl;
		std::cout << xorstr("ZiplineSpeedFactor: 0x") << std::hex << std::uppercase << ue::cached::offsets::ZiplineSpeedFactor << std::endl;
		std::cout << xorstr("ZiplineState: 0x") << std::hex << std::uppercase << ue::cached::offsets::ZiplineState << std::endl;
		std::cout << xorstr("bADSWhileNotOnGround: 0x") << std::hex << std::uppercase << ue::cached::offsets::bADSWhileNotOnGround << std::endl;
		std::cout << xorstr("CurrentWeapon: 0x") << std::hex << std::uppercase << ue::cached::offsets::CurrentWeapon << std::endl;
		std::cout << xorstr("WeaponData: 0x") << std::hex << std::uppercase << ue::cached::offsets::WeaponData << std::endl;
		std::cout << xorstr("LastFireTime: 0x") << std::hex << std::uppercase << ue::cached::offsets::LastFireTime << std::endl;
		std::cout << xorstr("LastFireTimeVerified: 0x") << std::hex << std::uppercase << ue::cached::offsets::LastFireTimeVerified << std::endl;
		std::cout << xorstr("TriggerType: 0x") << std::hex << std::uppercase << ue::cached::offsets::TriggerType << std::endl;
		std::cout << xorstr("bIsReloadingWeapon: 0x") << std::hex << std::uppercase << ue::cached::offsets::bIsReloadingWeapon << std::endl;
		std::cout << xorstr("VehicleAttributes: 0x") << std::hex << std::uppercase << ue::cached::offsets::VehicleAttributes << std::endl;
		Sleep(-1);
	}

	HMODULE kernel32 = GetModuleHandleA(xorstr("Kernel32.dll"));
	HMODULE winmm = GetModuleHandleA(xorstr("Winmm.dll"));

	// TODO: check if the modules are even loaded.

	// Get all the original addresses of target functions
	g_GetTickCountOriginal = (GetTickCountType)GetProcAddress(kernel32, xorstr("GetTickCount"));
	g_GetTickCount64Original = (GetTickCount64Type)GetProcAddress(kernel32, xorstr("GetTickCount64"));

	g_TimeGetTimeOriginal = (GetTickCountType)GetProcAddress(winmm, xorstr("timeGetTime"));

	g_QueryPerformanceCounterOriginal = (QueryPerformanceCounterType)GetProcAddress(kernel32, xorstr("QueryPerformanceCounter"));

	// Setup the speed hack object for the Performance Counter
	LARGE_INTEGER performanceCounter;
	g_QueryPerformanceCounterOriginal(&performanceCounter);

	g_speedHackLL = SpeedHack<LONGLONG>(performanceCounter.QuadPart, kInitialSpeed);

	// Detour functions
	DetourTransactionBegin();

	DetourAttach((PVOID*)&g_GetTickCountOriginal, (PVOID)GetTickCountHacked);
	DetourAttach((PVOID*)&g_GetTickCount64Original, (PVOID)GetTickCount64Hacked);

	// Detour timeGetTime to the hacked GetTickCount (same signature)
	DetourAttach((PVOID*)&g_TimeGetTimeOriginal, (PVOID)GetTickCountHacked);

	DetourAttach((PVOID*)&g_QueryPerformanceCounterOriginal, (PVOID)QueryPerformanceCounterHacked);

	DetourTransactionCommit();

	Beep(500, 500);

	auto UViewportClient = ((GWorld*)(*(uintptr_t*)(ue::cached::signatures::GWorld)))->OwningGameInstance()->LocalPlayers()->LocalPlayer()->ViewportClient();

	void** DrawTransition_VTable = *(void***)(UViewportClient);
	DWORD OldProtection;
	safe_call(VirtualProtect)(&DrawTransition_VTable[0x6E], 8, PAGE_EXECUTE_READWRITE, &OldProtection);
	DrawTransitionOriginal = decltype(DrawTransitionOriginal)(DrawTransition_VTable[0x6E]);
	DrawTransition_VTable[0x6E] = &DrawTransition;
	safe_call(VirtualProtect)(&DrawTransition_VTable[0x6E], 8, OldProtection, &OldProtection);
	
	DetourAttach((PVOID*)&Spread, (PVOID)SpreadHook);

	//void** CalculateShotAddress = *(void***)(ue::cached::signatures::CalculateShot);
	//DWORD OldProtection3;
	//safe_call(VirtualProtect)(&CalculateShotAddress, 8, PAGE_EXECUTE_READWRITE, &OldProtection3);
	//CalculateShot = decltype(CalculateShot)(CalculateShotAddress);
	//CalculateShotAddress = &CalculateShotHook;
	//safe_call(VirtualProtect)(&CalculateShotAddress, 8, OldProtection3, &OldProtection3);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason != DLL_PROCESS_ATTACH) return FALSE;

	Main();

	return TRUE;
}
