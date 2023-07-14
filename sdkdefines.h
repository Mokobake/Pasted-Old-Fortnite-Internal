#pragma once
#include "vector.h"
#include <xmmintrin.h>
#include <immintrin.h>


namespace ue {
	struct FVector
	{
		double X;
		double Y;
		double Z;
	};

	struct FMatrix
	{
		double M[4][4];
	};
	static FMatrix* myMatrix = new FMatrix();

	template<class T>
	struct TArray
	{
		friend struct FString;

	public:
		inline TArray()
		{
			Data = nullptr;
			Count = Max = 0;
		};

		inline int Num() const
		{
			return Count;
		};

		inline T& operator[](int i)
		{
			return Data[i];
		};

		inline const T& operator[](int i) const
		{
			return Data[i];
		};

		inline bool IsValidIndex(int i) const
		{
			return i < Num();
		}

	private:
		T* Data;
		int32_t Count;
		int32_t Max;
	};

	struct FString : private TArray<wchar_t>
	{
		inline FString()
		{

		};

		FString(const wchar_t* other)
		{
			Max = Count = *other ? wcslen(other) + 1 : 0;

			if (Count)
			{
				Data = const_cast<wchar_t*>(other);
			}
		};

		inline bool IsValid() const
		{
			return Data != nullptr;
		}

		inline const wchar_t* c_str() const
		{
			return Data;
		}
		std::string ToString() const
		{
			auto length = std::wcslen(Data);

			std::string str(length, '\0');

			std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

			return str;
		}
	};

	struct FName
	{
		uint32_t ComparisonIndex;
		uint32_t DisplayIndex;
	};

	struct FKey
	{
		FName KeyName;
		unsigned char Padding_00[0x10];
	};

	struct FLinearColor
	{
		float R;
		float G;
		float B;
		float A;

		FLinearColor()
		{
			R = G = B = A = 0;
		}

		FLinearColor(float R, float G, float B, float A)
		{
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}
	};

	struct FPawnHighlight
	{
		float                                              Priority_28_E2E1B5344846E187B9C11B863A7F0698;             // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
		FLinearColor                                Inner_21_4CC2801147EA190DE16F59B34F36853E;                // 0x0004(0x0010) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
		FLinearColor                                Outer_22_5A1D7D0543D303E8B54B66A7F7BD2E2E;                // 0x0014(0x0010) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
		float                                              FresnelBrightness_23_52B0F96447FF640F47DF2895B0602E92;    // 0x0024(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
		float                                              FresnelExponent_24_B427CF0C441AA37ED49833BF7579DE6D;      // 0x0028(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
		float                                              UsesPulse_25_E29229F64E540F0617E4C4987AD77605;            // 0x002C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	};

	struct FRotator
	{
		double Pitch;
		double Yaw;
		double Roll;
	};

	struct FMinimalViewInfo
	{
		Vector3 Location;
		Vector3 Rotation;
		float FOV;
		float OrthoWidth;
		float OrthoNearClipPlane;
		float OrthoFarClipPlane;
		float AspectRatio;
	};

	class FTextData
	{
	public:
		char pad_0x0000[0x30];  //0x0000
		wchar_t* Name;          //0x0028 
		__int32 Length;         //0x0030 

	};

	struct FText
	{
		FTextData* Data;
		char UnknownData[0x10];

		wchar_t* Get() const {
			if (Data) {
				return Data->Name;
			}

			return nullptr;
		}
	};

	namespace classes
	{
		class UClass
		{
		public:
			BYTE _padding_0[0x40];
			UClass* SuperClass;
		};

		class UObject
		{
		public:
			PVOID VTableObject;
			DWORD ObjectFlags;
			DWORD InternalIndex;
			UClass* Class;
			BYTE _padding_0[0x8];
			UObject* Outer;

			inline BOOLEAN IsA(PVOID parentClass)
			{
				for (auto super = this->Class; super; super = super->SuperClass)
					if (super == parentClass)
						return true;

				return false;
			}
		};

		class UControl
		{
		public:
			Vector2 Origin;
			Vector2 Size;
			bool* BoundBool = nullptr;
			bool bIsMenuTabControl;
			bool bIsRangeSlider;
			int RangeValueMin;
			int RangeValueMax;
			int* pBoundRangeValue;
			int BoundMenuTabIndex;

			bool ContainsPoint(Vector2 pt)
			{
				auto extent = Origin + Size;
				return (pt.x > Origin.x && pt.y > Origin.y && pt.x < extent.x&& pt.y < extent.y);
			}
		};

		class FUObjectItem
		{
		public:
			UObject* Object;
			DWORD Flags;
			DWORD ClusterIndex;
			DWORD SerialNumber;
			DWORD SerialNumber2;
		};

		class TUObjectArray
		{
		public:
			FUObjectItem* Objects[9];
		};

		class GObjects
		{
		public:
			TUObjectArray* ObjectArray;
			BYTE _padding_0[0xC];
			DWORD ObjectCount;
		};
		inline GObjects* objects = nullptr;
	}

	namespace cached
	{
		inline uintptr_t Base = 0;

		namespace keys
		{
			inline ue::FKey LButton;
			inline ue::FKey Insert;
		}

		namespace offsets
		{
			inline uintptr_t OwningGameInstance;
			inline uintptr_t Levels;
			inline uintptr_t AcknowledgedPawn;
			inline uintptr_t PlayerCameraManager;
			inline uintptr_t PlayerState;
			inline uintptr_t bADSWhileNotOnGround;
			inline uintptr_t CurrentWeapon;
			inline uintptr_t TeamIndex;
			inline uintptr_t WeaponData;
			inline uintptr_t DisplayName;
			inline uintptr_t Mesh;
			inline uintptr_t RootComponent;
			inline uintptr_t LastFireTime;
			inline uintptr_t AmmoCount;
			inline uintptr_t LastFireTimeVerified;
			inline uintptr_t RelativeLocation;
			inline uintptr_t TimeDilation;
			inline uintptr_t Tier;
			inline uintptr_t TriggerType;
			inline uintptr_t bIsReloadingWeapon;
			inline uintptr_t VehicleAttributes;
			inline uintptr_t ZiplineState;
			inline uintptr_t ZiplineSpeedFactor;
			inline uintptr_t bIsDying;
			inline uintptr_t ViewTarget;
			inline uintptr_t bAlreadySearched;
			inline uintptr_t GlobalAnimRateScale;
		}

		namespace signatures
		{
			inline uintptr_t GWorld = 0;
			inline uintptr_t FreeFn = 0;
			inline uintptr_t GetNameByIndex = 0;
			inline uintptr_t GetBoneMatrix = 0;
			inline uintptr_t ProjectWorldToScreen = 0;
			inline uintptr_t CalculateSpreadAddress = 0;
			inline uintptr_t LineOfSightTo = 0;
			inline uintptr_t CalculateShot = 0;
			inline uintptr_t GetWeaponStats = 0;
		}

		inline  ue::classes::UObject* FortKismetLibrary = 0;
		inline  ue::classes::UObject* KismetStringLibrary = 0;
		inline  ue::classes::UObject* KismetMathLibrary = 0;
		inline  ue::classes::UObject* Conv_StringToName = 0;
		inline  ue::classes::UObject* Cos = 0;
		inline  ue::classes::UObject* Sin = 0;
		inline  ue::classes::UObject* GetRangedWeaponStatsRow = 0;

		namespace objects
		{
			inline ue::classes::UObject* FortProjectileBase = 0;
			inline ue::classes::UObject* K2_SetActorLocation = 0;
			inline ue::classes::UObject* K2_DestroyActor = 0;
			inline ue::classes::UObject* K2_GetWorldSettings = 0;
			inline ue::classes::UObject* K2_TeleportTo = 0;
			inline ue::classes::UObject* FortRangedWeaponStats = 0;
			inline ue::classes::UObject* K2_GetActorRotation = 0;

			namespace actor
			{
				inline  ue::classes::UObject* FortPickupAthena = 0;
				inline  ue::classes::UObject* FortPlayerPawnAthena = 0;
				inline  ue::classes::UObject* ClientSetRotation = 0;
				inline  ue::classes::UObject* GetPlayerName = 0;
				inline  ue::classes::UObject* ApplyPawnHighlight = 0;
				inline  ue::classes::UObject* ScalePlayerUp = 0;
				inline  ue::classes::UObject* ScalePlayerDown = 0;
				inline  ue::classes::UObject* isHiddingInProp = 0;
				inline  ue::classes::UObject* Fov = 0;
				inline  ue::classes::UObject* SetMouseLocation = 0;
				inline  ue::classes::UObject* SetCharacterBodyVisibilityForPossession = 0;
				inline  ue::classes::UObject* K2_PayBuildingResourceCost = 0;
				inline  ue::classes::UObject* GetVehicle = 0;
			}

			namespace camera
			{
				inline ue::classes::UObject* GetCameraLocation = 0; 
				inline ue::classes::UObject* GetCameraRotation = 0;
				inline ue::classes::UObject* GetFOVAngle = 0;
			}
			
			namespace render
			{
				inline ue::classes::UObject* Font = 0;
				inline ue::classes::UObject* K2_DrawLine = 0;
				inline ue::classes::UObject* K2_DrawBox = 0;
				inline ue::classes::UObject* K2_DrawText = 0;
				inline ue::classes::UObject* K2_TextSize = 0;
			}
		}
	}
}

namespace math
{
	inline float custom_sqrtf(float _X)
	{
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_X)));
	}

	inline float custom_sinf(float _X)
	{
		return _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(_X)));
	}

	inline float custom_cosf(float _X)
	{
		return _mm_cvtss_f32(_mm_cos_ps(_mm_set_ss(_X)));
	}

	inline float custom_acosf(float _X)
	{
		return _mm_cvtss_f32(_mm_acos_ps(_mm_set_ss(_X)));
	}

	inline float custom_tanf(float _X)
	{
		return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(_X)));
	}

	inline float custom_atan2f(float _X, float _Y)
	{
		return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(_X), _mm_set_ss(_Y)));
	}

	inline int custom_compare(const char* X, const char* Y)
	{
		while (*X && *Y) {
			if (*X != *Y) {
				return 0;
			}
			X++;
			Y++;
		}

		return (*Y == '\0');
	}

	inline int custom_wcompare(const wchar_t* X, const wchar_t* Y)
	{
		while (*X && *Y) {
			if (*X != *Y) {
				return 0;
			}
			X++;
			Y++;
		}

		return (*Y == L'\0');
	}

	inline const wchar_t* custom_wcsstr(const wchar_t* X, const wchar_t* Y)
	{
		while (*X != L'\0') {
			if ((*X == *Y) && custom_wcompare(X, Y)) {
				return X;
			}
			X++;
		}
		return NULL;
	}

	inline const char* custom_strstr(const char* X, const char* Y)
	{
		while (*X != '\0') {
			if ((*X == *Y) && custom_compare(X, Y)) {
				return X;
			}
			X++;
		}
		return NULL;
	}

	inline int custom_strlen(const char* string)
	{
		int cnt = 0;
		if (string)
		{
			for (; *string != 0; ++string) ++cnt;
		}
		return cnt;
	}

	inline int custom_wcslen(const wchar_t* string)
	{
		int cnt = 0;
		if (string)
		{
			for (; *string != 0; ++string) ++cnt;
		}
		return cnt;
	}
}
