#include "stdafx.h"
#include <string>
#include <sstream>
#include <algorithm>
#include "LibretroRenderer.h"
#include "LibretroSoundManager.h"
#include "LibretroKeyManager.h"
#include "LibretroMessageManager.h"
#include "libretro.h"
#include "../Core/Console.h"
#include "../Core/VideoDecoder.h"
#include "../Core/VideoRenderer.h"
#include "../Core/MemoryManager.h"
#include "../Core/BaseMapper.h"
#include "../Core/EmulationSettings.h"
#include "../Core/CheatManager.h"
#include "../Core/HdData.h"
#include "../Core/SaveStateManager.h"
#include "../Core/DebuggerTypes.h"
#include "../Core/GameDatabase.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/HexUtilities.h"

#define DEVICE_AUTO               RETRO_DEVICE_JOYPAD
#define DEVICE_GAMEPAD            RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define DEVICE_POWERPAD           RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 1)
#define DEVICE_FAMILYTRAINER      RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 2)
#define DEVICE_PARTYTAP           RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 3)
#define DEVICE_PACHINKO           RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 4)
#define DEVICE_EXCITINGBOXING     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 5)
#define DEVICE_KONAMIHYPERSHOT    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 6)
#define DEVICE_SNESGAMEPAD        RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 7)
#define DEVICE_VBGAMEPAD          RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 8)
#define DEVICE_ZAPPER             RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_POINTER, 0)
#define DEVICE_OEKAKIDS           RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_POINTER, 1)
#define DEVICE_BANDAIHYPERSHOT    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_POINTER, 2)
#define DEVICE_ARKANOID           RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 0)
#define DEVICE_HORITRACK          RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 1)
#define DEVICE_SNESMOUSE          RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 2)
#define DEVICE_ASCIITURBOFILE     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_NONE, 0)
#define DEVICE_BATTLEBOX          RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_NONE, 1)
#define DEVICE_FOURPLAYERADAPTER  RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_NONE, 2)

static retro_log_printf_t logCallback = nullptr;
static retro_environment_t retroEnv = nullptr;
static unsigned _inputDevices[5] = { DEVICE_AUTO, DEVICE_AUTO, DEVICE_AUTO, DEVICE_AUTO, DEVICE_AUTO };
static bool _hdPacksEnabled = false;
static string _mesenVersion = "";
static int32_t _saveStateSize = -1;
static bool _shiftButtonsClockwise = false;
static int32_t _audioSampleRate = 44100;

//Include game database as a byte array (representing the MesenDB.txt file)
#include "MesenDB.inc"

static std::shared_ptr<Console> _console;
static std::unique_ptr<LibretroRenderer> _renderer;
static std::unique_ptr<LibretroSoundManager> _soundManager;
static std::unique_ptr<LibretroKeyManager> _keyManager;
static std::unique_ptr<LibretroMessageManager> _messageManager;

static constexpr const char* MesenNtscFilter = "mesen_ntsc_filter";
static constexpr const char* MesenPalette = "mesen_palette";
static constexpr const char* MesenNoSpriteLimit = "mesen_nospritelimit";
static constexpr const char* MesenOverclock = "mesen_overclock";
static constexpr const char* MesenOverclockType = "mesen_overclock_type";
static constexpr const char* MesenOverscanLeft = "mesen_overscan_left";
static constexpr const char* MesenOverscanRight = "mesen_overscan_right";
static constexpr const char* MesenOverscanTop = "mesen_overscan_up";
static constexpr const char* MesenOverscanBottom = "mesen_overscan_down";
static constexpr const char* MesenAspectRatio = "mesen_aspect_ratio";
static constexpr const char* MesenRegion = "mesen_region";
static constexpr const char* MesenRamState = "mesen_ramstate";
static constexpr const char* MesenControllerTurboSpeed = "mesen_controllerturbospeed";
static constexpr const char* MesenFdsAutoSelectDisk = "mesen_fdsautoinsertdisk";
static constexpr const char* MesenFdsFastForwardLoad = "mesen_fdsfastforwardload";
static constexpr const char* MesenHdPacks = "mesen_hdpacks";
static constexpr const char* MesenScreenRotation = "mesen_screenrotation";
static constexpr const char* MesenFakeStereo = "mesen_fake_stereo";
static constexpr const char* MesenDisableSquarePhaseReset = "mesen_disable_square_phase";
static constexpr const char* MesenNonLinearSquareMixer = "mesen_non_linear_square_mixer";
static constexpr const char* MesenSwapDutyCycle = "mesen_swap_duty_cycle";
static constexpr const char* MesenDisableNoiseModeFlag = "mesen_disable_noise_mode_flag";
static constexpr const char* MesenShiftButtonsClockwise = "mesen_shift_buttons_clockwise";
static constexpr const char* MesenAudioSampleRate = "mesen_audio_sample_rate";

uint32_t defaultPalette[0x40] { 0xFF6E6E6E, 0xFF001E96, 0xFF2800A0, 0xFF41008C, 0xFF5A005F, 0xFF64001E, 0xFF5F0F00, 0xFF501E00, 0xFF2D3700, 0xFF0F4600, 0xFF004B00, 0xFF00461E, 0xFF00375F, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB4B4B4, 0xFF005AE6, 0xFF4637F0, 0xFF781EEB, 0xFFA500B4, 0xFFBE005F, 0xFFB92800, 0xFF965000, 0xFF6E7300, 0xFF2D8700, 0xFF008C00, 0xFF008741, 0xFF00789B, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF5AAFFF, 0xFF8796FF, 0xFFBE82FF, 0xFFEB6EFF, 0xFFFF6EC8, 0xFFFF8269, 0xFFEBA500, 0xFFC8C300, 0xFF78DC00, 0xFF3CE61E, 0xFF2DE178, 0xFF32D2DC, 0xFF505050, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB9E1FF, 0xFFCDD2FF, 0xFFE6C8FF, 0xFFF5C3FF, 0xFFFFC3EB, 0xFFFFCDBE, 0xFFF5DCAA, 0xFFEBE6A0, 0xFFC8F0A0, 0xFFAFF5AA, 0xFFAAF0C8, 0xFFA0EBF0, 0xFFBEBEBE, 0xFF000000, 0xFF000000 };
uint32_t compositeDirectPalette[0x40]{ 0xFF696969, 0xFF00148F, 0xFF1E029B, 0xFF3F008A, 0xFF600060, 0xFF660017, 0xFF570D00, 0xFF451B00, 0xFF243400, 0xFF034200, 0xFF004500, 0xFF003C1F, 0xFF00315C, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAFAFAF, 0xFF0F51DD, 0xFF442FF3, 0xFF7220E2, 0xFFA319B3, 0xFFAE1C51, 0xFFA43400, 0xFF884D00, 0xFF676D00, 0xFF208000, 0xFF008B00, 0xFF007F42, 0xFF006C97, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF65AAFF, 0xFF8C96FF, 0xFFB983FF, 0xFFDD6FFF, 0xFFEA6FBD, 0xFFEB8466, 0xFFDCA21F, 0xFFBAB403, 0xFF7ECB07, 0xFF54D33E, 0xFF3CD284, 0xFF3EC7CC, 0xFF4B4B4B, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBDE2FF, 0xFFCECFFF, 0xFFE6C2FF, 0xFFF6BCFF, 0xFFF9C2ED, 0xFFFACFC6, 0xFFF8DEAC, 0xFFEEE9A1, 0xFFD0F59F, 0xFFBBF5AF, 0xFFB3F5CD, 0xFFB9EDF0, 0xFFB9B9B9, 0xFF000000, 0xFF000000 };
uint32_t nesClassicPalette[0x40]{ 0xFF6B6B6B, 0xFF001B87, 0xFF21009A, 0xFF40008C, 0xFF600067, 0xFF64001E, 0xFF590800, 0xFF461600, 0xFF263600, 0xFF024500, 0xFF004700, 0xFF00421D, 0xFF003659, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB4B4B4, 0xFF1555CE, 0xFF4337EA, 0xFF7124DA, 0xFF9C1AB6, 0xFFAA1164, 0xFFA82E00, 0xFF874B00, 0xFF666B00, 0xFF218300, 0xFF008A00, 0xFF008144, 0xFF007691, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF63AFFF, 0xFF8296FF, 0xFFC07DFE, 0xFFE977FF, 0xFFF572CD, 0xFFF4886B, 0xFFDDA029, 0xFFBDBD0A, 0xFF89D20E, 0xFF5CDE3E, 0xFF4BD886, 0xFF4DCFD2, 0xFF505050, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBEE1FF, 0xFFD2D4FF, 0xFFE3CAFF, 0xFFF0C9FF, 0xFFFFC6E3, 0xFFFFCEC9, 0xFFF4DCAF, 0xFFEBE5A1, 0xFFD2EFA2, 0xFFBEF4B5, 0xFFB8F1D0, 0xFFB8EDF1, 0xFFBDBDBD, 0xFF000000, 0xFF000000 };
uint32_t nestopiaRgbPalette[0x40]{ 0xFF515151, 0xFF0C0E8A, 0xFF140D88, 0xFF360A6D, 0xFF480745, 0xFF51050A, 0xFF510600, 0xFF401C07, 0xFF242804, 0xFF0C2D04, 0xFF062D04, 0xFF072C29, 0xFF09245B, 0xFF000000, 0xFF020202, 0xFF020202, 0xFF8E8E8E, 0xFF2843CE, 0xFF4624F5, 0xFF661FDB, 0xFF871AA0, 0xFF9B154A, 0xFF912D03, 0xFF7B4100, 0xFF5A5101, 0xFF295C06, 0xFF125E00, 0xFF135C42, 0xFF175589, 0xFF020202, 0xFF020202, 0xFF020202, 0xFFE9E9E9, 0xFF528DFF, 0xFF8082FF, 0xFFA972FF, 0xFFC267EF, 0xFFD567A6, 0xFFD76F4A, 0xFFC17E00, 0xFF9E8E00, 0xFF639D00, 0xFF26A326, 0xFF27A16C, 0xFF289BB8, 0xFF393939, 0xFF020202, 0xFF020202, 0xFFE9E9E9, 0xFFA2C3F3, 0xFFBBBBFF, 0xFFCBB5FF, 0xFFD8B3ED, 0xFFE2B1D9, 0xFFE1B6AB, 0xFFD9BB91, 0xFFCAC17F, 0xFFB1C882, 0xFFA1CB98, 0xFF95CCB2, 0xFF9AC8D2, 0xFFA8A8A8, 0xFF020202, 0xFF020202 };
uint32_t originalHardwarePalette[0x40]{ 0xFF606060, 0xFF000088, 0xFF200C98, 0xFF381478, 0xFF541460, 0xFF5C0010, 0xFF541000, 0xFF3C2408, 0xFF20340C, 0xFF0C400C, 0xFF184418, 0xFF003C20, 0xFF003058, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFA8A8A8, 0xFF0C4CC4, 0xFF4C24E0, 0xFF6814D0, 0xFF9014AC, 0xFF9C1C48, 0xFF903400, 0xFF745004, 0xFF5C6814, 0xFF187C10, 0xFF148008, 0xFF107448, 0xFF1C6490, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFCFCFC, 0xFF6498FC, 0xFF887CFC, 0xFFB068FC, 0xFFDC6CF4, 0xFFE870AC, 0xFFE48858, 0xFFCC9C20, 0xFFA8B000, 0xFF74C000, 0xFF5CCC50, 0xFF34C090, 0xFF50C0CC, 0xFF404040, 0xFF000000, 0xFF000000, 0xFFFCFCFC, 0xFFBCD4FC, 0xFFCCCCFC, 0xFFD8C4FC, 0xFFECC0FC, 0xFFF8C4E8, 0xFFF8CCC4, 0xFFE4CCA8, 0xFFD8DC9C, 0xFFC8E4A0, 0xFFC0E4B8, 0xFFB4ECC8, 0xFFB8E4EC, 0xFFBABABA, 0xFF000000, 0xFF000000 };
uint32_t pvmStylePalette[0x40]{ 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0700, 0xFF561D00, 0xFF333500, 0xFF0C4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0D9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF26AFF, 0xFFFF6ECC, 0xFFFF8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFAC2FF, 0xFFFFC4EA, 0xFFFFCCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };
uint32_t sonyCxa2025AsPalette[0x40]{ 0xFF626262, 0xFF002B96, 0xFF1210BA, 0xFF3E00B6, 0xFF63008A, 0xFF770043, 0xFF750300, 0xFF5C1C00, 0xFF353700, 0xFF084C00, 0xFF005700, 0xFF005403, 0xFF004453, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFABABAB, 0xFF0860F1, 0xFF3E3BFF, 0xFF7A1DFF, 0xFFAD0FE2, 0xFFC81381, 0xFFC42914, 0xFFA34B00, 0xFF6D7000, 0xFF308D00, 0xFF009C00, 0xFF00982A, 0xFF008296, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF56B1FF, 0xFF8E8BFF, 0xFFCD6CFF, 0xFFFF5DFF, 0xFFFF62D4, 0xFFFF7863, 0xFFF79C05, 0xFFBFC200, 0xFF80E100, 0xFF4CF015, 0xFF30EB79, 0xFF33D4EA, 0xFF4D4D4D, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBBE0FF, 0xFFD2D1FF, 0xFFEBC4FF, 0xFFFFBEFF, 0xFFFFC0EE, 0xFFFFC9C1, 0xFFFCD79B, 0xFFE6E786, 0xFFCCF389, 0xFFB7F9A1, 0xFFACF7CA, 0xFFAEEEF7, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };
uint32_t unsaturatedPalette[0x40] { 0xFF585858, 0xFF00238C, 0xFF00139B, 0xFF2D0585, 0xFF5D0052, 0xFF7A0017, 0xFF7A0800, 0xFF5F1800, 0xFF352A00, 0xFF093900, 0xFF003F00, 0xFF003C22, 0xFF00325D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFA1A1A1, 0xFF0053EE, 0xFF153CFE, 0xFF6028E4, 0xFFA91D98, 0xFFD41E41, 0xFFD22C00, 0xFFAA4400, 0xFF6C5E00, 0xFF2D7300, 0xFF007D06, 0xFF007852, 0xFF0069A9, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF1FA5FE, 0xFF5E89FE, 0xFFB572FE, 0xFFFE65F6, 0xFFFE6790, 0xFFFE773C, 0xFFFE9308, 0xFFC4B200, 0xFF79CA10, 0xFF3AD54A, 0xFF11D1A4, 0xFF06BFFE, 0xFF424242, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFA0D9FE, 0xFFBDCCFE, 0xFFE1C2FE, 0xFFFEBCFB, 0xFFFEBDD0, 0xFFFEC5A9, 0xFFFED18E, 0xFFE9DE86, 0xFFC7E992, 0xFFA8EEB0, 0xFF95ECD9, 0xFF91E4FE, 0xFFACACAC, 0xFF000000, 0xFF000000 };
uint32_t yuvPalette[0x40] { 0xFF626262, 0xFF002C7D, 0xFF11169E, 0xFF36039E, 0xFF56007D, 0xFF680044, 0xFF680602, 0xFF561A00, 0xFF363100, 0xFF114300, 0xFF004E00, 0xFF004C02, 0xFF004044, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFAAAAAA, 0xFF1061D0, 0xFF3C43FC, 0xFF6E29FC, 0xFF9A1BD0, 0xFFB31D82, 0xFFB32D28, 0xFF9A4900, 0xFF6E6700, 0xFF3C8100, 0xFF108F00, 0xFF008D28, 0xFF007C82, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF5EB2FF, 0xFF8C93FF, 0xFFC078FF, 0xFFED6AFF, 0xFFFF6CD4, 0xFFFF7D77, 0xFFED9927, 0xFFC0B900, 0xFF8CD400, 0xFF5EE227, 0xFF44E077, 0xFF44CFD4, 0xFF4D4D4D, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFBEE0FF, 0xFFD0D3FF, 0xFFE5C9FF, 0xFFF8C3FF, 0xFFFFC3EE, 0xFFFFCAC8, 0xFFF8D6A8, 0xFFE5E395, 0xFFD0ED95, 0xFFBEF3A8, 0xFFB4F2C8, 0xFFB4EBEE, 0xFFB7B7B7, 0xFF000000, 0xFF000000 };
uint32_t wavebeamPalette[0x40] { 0xFF737373, 0xFF002194, 0xFF1800A5, 0xFF39008C, 0xFF52006B, 0xFF630042, 0xFF630000, 0xFF521800, 0xFF393100, 0xFF184200, 0xFF084A00, 0xFF004A18, 0xFF003939, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB5B5B5, 0xFF0063D6, 0xFF2142F7, 0xFF7300FF, 0xFF9C00C6, 0xFFBD0084, 0xFFC60000, 0xFFA54A00, 0xFF7B6B00, 0xFF398400, 0xFF108C00, 0xFF008C39, 0xFF007B7B, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF4AADFF, 0xFF6B94FF, 0xFFAD84FF, 0xFFDE73FF, 0xFFFF6BDE, 0xFFFF7B7B, 0xFFF79C00, 0xFFD6BD00, 0xFF84DE00, 0xFF29EF00, 0xFF00E76B, 0xFF00D6D6, 0xFF525252, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFADDEFF, 0xFFBDCEFF, 0xFFD6C6FF, 0xFFEFBDFF, 0xFFFFB5EF, 0xFFFFC6C6, 0xFFF7D6B5, 0xFFEFE7A5, 0xFFCEF794, 0xFFADFFA5, 0xFF9CFFB5, 0xFF8CF7F7, 0xFFC6C6C6, 0xFF000000, 0xFF000000 };

extern "C" {
	void logMessage(retro_log_level level, const char* message)
	{
		if(logCallback) {
			logCallback(level, message);
		}
	}

	RETRO_API unsigned retro_api_version()
	{
		return RETRO_API_VERSION;
	}

	RETRO_API void retro_init()
	{
		struct retro_log_callback log;
		if(retroEnv(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) {
			logCallback = log.log;
		} else {
			logCallback = nullptr;
		}

		_console.reset(new Console());
		_console->Init();

		_renderer.reset(new LibretroRenderer(_console, retroEnv));
		_soundManager.reset(new LibretroSoundManager(_console));
		_keyManager.reset(new LibretroKeyManager(_console));
		_messageManager.reset(new LibretroMessageManager(logCallback, retroEnv));

		std::stringstream databaseData;
		databaseData.write((const char*)MesenDatabase, sizeof(MesenDatabase));
		GameDatabase::LoadGameDb(databaseData);

		_console->GetSettings()->SetFlags(EmulationFlags::FdsAutoLoadDisk);
		_console->GetSettings()->SetFlags(EmulationFlags::AutoConfigureInput);
		_console->GetSettings()->SetSampleRate(_audioSampleRate);
		_console->GetSettings()->SetAutoSaveOptions(0, false);
		_console->GetSettings()->SetRewindBufferSize(0);
	}

	RETRO_API void retro_deinit()
	{
		_renderer.reset();
		_soundManager.reset();
		_keyManager.reset();
		_messageManager.reset();

		_console->SaveBatteries();
		_console->Release(true);
		_console.reset();
	}

	RETRO_API void retro_set_environment(retro_environment_t env)
	{
		retroEnv = env;

		static constexpr struct retro_variable vars[] = {
			{ MesenNtscFilter, "NTSC filter; Disabled|Composite (Blargg)|S-Video (Blargg)|RGB (Blargg)|Monochrome (Blargg)|Bisqwit 2x|Bisqwit 4x|Bisqwit 8x" },
			{ MesenPalette, "Palette; Default|Nestopia YUV|Nintendulator NTSC|Sony CXA2025AS|Zero Degree YUV|NES Classic|Digital Prime (by FirebrandX)|Wavebeam (by nakedarthur)|Kizul's Definitive NTSC-U NES|Palightful (by Yave Yu)|Custom|Raw" },
			{ MesenOverclock, "Overclock; None|Low|Medium|High|Very High" },
			{ MesenOverclockType, "Overclock Type; Before NMI (Recommended)|After NMI" },
			{ MesenRegion, "Region; Auto|NTSC|PAL|Dendy" },
			{ MesenOverscanLeft, "Left Overscan; None|4px|8px|12px|16px" },
			{ MesenOverscanRight, "Right Overscan; None|4px|8px|12px|16px" },
			{ MesenOverscanTop, "Top Overscan; None|4px|8px|12px|16px" },
			{ MesenOverscanBottom, "Bottom Overscan; None|4px|8px|12px|16px" },
			{ MesenAspectRatio, "Aspect Ratio; Auto|No Stretching|NTSC|PAL|4:3|16:9" },
			{ MesenControllerTurboSpeed, "Controller Turbo Speed; Fast|Very Fast|Disabled|Slow|Normal" },
			{ MesenShiftButtonsClockwise, u8"Shift A/B/X/Y clockwise; disabled|enabled" },
			{ MesenHdPacks, "Enable HD Packs; enabled|disabled" },
			{ MesenNoSpriteLimit, "Remove sprite limit; disabled|enabled" },
			{ MesenFakeStereo, u8"Enable fake stereo effect; disabled|enabled" },
			{ MesenDisableSquarePhaseReset, u8"Not reset square channels phase; enabled|disabled" },
			{ MesenNonLinearSquareMixer, u8"Use non-linear square channel mixer; enabled|disabled" },
			{ MesenSwapDutyCycle, u8"Swap Square channel duty cycles; disabled|enabled" },
			{ MesenDisableNoiseModeFlag, u8"Disable Noise channel mode flag; disabled|enabled" },
			{ MesenScreenRotation, u8"Screen Rotation; None|90 degrees|180 degrees|270 degrees" },
			{ MesenRamState, "Default power-on state for RAM; All 0s (Default)|All 1s|Random Values" },
			{ MesenFdsAutoSelectDisk, "FDS: Automatically insert disks; disabled|enabled" },
			{ MesenFdsFastForwardLoad, "FDS: Fast forward while loading; disabled|enabled" },
			{ MesenAudioSampleRate, "Sound Output Sample Rate; 96000|192000|384000|11025|22050|44100|48000" },
			{ NULL, NULL },
		};

		static constexpr struct retro_controller_description pads1[] = {
			{ "Auto", DEVICE_AUTO },
			{ "Standard Controller", DEVICE_GAMEPAD },
			{ "Zapper", DEVICE_ZAPPER },
			{ "Power Pad", DEVICE_POWERPAD },
			{ "Arkanoid", DEVICE_ARKANOID },
			{ "SNES Controller", DEVICE_SNESGAMEPAD },
			{ "SNES Mouse", DEVICE_SNESMOUSE },
			{ "Virtual Boy Controller" ,DEVICE_VBGAMEPAD },
			{ NULL, 0 },
		};

		static constexpr struct retro_controller_description pads2[] = {
			{ "Auto", DEVICE_AUTO },
			{ "Standard Controller", DEVICE_GAMEPAD },
			{ "Zapper", DEVICE_ZAPPER },
			{ "Power Pad", DEVICE_POWERPAD },
			{ "Arkanoid", DEVICE_ARKANOID },
			{ "SNES Controller", DEVICE_SNESGAMEPAD },
			{ "SNES Mouse", DEVICE_SNESMOUSE },
			{ "Virtual Boy Controller", DEVICE_VBGAMEPAD },
			{ NULL, 0 },
		};

		static constexpr struct retro_controller_description pads3[] = {
			{ "Auto", DEVICE_AUTO },
			{ "Standard Controller", DEVICE_GAMEPAD },
			{ NULL, 0 },
		};

		static constexpr struct retro_controller_description pads4[] = {
			{ "Auto", DEVICE_AUTO },
			{ "Standard Controller", DEVICE_GAMEPAD },
			{ NULL, 0 },
		};
		
		static constexpr struct retro_controller_description pads5[] = {
			{ "Auto",     RETRO_DEVICE_JOYPAD },
			{ "Arkanoid", DEVICE_ARKANOID },
			{ "Ascii Turbo File", DEVICE_ASCIITURBOFILE },
			{ "Bandai Hypershot", DEVICE_BANDAIHYPERSHOT },
			{ "Battle Box", DEVICE_BATTLEBOX },
			{ "Exciting Boxing", DEVICE_EXCITINGBOXING },
			{ "Family Trainer", DEVICE_FAMILYTRAINER },
			{ "Four Player Adapter", DEVICE_FOURPLAYERADAPTER },
			{ "Hori Track", DEVICE_HORITRACK },
			{ "Konami Hypershot", DEVICE_KONAMIHYPERSHOT },
			{ "Pachinko", DEVICE_PACHINKO },
			{ "Partytap", DEVICE_PARTYTAP },
			{ "Oeka Kids Tablet", DEVICE_OEKAKIDS },			
			{ NULL, 0 },
		};
		
		static constexpr struct retro_controller_info ports[] = {
			{ pads1, 7 },
			{ pads2, 7 },
			{ pads3, 2 },
			{ pads4, 2 },
			{ pads5, 13 },
			{ 0 },
		};

		static const struct retro_system_content_info_override content_overrides[] = {
			{
				"nes|fds|unf|unif", /* extensions */
				false,              /* need_fullpath */
				false               /* persistent_data */
			},
			{ NULL, false, false }
		};

		retroEnv(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
		retroEnv(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
		retroEnv(RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE, (void*)content_overrides);
	}

	RETRO_API void retro_set_video_refresh(retro_video_refresh_t sendFrame)
	{
		_renderer->SetVideoCallback(sendFrame);
	}

	RETRO_API void retro_set_audio_sample(retro_audio_sample_t sendAudioSample)
	{
	}

	RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t audioSampleBatch)
	{
		_soundManager->SetSendAudioSample(audioSampleBatch);
	}

	RETRO_API void retro_set_input_poll(retro_input_poll_t pollInput)
	{	
		_keyManager->SetPollInput(pollInput);
	}

	RETRO_API void retro_set_input_state(retro_input_state_t getInputState)
	{
		_keyManager->SetGetInputState(getInputState);
	}

	RETRO_API void retro_reset()
	{
		_console->Reset(true);
	}

	bool readVariable(const char* key, retro_variable &var)
	{
		var.key = key;
		var.value = nullptr;
		if(retroEnv(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value != nullptr) {
			return true;
		}
		return false;
	}

	uint8_t readOverscanValue(const char* key)
	{
		retro_variable var = {};
		if(readVariable(key, var)) {
			string value = string(var.value);
			if(value == "4px") {
				return 4;
			} else if(value == "8px") {
				return 8;
			} else if(value == "12px") {
				return 12;
			} else if(value == "16px") {
				return 16;
			}
		}
		return 0;
	}

	void set_flag(const char* flagName, uint64_t flagValue)
	{
		struct retro_variable var = {};
		if(readVariable(flagName, var)) {
			string value = string(var.value);
			if(value == "disabled") {
				_console->GetSettings()->ClearFlags(flagValue);
			} else {
				_console->GetSettings()->SetFlags(flagValue);
			}
		}
	}

	void load_custom_palette()
	{
		//Setup default palette in case we can't load the custom one
		_console->GetSettings()->SetUserRgbPalette(defaultPalette);

		//Try to load the custom palette from the MesenPalette.pal file
		string palettePath = FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "MesenPalette.pal");
		uint8_t fileData[512 * 3] = {};
		ifstream palette(palettePath, ios::binary);
		if(palette) {
			palette.seekg(0, ios::end);
			std::streamoff fileSize = palette.tellg();
			palette.seekg(0, ios::beg);
			if((fileSize == 64 * 3) || (fileSize == 512 * 3)) {
				palette.read((char*)fileData, fileSize);
				uint32_t customPalette[512];
				for(int i = 0; i < fileSize / 3; i++) {
					customPalette[i] = 0xFF000000 | fileData[i * 3 + 2] | (fileData[i * 3 + 1] << 8) | (fileData[i * 3] << 16);
				}
				_console->GetSettings()->SetUserRgbPalette(customPalette, (uint32_t)fileSize / 3);
			}
		}
	}

	void update_settings()
	{
		struct retro_variable var = { };
		_console->GetSettings()->SetPictureSettings(0, 0, 0, 0, 0);

		_hdPacksEnabled = _console->GetSettings()->CheckFlag(EmulationFlags::UseHdPacks);

		set_flag(MesenNoSpriteLimit, EmulationFlags::RemoveSpriteLimit | EmulationFlags::AdaptiveSpriteLimit);
		set_flag(MesenHdPacks, EmulationFlags::UseHdPacks);
		set_flag(MesenDisableSquarePhaseReset, EmulationFlags::DisableSquarePhaseReset);
		set_flag(MesenNonLinearSquareMixer, EmulationFlags::NonLinearSquareMixer);
		set_flag(MesenSwapDutyCycle, EmulationFlags::SwapDutyCycles);
		set_flag(MesenDisableNoiseModeFlag, EmulationFlags::DisableNoiseModeFlag);
		set_flag(MesenFdsAutoSelectDisk, EmulationFlags::FdsAutoInsertDisk);
		set_flag(MesenFdsFastForwardLoad, EmulationFlags::FdsFastForwardOnLoad);

		if(readVariable(MesenFakeStereo, var)) {
			string value = string(var.value);
			AudioFilterSettings settings;
			if(value == "enabled") {
				settings.Filter = StereoFilter::Delay;
				settings.Delay = 15;
				_console->GetSettings()->SetAudioFilterSettings(settings);
			} else {
				_console->GetSettings()->SetAudioFilterSettings(settings);
			}
		}
		
		if(readVariable(MesenNtscFilter, var)) {
			string value = string(var.value);
			if(value == "Disabled") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::None);
			} else if(value == "Composite (Blargg)") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::NTSC);
				_console->GetSettings()->SetNtscFilterSettings(0, 0, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "S-Video (Blargg)") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::NTSC);
				_console->GetSettings()->SetNtscFilterSettings(-1.0, 0, -1.0, 0, 0.2, 0.2, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "RGB (Blargg)") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::NTSC);
				_console->GetSettings()->SetPictureSettings(0, 0, 0, 0, 0);
				_console->GetSettings()->SetNtscFilterSettings(-1.0, -1.0, -1.0, 0, 0.7, 0.2, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "Monochrome (Blargg)") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::NTSC);
				_console->GetSettings()->SetPictureSettings(0, 0, -1.0, 0, 0);
				_console->GetSettings()->SetNtscFilterSettings(-0.2, -0.1, -0.2, 0, 0.7, 0.2, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "Bisqwit 2x") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::BisqwitNtscQuarterRes);
				_console->GetSettings()->SetNtscFilterSettings(0, 0, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "Bisqwit 4x") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::BisqwitNtscHalfRes);
				_console->GetSettings()->SetNtscFilterSettings(0, 0, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			} else if(value == "Bisqwit 8x") {
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::BisqwitNtsc);
				_console->GetSettings()->SetNtscFilterSettings(0, 0, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, true, true, true);
			}
		}

		if(readVariable(MesenPalette, var)) {
			string value = string(var.value);
			if(value == "Default NTSC") {
				_console->GetSettings()->SetUserRgbPalette(defaultPalette);
			} else if(value == "Digital Prime (by FirebrandX)") {
				_console->GetSettings()->SetUserRgbPalette(compositeDirectPalette);
			} else if(value == "Wavebeam (by nakedarthur)") {
				_console->GetSettings()->SetUserRgbPalette(nesClassicPalette);
			} else if(value == "Kizul's Definitive NTSC-U NES") {
				_console->GetSettings()->SetUserRgbPalette(nestopiaRgbPalette);
			} else if(value == "NES Classic") {
				_console->GetSettings()->SetUserRgbPalette(originalHardwarePalette);
			} else if(value == "Nestopia YUV") {
				_console->GetSettings()->SetUserRgbPalette(pvmStylePalette);
			} else if(value == "Nintendulator NTSC") {
				_console->GetSettings()->SetUserRgbPalette(sonyCxa2025AsPalette);
			} else if(value == "Sony CXA2025AS") {
				_console->GetSettings()->SetUserRgbPalette(unsaturatedPalette);
			} else if(value == "Zero Degree YUV") {
				_console->GetSettings()->SetUserRgbPalette(yuvPalette);
			} else if(value == "Palightful (by Yave Yu)") {
				_console->GetSettings()->SetUserRgbPalette(wavebeamPalette);
			} else if(value == "Custom") {
				load_custom_palette();
			} else if(value == "Raw") {
				//Using the raw palette replaces the NTSC filters, if one is selected
				_console->GetSettings()->SetVideoFilterType(VideoFilterType::Raw);
			}
		}

		bool beforeNmi = true;
		if(readVariable(MesenOverclockType, var)) {
			string value = string(var.value);
			if(value == "After NMI") {
				beforeNmi = false;
			}
		}

		if(readVariable(MesenOverclock, var)) {
			string value = string(var.value);
			int lineCount = 0;
			if(value == "None") {
				lineCount = 0;
			} else if(value == "Low") {
				lineCount = 100;
			} else if(value == "Medium") {
				lineCount = 250;
			} else if(value == "High") {
				lineCount = 500;
			} else if(value == "Very High") {
				lineCount = 1000;
			}

			if(beforeNmi) {
				_console->GetSettings()->SetPpuNmiConfig(lineCount, 0);
			} else {
				_console->GetSettings()->SetPpuNmiConfig(0, lineCount);
			}
		}

		_console->GetSettings()->SetOverscanDimensions(
			readOverscanValue(MesenOverscanLeft),
			readOverscanValue(MesenOverscanRight),
			readOverscanValue(MesenOverscanTop),
			readOverscanValue(MesenOverscanBottom)
		);

		if(readVariable(MesenAspectRatio, var)) {
			string value = string(var.value);
			if(value == "Auto") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::Auto, 1.0);
			} else if(value == "No Stretching") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::NoStretching, 1.0);
			} else if(value == "NTSC") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::NTSC, 1.0);
			} else if(value == "PAL") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::PAL, 1.0);
			} else if(value == "4:3") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::Standard, 1.0);
			} else if(value == "16:9") {
				_console->GetSettings()->SetVideoAspectRatio(VideoAspectRatio::Widescreen, 1.0);
			}
		}

		if(readVariable(MesenRegion, var)) {
			string value = string(var.value);
			if(value == "Auto") {
				_console->GetSettings()->SetNesModel(NesModel::Auto);
			} else if(value == "NTSC") {
				_console->GetSettings()->SetNesModel(NesModel::NTSC);
			} else if(value == "PAL") {
				_console->GetSettings()->SetNesModel(NesModel::PAL);
			} else if(value == "Dendy") {
				_console->GetSettings()->SetNesModel(NesModel::Dendy);
			}
		}
		
		if(readVariable(MesenRamState, var)) {
			string value = string(var.value);
			if(value == "All 0s (Default)") {
				_console->GetSettings()->SetRamPowerOnState(RamPowerOnState::AllZeros);
			} else if(value == "All 1s") {
				_console->GetSettings()->SetRamPowerOnState(RamPowerOnState::AllOnes);
			} else if(value == "Random Values") {
				_console->GetSettings()->SetRamPowerOnState(RamPowerOnState::Random);
			}
		}

		if(readVariable(MesenScreenRotation, var)) {
			string value = string(var.value);
			if(value == "None") {
				_console->GetSettings()->SetScreenRotation(0);
			} else if(value == u8"90 degrees") {
				_console->GetSettings()->SetScreenRotation(90);
			} else if(value == u8"180 degrees") {
				_console->GetSettings()->SetScreenRotation(180);
			} else if(value == u8"270 degrees") {
				_console->GetSettings()->SetScreenRotation(270);
			}
		}

		int turboSpeed = 0;
		bool turboEnabled = true;
		if(readVariable(MesenControllerTurboSpeed, var)) {
			string value = string(var.value);
			if(value == "Slow") {
				turboSpeed = 0;
			} else if(value == "Normal") {
				turboSpeed = 1;
			} else if(value == "Fast") {
				turboSpeed = 2;
			} else if(value == "Very Fast") {
				turboSpeed = 3;
			} else if(value == "Disabled") {
				turboEnabled = false;
			}
		}

		_shiftButtonsClockwise = false;
		if(readVariable(MesenShiftButtonsClockwise, var)) {
			string value = string(var.value);
			if(value == "enabled") {
				_shiftButtonsClockwise = true;
 			}
		}

		if(readVariable(MesenAudioSampleRate, var)) {
			int old_value = _audioSampleRate;

			_audioSampleRate = atoi(var.value);

			if(old_value != _audioSampleRate) {
				_console->GetSettings()->SetSampleRate(_audioSampleRate);

				// switch when core actively running
				if(_saveStateSize != -1) {
					struct retro_system_av_info system_av_info;
					retro_get_system_av_info(&system_av_info);
					retroEnv(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &system_av_info);
				}
			}
		}

		auto getKeyCode = [=](int port, int retroKey) {
			return (port << 8) | (retroKey + 1);
		};

		auto getKeyBindings = [=](int port) {
			KeyMappingSet keyMappings;
			keyMappings.TurboSpeed = turboSpeed;
			if(_console->GetSettings()->GetControllerType(port) == ControllerType::SnesController) {
				keyMappings.Mapping1.LButton = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_L);
				keyMappings.Mapping1.RButton = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_R);
				keyMappings.Mapping1.A = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_A);
				keyMappings.Mapping1.B = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_B);
				keyMappings.Mapping1.TurboA = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_X);
				keyMappings.Mapping1.TurboB = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_Y);
			} else {
				keyMappings.Mapping1.A = getKeyCode(port, _shiftButtonsClockwise ? RETRO_DEVICE_ID_JOYPAD_B : RETRO_DEVICE_ID_JOYPAD_A);
				keyMappings.Mapping1.B = getKeyCode(port, _shiftButtonsClockwise ? RETRO_DEVICE_ID_JOYPAD_Y : RETRO_DEVICE_ID_JOYPAD_B);
				if(turboEnabled) {
					keyMappings.Mapping1.TurboA = getKeyCode(port, _shiftButtonsClockwise ? RETRO_DEVICE_ID_JOYPAD_A : RETRO_DEVICE_ID_JOYPAD_X);
					keyMappings.Mapping1.TurboB = getKeyCode(port, _shiftButtonsClockwise ? RETRO_DEVICE_ID_JOYPAD_X : RETRO_DEVICE_ID_JOYPAD_Y);
				}
			}

			keyMappings.Mapping1.Start = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_START);
			keyMappings.Mapping1.Select = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_SELECT);

			keyMappings.Mapping1.Up = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_UP);
			keyMappings.Mapping1.Down = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_DOWN);
			keyMappings.Mapping1.Left = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_LEFT);
			keyMappings.Mapping1.Right = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_RIGHT);

			if(port == 0) {
				keyMappings.Mapping1.PartyTapButtons[0] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_B);
				keyMappings.Mapping1.PartyTapButtons[1] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_A);
				keyMappings.Mapping1.PartyTapButtons[2] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_Y);
				keyMappings.Mapping1.PartyTapButtons[3] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_X);
				keyMappings.Mapping1.PartyTapButtons[4] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_L);
				keyMappings.Mapping1.PartyTapButtons[5] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_R);

				unsigned powerPadPort = 0;
				if(_console->GetSettings()->GetExpansionDevice() == ExpansionPortDevice::FamilyTrainerMat) {
					powerPadPort = 4;
				}

				keyMappings.Mapping1.PowerPadButtons[0] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_B);
				keyMappings.Mapping1.PowerPadButtons[1] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_A);
				keyMappings.Mapping1.PowerPadButtons[2] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_Y);
				keyMappings.Mapping1.PowerPadButtons[3] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_X);
				keyMappings.Mapping1.PowerPadButtons[4] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_L);
				keyMappings.Mapping1.PowerPadButtons[5] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_R);
				keyMappings.Mapping1.PowerPadButtons[6] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_LEFT);
				keyMappings.Mapping1.PowerPadButtons[7] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_RIGHT);
				keyMappings.Mapping1.PowerPadButtons[8] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_UP);
				keyMappings.Mapping1.PowerPadButtons[9] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_DOWN);
				keyMappings.Mapping1.PowerPadButtons[10] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_SELECT);
				keyMappings.Mapping1.PowerPadButtons[11] = getKeyCode(powerPadPort, RETRO_DEVICE_ID_JOYPAD_START);

				keyMappings.Mapping1.PachinkoButtons[0] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_R);
				keyMappings.Mapping1.PachinkoButtons[1] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_L);

				keyMappings.Mapping1.ExcitingBoxingButtons[0] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_B); //left hook
				keyMappings.Mapping1.ExcitingBoxingButtons[1] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_RIGHT); //move right
				keyMappings.Mapping1.ExcitingBoxingButtons[2] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_LEFT); //move left
				keyMappings.Mapping1.ExcitingBoxingButtons[3] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_A); //right hook
				keyMappings.Mapping1.ExcitingBoxingButtons[4] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_Y); //left jab
				keyMappings.Mapping1.ExcitingBoxingButtons[5] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_L); //body
				keyMappings.Mapping1.ExcitingBoxingButtons[6] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_X); //right jab
				keyMappings.Mapping1.ExcitingBoxingButtons[7] = getKeyCode(4, RETRO_DEVICE_ID_JOYPAD_R); //straight
			} else if(port == 1) {
				keyMappings.Mapping1.Microphone = getKeyCode(0, RETRO_DEVICE_ID_JOYPAD_L3);
				keyMappings.Mapping1.PowerPadButtons[0] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_B);
				keyMappings.Mapping1.PowerPadButtons[1] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_A);
				keyMappings.Mapping1.PowerPadButtons[2] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_Y);
				keyMappings.Mapping1.PowerPadButtons[3] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_X);
				keyMappings.Mapping1.PowerPadButtons[4] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_L);
				keyMappings.Mapping1.PowerPadButtons[5] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_R);
				keyMappings.Mapping1.PowerPadButtons[6] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_LEFT);
				keyMappings.Mapping1.PowerPadButtons[7] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_RIGHT);
				keyMappings.Mapping1.PowerPadButtons[8] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_UP);
				keyMappings.Mapping1.PowerPadButtons[9] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_DOWN);
				keyMappings.Mapping1.PowerPadButtons[10] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_SELECT);
				keyMappings.Mapping1.PowerPadButtons[11] = getKeyCode(1, RETRO_DEVICE_ID_JOYPAD_START);
			}
			return keyMappings;
		};

		_console->GetSettings()->SetControllerKeys(0, getKeyBindings(0));
		_console->GetSettings()->SetControllerKeys(1, getKeyBindings(1));
		_console->GetSettings()->SetControllerKeys(2, getKeyBindings(2));
		_console->GetSettings()->SetControllerKeys(3, getKeyBindings(3));

		retro_system_av_info avInfo = {};
		_renderer->GetSystemAudioVideoInfo(avInfo);
		retroEnv(RETRO_ENVIRONMENT_SET_GEOMETRY, &avInfo);
	}

	RETRO_API void retro_run()
	{
		if(_console->GetSettings()->CheckFlag(EmulationFlags::ForceMaxSpeed)) {
			//Skip frames to speed up emulation while still outputting at 50/60 fps (needed for FDS fast forward while loading)
			_renderer->SetSkipMode(true);
			_soundManager->SetSkipMode(true);
			for(int i = 0; i < 9; i++) {
				//Attempt to speed up to 1000% speed
				_console->RunSingleFrame();
			}
			_renderer->SetSkipMode(false);
			_soundManager->SetSkipMode(false);
		}

		bool updated = false;
		if(retroEnv(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
			update_settings();

			bool hdPacksEnabled = _console->GetSettings()->CheckFlag(EmulationFlags::UseHdPacks);
			if(hdPacksEnabled != _hdPacksEnabled) {
				//Try to load/unload HD pack when the flag is toggled
				_console->UpdateHdPackMode();
				_hdPacksEnabled = hdPacksEnabled;
			}
		}

		_console->RunSingleFrame();

		if(updated) {
			//Update geometry after running the frame, in case the console's region changed (affects "auto" aspect ratio)
			retro_system_av_info avInfo = {};
			_renderer->GetSystemAudioVideoInfo(avInfo);
			retroEnv(RETRO_ENVIRONMENT_SET_GEOMETRY, &avInfo);
		}
	}

	RETRO_API size_t retro_serialize_size()
	{
		return _saveStateSize;
	}

	RETRO_API bool retro_serialize(void *data, size_t size)
	{
		std::stringstream ss;
		_console->GetSaveStateManager()->SaveState(ss);
		
		string saveStateData = ss.str();
		memset(data, 0, size);
		memcpy(data, saveStateData.c_str(), std::min(size, saveStateData.size()));

		return true;
	}

	RETRO_API bool retro_unserialize(const void *data, size_t size)
	{
		std::stringstream ss;
		ss.write((char*)data, size);

		bool result = _console->GetSaveStateManager()->LoadState(ss, false);
		if(result)
			_console->GetSettings()->SetSampleRate(_audioSampleRate);
		return result;
	}

	RETRO_API void retro_cheat_reset()
	{
		_console->GetCheatManager()->ClearCodes();
	}

	RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *codeStr)
	{
		static const string validGgLetters = "APZLGITYEOXUKSVN";
		static const string validParLetters = "0123456789ABCDEF";
		int chl = 0;

		string code = codeStr;
		std::transform(code.begin(), code.end(), code.begin(), ::toupper);

		if(code[4] == ':') {
			for(;;) {
				string address = code.substr((0 + chl), 4);
				string value = code.substr((5 + chl), 2);
				_console->GetCheatManager()->AddCustomCode(HexUtilities::FromHex(address), HexUtilities::FromHex(value));
				if(code[(7 + chl)] != '+') {
					return;
				}
				chl = (chl + 8);
			}
		}

		else if(code[4] == '?' && code[7] == ':') {
			for(;;) {
				string address = code.substr((0 + chl), 4);
				string comparison = code.substr((5 + chl), 2);
				string value = code.substr((8 + chl), 2);
				_console->GetCheatManager()->AddCustomCode(HexUtilities::FromHex(address), HexUtilities::FromHex(value), HexUtilities::FromHex(comparison));
				if(code[(10 + chl)] != '+') {
					return;
				}
				chl = (chl + 11);
			}
		}

		else {
			//This is either a GG or PAR code
			bool isValidGgCode = true;
			bool isValidParCode = true;

			for(size_t i = 0; i < 6; i++) {
				if(validGgLetters.find(code[i]) == string::npos) {
					isValidGgCode = false;
				}
			}
			for(size_t i = 0; i < 8; i++) {
				if(validParLetters.find(code[i]) == string::npos) {
					isValidParCode = false;
				}
			}

			if(isValidGgCode && code[6] == '+') {
				for(;;) {
					string code1 = code.substr((0 + chl), 6);
					_console->GetCheatManager()->AddGameGenieCode(code1);;
					if(code[(6 + chl)] != '+') {
						return;
					}
					chl = (chl + 7);
				}
			}
			else if(isValidGgCode && code[8] == '+') {
				for(;;) {
					string code1 = code.substr((0 + chl), 8);
					_console->GetCheatManager()->AddGameGenieCode(code1);;
					if(code[(8 + chl)] != '+') {
						return;
					}
					chl = (chl + 9);
				}
			}
			else if(isValidGgCode) {
				_console->GetCheatManager()->AddGameGenieCode(code);;
			}

			else if(isValidParCode && code[8] == '+') {
				for(;;) {
					string code1 = code.substr((0 + chl), 8);
					_console->GetCheatManager()->AddProActionRockyCode(HexUtilities::FromHex(code1));
					if(code[(8 + chl)] != '+') {
						return;
					}
					chl = (chl + 9);
				}
			}
			else if(isValidParCode) {
				_console->GetCheatManager()->AddProActionRockyCode(HexUtilities::FromHex(code));
			}

		}

	}

	void update_input_descriptors()
	{
		vector<retro_input_descriptor> desc;

		auto addDesc = [&desc](unsigned port, unsigned button, const char* name) {
			retro_input_descriptor d = { port, RETRO_DEVICE_JOYPAD, 0, button, name };
			desc.push_back(d);
		};

		auto setupPlayerButtons = [addDesc](int port) {
			unsigned device = _inputDevices[port];
			if(device == DEVICE_AUTO) {
				if(port <= 3) {
					switch(_console->GetSettings()->GetControllerType(port)) {
						case ControllerType::StandardController: device = DEVICE_GAMEPAD; break;
						case ControllerType::PowerPad: device = DEVICE_POWERPAD; break;
						case ControllerType::SnesController: device = DEVICE_SNESGAMEPAD; break;
						case ControllerType::SnesMouse: device = DEVICE_SNESMOUSE; break;
						case ControllerType::Zapper: device = DEVICE_ZAPPER; break;
						case ControllerType::ArkanoidController: device = DEVICE_ARKANOID; break;
						case ControllerType::VbController: device = DEVICE_VBGAMEPAD; break;
						default: return;
					}
				} else if(port == 4) {
					switch(_console->GetSettings()->GetExpansionDevice()) {
						case ExpansionPortDevice::ArkanoidController: device = DEVICE_ARKANOID; break;
						case ExpansionPortDevice::BandaiHyperShot: device = DEVICE_BANDAIHYPERSHOT; break;
						case ExpansionPortDevice::ExcitingBoxing: device = DEVICE_EXCITINGBOXING; break;
						case ExpansionPortDevice::FamilyTrainerMat: device = DEVICE_FAMILYTRAINER; break;
						case ExpansionPortDevice::HoriTrack: device = DEVICE_HORITRACK; break;
						case ExpansionPortDevice::KonamiHyperShot: device = DEVICE_KONAMIHYPERSHOT; break;
						case ExpansionPortDevice::OekaKidsTablet: device = DEVICE_OEKAKIDS; break;
						case ExpansionPortDevice::Pachinko: device = DEVICE_PACHINKO; break;
						case ExpansionPortDevice::PartyTap: device = DEVICE_PARTYTAP; break;
						case ExpansionPortDevice::Zapper: device = DEVICE_ZAPPER; break;
						case ExpansionPortDevice::BattleBox: device = DEVICE_BATTLEBOX; break;
						case ExpansionPortDevice::AsciiTurboFile: device = DEVICE_ASCIITURBOFILE; break;
						case ExpansionPortDevice::FourPlayerAdapter: device = DEVICE_FOURPLAYERADAPTER; break;
						default: return;
					}
				}
			}

			if(device == DEVICE_GAMEPAD || device == DEVICE_SNESGAMEPAD) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right");
				if(device == DEVICE_SNESGAMEPAD) {
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "A");
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "B");
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "X");
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Y");
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "L");
					addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "R");
				} else {
					if(_shiftButtonsClockwise) {
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "A");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "B");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "Turbo A");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Turbo B");
					} else {
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "A");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "B");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Turbo A");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Turbo B");
					}

					if(port == 0) {
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "(FDS) Insert Next Disk");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "(FDS) Switch Disk Side");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_L2, "(VS) Insert Coin 1");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_R2, "(VS) Insert Coin 2");
						addDesc(port, RETRO_DEVICE_ID_JOYPAD_L3, "(Famicom) Microphone (P2)");
					}
				}
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_START, "Start");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select");
			} else if(device == DEVICE_EXCITINGBOXING) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "Left Hook");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "Right Hook");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Left Jab");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Right Jab");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "Body");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "Straight");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_LEFT, "Move Left");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Move Right");
			} else if(device == DEVICE_PARTYTAP) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "Partytap P1");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "Partytap P2");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Partytap P3");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Partytap P4");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "Partytap P5");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "Partytap P6");
			} else if(device == DEVICE_FAMILYTRAINER || device == DEVICE_POWERPAD) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "Powerpad B1");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "Powerpad B2");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Powerpad B3");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Powerpad B4");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "Powerpad B5");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "Powerpad B6");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_LEFT, "Powerpad B7");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Powerpad B8");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_UP, "Powerpad B9");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_DOWN, "Powerpad B10");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_SELECT, "Powerpad B11");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_START, "Powerpad B12");
			} else if(device == DEVICE_PACHINKO) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "Release Trigger");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "Press Trigger");
			} else if(device == DEVICE_VBGAMEPAD) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "Virtual Boy D-Pad 2 Down");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Virtual Boy D-Pad 2 Left");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_SELECT, "Virtual Boy Select");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_START, "Virtual Boy Start");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_UP, "Virtual Boy D-Pad 1 Up");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_DOWN, "Virtual Boy D-Pad 1 Down");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_LEFT, "Virtual Boy D-Pad 1 Left");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Virtual Boy D-Pad 1 Right");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "Virtual Boy D-Pad 2 Right");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "Virtual Boy D-Pad 2 Up");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "Virtual Boy L");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "Virtual Boy R");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L2, "Virtual Boy B");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R2, "Virtual Boy A");
			}
		};

		setupPlayerButtons(0);
		setupPlayerButtons(1);
		setupPlayerButtons(2);
		setupPlayerButtons(3);
		setupPlayerButtons(4);

		retro_input_descriptor end = { 0 };
		desc.push_back(end);

		retroEnv(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc.data());
	}

	void update_core_controllers()
	{
		//Setup all "auto" ports
		RomInfo romInfo = _console->GetRomInfo();
		if(romInfo.IsInDatabase || romInfo.IsNes20Header) {
			_console->GetSettings()->InitializeInputDevices(romInfo.InputType, romInfo.System, true);
		} else {
			_console->GetSettings()->InitializeInputDevices(GameInputType::StandardControllers, GameSystem::NesNtsc, true);
		}

		for(int port = 0; port < 5; port++) {
			if(_inputDevices[port] != DEVICE_AUTO) {
				if(port <= 3) {
					ControllerType type = ControllerType::StandardController;
					switch(_inputDevices[port]) {
						case RETRO_DEVICE_NONE: type = ControllerType::None; break;
						case DEVICE_GAMEPAD: type = ControllerType::StandardController; break;
						case DEVICE_ZAPPER: type = ControllerType::Zapper; break;
						case DEVICE_POWERPAD: type = ControllerType::PowerPad; break;
						case DEVICE_ARKANOID: type = ControllerType::ArkanoidController; break;
						case DEVICE_SNESGAMEPAD: type = ControllerType::SnesController; break;
						case DEVICE_SNESMOUSE: type = ControllerType::SnesMouse; break;
						case DEVICE_VBGAMEPAD: type = ControllerType::VbController; break;
					}
					_console->GetSettings()->SetControllerType(port, type);
				} else {
					ExpansionPortDevice type = ExpansionPortDevice::None;
					switch(_inputDevices[port]) {
						case RETRO_DEVICE_NONE: type = ExpansionPortDevice::None; break;
						case DEVICE_FAMILYTRAINER: type = ExpansionPortDevice::FamilyTrainerMat; break;
						case DEVICE_PARTYTAP: type = ExpansionPortDevice::PartyTap; break;
						case DEVICE_PACHINKO: type = ExpansionPortDevice::Pachinko; break;
						case DEVICE_EXCITINGBOXING: type = ExpansionPortDevice::ExcitingBoxing; break;
						case DEVICE_KONAMIHYPERSHOT: type = ExpansionPortDevice::KonamiHyperShot; break;
						case DEVICE_OEKAKIDS: type = ExpansionPortDevice::OekaKidsTablet; break;
						case DEVICE_BANDAIHYPERSHOT: type = ExpansionPortDevice::BandaiHyperShot; break;
						case DEVICE_ARKANOID: type = ExpansionPortDevice::ArkanoidController; break;
						case DEVICE_HORITRACK: type = ExpansionPortDevice::HoriTrack; break;
						case DEVICE_ASCIITURBOFILE: type = ExpansionPortDevice::AsciiTurboFile; break;
						case DEVICE_BATTLEBOX: type = ExpansionPortDevice::BattleBox; break;
						case DEVICE_FOURPLAYERADAPTER: type = ExpansionPortDevice::FourPlayerAdapter; break;
					}
					_console->GetSettings()->SetExpansionDevice(type);
				}
			}
		}

		bool hasFourScore = false;
		bool isFamicom = (_console->GetSettings()->GetExpansionDevice() != ExpansionPortDevice::None || romInfo.System == GameSystem::Famicom || romInfo.System == GameSystem::FDS || romInfo.System == GameSystem::Dendy);
		if(isFamicom) {
			_console->GetSettings()->SetConsoleType(ConsoleType::Famicom);
			if(_console->GetSettings()->GetExpansionDevice() == ExpansionPortDevice::FourPlayerAdapter) {
				hasFourScore = true;
			}
		} else {
			_console->GetSettings()->SetConsoleType(ConsoleType::Nes);
			if(_console->GetSettings()->GetControllerType(2) != ControllerType::None || _console->GetSettings()->GetControllerType(3) != ControllerType::None) {
				hasFourScore = true;
			}
		}

		_console->GetSettings()->SetFlagState(EmulationFlags::HasFourScore, hasFourScore);
	}
	
	void retro_set_memory_maps()
	{
		//Expose internal RAM and work/save RAM for retroachievements
		retro_memory_descriptor descriptors[256] = {};
		retro_memory_map memoryMap = {};

		int count = 0;
		for(int i = 0; i <= 0xFFFF; i += 0x100) {
			uint8_t* ram = _console->GetRamBuffer(i);
			if(ram) {
				descriptors[count].ptr = ram;
				descriptors[count].start = i;
				descriptors[count].len = 0x100;
				count++;
			}
		}

		memoryMap.descriptors = descriptors;
		memoryMap.num_descriptors = count;

		retroEnv(RETRO_ENVIRONMENT_SET_MEMORY_MAPS, &memoryMap);
	}

	RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
	{
		if(port < 5 && _inputDevices[port] != device) {
			_inputDevices[port] = device;
			update_core_controllers();
			update_input_descriptors();
		}
	}

	RETRO_API bool retro_load_game(const struct retro_game_info *game)
	{
		char *systemFolder;
		if(!retroEnv(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &systemFolder) || !systemFolder) {
			return false;
		}

		char *saveFolder;
		if(!retroEnv(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &saveFolder)) {
			logMessage(RETRO_LOG_ERROR, "Could not find save directory.\n");
		}

		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		if(!retroEnv(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
			logMessage(RETRO_LOG_ERROR, "XRGB8888 is not supported.\n");
			return false;
		}

		//Expect the following structure:
		// /system/disksys.rom
		// /system/HdPacks/*
		// /saves/*.sav
		FolderUtilities::SetHomeFolder(systemFolder);
		FolderUtilities::SetFolderOverrides(saveFolder, "", "");

		update_settings();

		//Plug in 2 standard controllers by default, game database will switch the controller types for recognized games
		_console->GetSettings()->SetMasterVolume(10.0);
		_console->GetSettings()->SetControllerType(0, ControllerType::StandardController);
		_console->GetSettings()->SetControllerType(1, ControllerType::StandardController);
		_console->GetSettings()->SetControllerType(2, ControllerType::None);
		_console->GetSettings()->SetControllerType(3, ControllerType::None);

		// Attempt to fetch extended game info
		const struct retro_game_info_ext *gameExt = NULL;
		const void *gameData = NULL;
		size_t gameSize = 0;
		string gamePath("");
		if (retroEnv(RETRO_ENVIRONMENT_GET_GAME_INFO_EXT, &gameExt)) {
			gameData = gameExt->data;
			gameSize = gameExt->size;
			if (gameExt->file_in_archive) {
				// We don't have a 'physical' file in this
				// case, but the core still needs a filename
				// in order to detect associated content
				// (i.e. HdPacks). We therefore fake it, using
				// the content directory, canonical content
				// name, and content file extension
#if defined(_WIN32)
				char slash = '\\';
#else
				char slash = '/';
#endif
				gamePath = string(gameExt->dir) +
							  string(1, slash) +
							  string(gameExt->name) +
							  "." +
							  string(gameExt->ext);
			} else {
				gamePath = gameExt->full_path;
			}
		} else {
			// No extended game info; all we have is the
			// content fullpath from the retro_game_info
			// struct
			gamePath = game->path;
		}

		// Load content
		VirtualFile romData(gameData, gameSize, gamePath);
		bool result = _console->Initialize(romData);

		if(result) {
			//Set default dipswitches for some VS System games
			switch(_console->GetRomInfo().Hash.PrgCrc32) {
				case 0x8850924B: _console->GetSettings()->SetDipSwitches(32); break; //VS Tetris
				case 0xE1AA8214: _console->GetSettings()->SetDipSwitches(32); break; //StarLuster
				default: _console->GetSettings()->SetDipSwitches(0); break;
			}

			update_core_controllers();
			update_input_descriptors();

			//Savestates in Mesen may change size over time
			//Retroarch doesn't like this for netplay or rewinding - it requires the states to always be the exact same size
			//So we need to send a large enough size to Retroarch to ensure Mesen's state will always fit within that buffer.
			std::stringstream ss;
			_console->GetSaveStateManager()->SaveState(ss);

			//Round up to the next 1kb multiple
			_saveStateSize = ((ss.str().size() * 2) + 0x400) & ~0x3FF;
			retro_set_memory_maps();
		}

		return result;
	}

	RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
	{
		return false;
	}

	RETRO_API void retro_unload_game()
	{
		_console->Stop();
	}

	RETRO_API unsigned retro_get_region()
	{
		NesModel model = _console->GetModel();
		return model == NesModel::NTSC ? RETRO_REGION_NTSC : RETRO_REGION_PAL;
	}

	RETRO_API void retro_get_system_info(struct retro_system_info *info)
	{
		_mesenVersion = EmulationSettings::GetMesenVersionString();

		info->library_name = "Mesen";
		info->library_version = _mesenVersion.c_str();
		// need_fullpath is required since HdPacks are
		// identified via the rom file name
		info->need_fullpath = true;
		info->valid_extensions = "nes|fds|unf|unif";
		info->block_extract = false;
	}

	RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
	{
		uint32_t hscale = 1;
		uint32_t vscale = 1;
		switch(_console->GetSettings()->GetVideoFilterType()) {
			case VideoFilterType::NTSC: hscale = 2; break;
			case VideoFilterType::BisqwitNtscQuarterRes: hscale = 2; break;
			case VideoFilterType::BisqwitNtscHalfRes: hscale = 4; break;
			case VideoFilterType::BisqwitNtsc: hscale = 8; break;
			default: hscale = 1; break;
		}
		
		shared_ptr<HdPackData> hdData = _console->GetHdData();
		if(hdData) {
			hscale = hdData->Scale;
			vscale = hdData->Scale;
		}

		if(hscale <= 2) {
			_renderer->GetSystemAudioVideoInfo(*info, NES_NTSC_OUT_WIDTH(256), 240 * vscale);
		} else {
			_renderer->GetSystemAudioVideoInfo(*info, 256 * hscale, 240 * vscale);
		}
	}

	RETRO_API void *retro_get_memory_data(unsigned id)
	{
		BaseMapper* mapper = _console->GetMapper();
		switch(id) {
			case RETRO_MEMORY_SAVE_RAM: return mapper->GetSaveRam();
			case RETRO_MEMORY_SYSTEM_RAM: return _console->GetMemoryManager()->GetInternalRAM();
		}
		return nullptr;
	}

	RETRO_API size_t retro_get_memory_size(unsigned id)
	{
		BaseMapper* mapper = _console->GetMapper();
		switch(id) {
			case RETRO_MEMORY_SAVE_RAM: return mapper->GetMemorySize(DebugMemoryType::SaveRam);
			case RETRO_MEMORY_SYSTEM_RAM: return MemoryManager::InternalRAMSize;
		}
		return 0;
	}
}
