#include "pch.h"
#include "multi.h"

extern bool MultiMenuEnabled;

uint8_t cursor[PLAYER_MAX];

NJS_TEXNAME multicharTex[10];
NJS_TEXLIST multichar_Texlist = { arrayptrandlength(multicharTex) };

int playerReady[PLAYER_MAX];

enum playReadyState {
	disconnected,
	pressedStart,
	ready
};

enum CharacterMenu {
	sonicIcon,
	eggIcon,
	tailsIcon,
	knuxIcon,
	tikalIcon,
	amyIcon,
	e102Icon,
	bigIcon,
	msIcon,
	cursorIcon
};

NJS_VECTOR CursorColor[8] = {
	{ 1, 1, 1},
	{ 0.145, 0.501, 0.894 }, //light blue
	{0.423, 0.894, 0.047}, //light green
	{ 0.894, 0.701, 0.047}, //yellow
	{ 0.976, 0.525, 0.862}, //pink 
	{ 0.584, 0.074, 0.560}, //purple
	{ 0.070, 0.047, 0.894}, //dark blue
	{ 0.521, 0, 0}, //dark red
};

NJS_TEXANIM MultiTexAnim[]{
	{ 64, 64, 0, 0, 0, 0, 255, 255, sonicIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, eggIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, tailsIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, knuxIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, tikalIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, amyIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, bigIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, e102Icon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, bigIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, msIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, cursorIcon, 0x20 },
};

NJS_POINT2 cursorPosArray[8]{
	{ 200, 150 },
	{ 260, 150 },
	{ 320, 150 },
	{ 380, 150 },
	{ 200, 220 },
	{ 260, 220 },
	{ 320, 220 },
	{ 380, 220 },

};

NJS_SPRITE MultiTexSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim };

NJS_SPRITE MultiCursorSprite[8] = {
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
	{ {0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim },
};


bool isAlreadySelected(char pnum, char character) {

	for (int i = 0; i < PLAYER_MAX; i++) {

		if (i == pnum)
			continue;

		if ((char)GetCurrentCharacter(i) == character)
			return true;
	}

	return false;
}

bool isEveryoneReady() {

	char countRDY = 0;
	char countNotRDY = 0;

	for (int i = 0; i < PLAYER_MAX; i++) {

		if (playerReady[i] <= disconnected) {
			DisplayDebugStringFormatted(NJM_LOCATION(2, 4 + i), "Player %d Disconnected.", i);
		}

		if (playerReady[i] == ready) {
			countRDY++;

			DisplayDebugStringFormatted(NJM_LOCATION(2, 4 + i), "Player %d READY!", i);
		}

		if (playerReady[i] == pressedStart) {
			countNotRDY++;

			DisplayDebugStringFormatted(NJM_LOCATION(2, 4 + i), "Player %d Select character...", i);
		}
	}

	if (countNotRDY <= 0 && countRDY >= 2)
		return true;

	return false;
}


void StartMulti_Game(TrialActSelWk* wk) {

	if (!isEveryoneReady())
		return;

	LastLevel = CurrentLevel;
	LastAct = CurrentAct;

	CurrentCharacter = cursor[0];

	CurrentLevel = 1;
	CurrentAct = 0;

	SeqTp->awp[1].work.ul[0] = 100;

	AvaStgActT stgact = { CurrentLevel, CurrentAct };
	AvaCmnPrm = stgact;
	AdvertiseWork.Stage = stgact.Stg;
	AdvertiseWork.Act = stgact.Act;
	wk->SelStg = stgact.Stg;

	wk->Stat = ADVA_STAT_FADEOUT;
	wk->T = 0.0f;
}

void DrawCharacterPortrait(char i) {

	SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
	MultiTexSprite.p.x = cursorPosArray[i].x;
	MultiTexSprite.p.y = cursorPosArray[i].y;
	njDrawSprite2D_DrawNow(&MultiTexSprite, i, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	return;
}


void DrawCursor(char pnum) {

	if (playerReady[pnum] < pressedStart || pnum > PLAYER_MAX)
		return;

	SetMaterialAndSpriteColor_Float(1, CursorColor[pnum].x, CursorColor[pnum].y, CursorColor[pnum].z);
	MultiCursorSprite[pnum].p.x = cursorPosArray[cursor[pnum]].x;
	MultiCursorSprite[pnum].p.y = cursorPosArray[cursor[pnum]].y;

	njDrawSprite2D_DrawNow(&MultiCursorSprite[pnum], cursorIcon + 1, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	ClampGlobalColorThing_Thing();
}

void __cdecl MultiMenuExec_Display(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_disp(tp);
		return;
	}

	if (!MissedFrames && TrialActStelTp)
	{
		auto wk = (TrialActSelWk*)tp->awp;

		if (wk->BaseCol != 0)
		{
			gHelperFunctions->PushScaleUI(uiscale::Align_Center, false, 1.0f, 1.0f);
			SetDefaultAlphaBlend();

			for (int i = 0; i < 8; i++) {
				DrawCursor(i);
				DrawCharacterPortrait(i);
			}


			DrawSADXText("MULTIPLAYER - BATTLE", 0, 24, 120, 40);
			ClampGlobalColorThing_Thing();
			gHelperFunctions->PopScaleUI();
		}


		float a1 = wk->BaseZ - 2.0;
		DrawTiledBG_AVA_BACK(a1);
	}
}

bool MultiMenu_CheckMoveInput(int button, char pNum)
{
	//lock control if player has the character selected.

	if ((PressedButtons[pNum] & button) != 0)
	{
		switch (button)
		{
		case Buttons_Up:
			cursor[pNum] -= 4;
			break;
		case Buttons_Down:
			cursor[pNum] += 4;
			break;
		case Buttons_Left:
			cursor[pNum]--;
			break;
		case Buttons_Right:
			cursor[pNum]++;
			break;
		default:
			return false;
		}

		if (cursor[pNum] < sonicIcon)
			cursor[pNum] = bigIcon;

		if (cursor[pNum] > msIcon)
			cursor[pNum] = sonicIcon;

		PlayMenuBipSound();

	}

	return false;
}

void MultiMenu_InputCheck(task* tp, TrialActSelWk* wk)
{

	for (int i = 0; i < PLAYER_MAX; i++) {

		if (MenuSelectButtonsPressed_r(i))
		{
			PlayMenuEnterSound();

			if (playerReady[i] < ready)
				playerReady[i]++;


			if (playerReady[i] >= ready)
			{
				SetCurrentCharacter(i, cursor[i]);
				continue;
			}

		}

		if (playerReady[i] <= disconnected)
			continue;

		if (MenuBackButtonsPressed_r(i))
		{
			PlayMenuBackSound();

			if (i == 0 && playerReady[0] == pressedStart) {
				CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU); // force back to lead to menu
				PlayMenuBackSound();
				wk->Stat = ADVA_STAT_FADEOUT;
				wk->T = 0.0;
				return;
			}

			if (playerReady[i] >= pressedStart) {
				SetCurrentCharacter(i, -1);
				playerReady[i]--;
			}
		}


		if (playerReady[i] >= ready || playerReady[i] <= disconnected)
			continue;


		if (MultiMenu_CheckMoveInput(Buttons_Up, i))
			continue;

		if (MultiMenu_CheckMoveInput(Buttons_Down, i))
			continue;

		if (MultiMenu_CheckMoveInput(Buttons_Left, i))
			continue;

		if (MultiMenu_CheckMoveInput(Buttons_Right, i))
			continue;
	}

	return;
}

void __cdecl MultiMenuExec_Main(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		MusicList[MusicIDs_Trial].Name = "Trial";
		trial_act_sel_exec(tp);
		return;
	}

	auto wk = (TrialActSelWk*)tp->awp;

	if (SeqTp->awp->work.ul[1] == ADVA_MODE_TRIALACT_SEL && wk->Stat == ADVA_STAT_REQWAIT)
	{
		memset(playerReady, 0, sizeof(playerReady));
		ResetCharactersArray();
		player_count = 0;
		MusicList[MusicIDs_Trial].Name = "btl_sel";
		PlayMenuMusicID(4);
		PlayVoice(40);
		LoadPVM("multichar", &multichar_Texlist);
		playerReady[0] = pressedStart;
		SetDebugFontSize(13.0f * (unsigned short)VerticalResolution / 480.0f);
		SetDebugFontColor(0x8e8e8e);

		wk->Stat = ADVA_STAT_FADEIN;
		wk->T = 0.0f;
		wk->SelStg = -1;
	}

	if (SeqTp->awp->work.ul[2] == ADVA_MODE_TITLE_NEW && wk->Stat == ADVA_STAT_KEEP)
	{
		PlayMenuEnterSound();
	}

	switch (wk->Stat)
	{
	case ADVA_STAT_REQWAIT:
		wk->BaseCol = 0;
		wk->SelStg = -1;
		break;
	case ADVA_STAT_FADEIN:
		wk->T += MissedFrames_B * 0.025f;
		wk->BaseCol = GetFadeOutColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->BaseCol = 0xFFFFFFFF;
			wk->Stat = ADVA_STAT_KEEP;
		}

		break;
	case ADVA_STAT_KEEP:
		SeqTp->awp->work.ul[3] = 3;
		MultiMenu_InputCheck(tp, wk);
		StartMulti_Game(wk);
		break;
	case ADVA_STAT_FADEOUT:
		wk->T += MissedFrames_B * 0.1f;
		wk->BaseCol = GetFadeInColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->Stat = ADVA_STAT_REQWAIT;
			SeqTp->awp[1].work.ub[15] = 1;
			njReleaseTexture(&multichar_Texlist);
			MenuLaunchNext();
			MultiMenuEnabled = false;

			if (wk->SelStg != -1)
			{
				SeqTp->awp[1].work.sl[1] = 100;
			}
		}

		break;
	default:
		return;
	}

	tp->disp(tp);
}

void __cdecl LoadMultiMenuExec(ModeSelPrmType* prmp)
{
	auto tp = TrialActStelTp = CreateElementalTask(0, LEV_4, MultiMenuExec_Main);

	auto wk = (TrialActSelWk*)AllocateArray(1, 208);
	wk->NextMode = prmp->NextMode;
	wk->PrevMode = prmp->PrevMode;

	tp->awp = (anywk*)wk;
	tp->disp = MultiMenuExec_Display;

	ChgSubModeToStay_0(prmp, tp);
}

void init_MultiMenu()
{
	WriteJump(LevelSelect_Load, LoadMultiMenuExec);
}