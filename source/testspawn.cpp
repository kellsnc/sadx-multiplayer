#include "pch.h"
#include <cstdlib>
#include <cwchar>
#include <ShellAPI.h>
#include "multiplayer.h"
#include "players.h"
#include "netplay.h"

static const std::unordered_map<std::wstring, multiplayer::mode> multi_mode_map = {
	{ L"coop", multiplayer::mode::coop },
	{ L"battle", multiplayer::mode::battle }
};

static multiplayer::mode parse_multi_mode(const std::wstring& str)
{
	std::wstring lowercase = str;
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = multi_mode_map.find(lowercase);

	if (it != multi_mode_map.end())
		return it->second;

	return multiplayer::mode::battle;
}

static const std::unordered_map<std::wstring, Characters> character_name_ids_map = {
	{ L"sonic",      Characters_Sonic },
	{ L"eggman",     Characters_Eggman },
	{ L"tails",      Characters_Tails },
	{ L"knuckles",   Characters_Knuckles },
	{ L"tikal",      Characters_Tikal },
	{ L"amy",        Characters_Amy },
	{ L"gamma",      Characters_Gamma },
	{ L"big",        Characters_Big },
	{ L"metalsonic", Characters_MetalSonic }
};

static Characters parse_character_id(const std::wstring& str)
{
	std::wstring lowercase = str;
	transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = character_name_ids_map.find(lowercase);

	if (it != character_name_ids_map.end())
		return it->second;

	return static_cast<Characters>(std::stol(lowercase));
}

void TestSpawn()
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; i++)
	{
		if (!wcscmp(argv[i], L"--multi"))
		{
			multiplayer::Enable(_wtoi(argv[i + 1]), parse_multi_mode(argv[i + 2]));
			i += 2;
		}

		if (!wcscmp(argv[i], L"-p2"))
		{
			SetCurrentCharacter(1, parse_character_id(argv[++i]));
		}

		if (!wcscmp(argv[i], L"-p3"))
		{
			SetCurrentCharacter(2, parse_character_id(argv[++i]));
		}

		if (!wcscmp(argv[i], L"-p4"))
		{
			SetCurrentCharacter(3, parse_character_id(argv[++i]));
		}

		if (!wcscmp(argv[i], L"--net"))
		{
			char ip[MAX_PATH];

			WriteData<2>(reinterpret_cast<void*>(0x00789D18), static_cast<uint8_t>(0x90));; // remove mutex

			if (!wcstombs_s(NULL, ip, MAX_PATH, argv[i + 2], wcslen(argv[i + 2])))
			{
				netplay.Create(!wcscmp(argv[i + 1], L"host") ? Netplay::Type::Server : Netplay::Type::Client, ip, _wtoi(argv[i + 3]));
			}

			i += 3;
		}
	}

	LocalFree(argv);
}