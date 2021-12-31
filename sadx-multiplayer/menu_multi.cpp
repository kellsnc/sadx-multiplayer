#include "pch.h"
#include "menu_multi.h"

enum AVA_MULTI_TEX
{
	AVAMULTITEX_SONIC,
	AVAMULTITEX_EGGMAN,
	AVAMULTITEX_TAILS,
	AVAMULTITEX_KNUCKLES,
	AVAMULTITEX_TIKAL,
	AVAMULTITEX_AMY,
	AVAMULTITEX_E102,
	AVAMULTITEX_BIG,
	AVAMULTITEX_METAL,
	AVAMULTITEX_CURSOR,
	AVAMULTITEX_STG1,
	AVAMULTITEX_STG2,
	AVAMULTITEX_STG3,
	AVAMULTITEX_STG4,
	AVAMULTITEX_STG5,
	AVAMULTITEX_STG6,
	AVAMULTITEX_STG7,
	AVAMULTITEX_STG8,
	AVAMULTITEX_STG9,
	AVAMULTITEX_STG10,
	AVAMULTITEX_BACK,
	AVAMULTITEX_CONFIRM,
	AVAMULTITEX_SELECT
};

enum AVA_MULTI_ANM
{
	AVAMULTIANM_CHARA = 0,
	AVAMULTIANM_CURSORCHAR = 9,
	AVAMULTIANM_CURSORSTG,
	AVAMULTIANM_STG,
	AVAMULTIANM_BACK = 21,
	AVAMULTIANM_CONFIRM,
	AVAMULTIANM_SELECT
};

enum playReadyState {
	disconnected,
	pressedStart,
	ready
};

enum class multiTask {
	init,
	charSelect,
	levelSelect,
	finish
};

extern bool MultiMenuEnabled;
bool isStageSelected = false;
const int levelMax = 10;
int playerReady[PLAYER_MAX];

uint8_t cursorChar[PLAYER_MAX];
uint8_t cursorLvl;

NJS_TEXNAME AVA_MULTI_TEXNAME[10];
NJS_TEXLIST AVA_MULTI_TEXLIST = { arrayptrandlength(AVA_MULTI_TEXNAME) };

NJS_TEXANIM AVA_MULTI_TEXANIM[]{
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_SONIC, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_EGGMAN, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_TAILS, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_KNUCKLES, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_TIKAL, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_AMY, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_BIG, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_E102, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_METAL, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_CURSOR, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG1, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG2, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG3, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG4, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG5, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG6, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG7, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG8, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG9, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_STG10, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, AVAMULTITEX_CURSOR, 0x20 },
	{ 110, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_BACK, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_CONFIRM, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_SELECT, 0x20},
};

NJS_SPRITE MultiLegendSprite = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };
NJS_SPRITE MultiLevel_Sprite = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };
NJS_SPRITE MultiLevelCursor_Sprite = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };
NJS_SPRITE MultiCharTexSprite = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };

NJS_SPRITE MultiChar_CursorSprite[8] = {
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
	{ { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM },
};

void MultiMenu_DrawControls()
{
	gHelperFunctions->PushScaleUI((uiscale::Align)(Align_Bottom | Align_Center_Horizontal), false, 1.0f, 1.0f);

	MultiLegendSprite.p.y = 448.0f;

	MultiLegendSprite.p.x = 165.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, AVAMULTIANM_SELECT, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	MultiLegendSprite.p.x = 330.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, AVAMULTIANM_CONFIRM, -64, NJD_SPRITE_ALPHA);

	MultiLegendSprite.p.x = 495.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, AVAMULTIANM_BACK, -64, NJD_SPRITE_ALPHA);

	gHelperFunctions->PopScaleUI();
}

LevelAndActIDs multiLevelsSonicArray[]
{
	{ LevelAndActIDs_EmeraldCoast1 },
	//{ LevelAndActIDs_EmeraldCoast2 },
	{ LevelAndActIDs_WindyValley1 },
	//{ LevelAndActIDs_WindyValley2 },
	//{ LevelAndActIDs_WindyValley3 },
	{ LevelAndActIDs_Casinopolis2 },
	{ LevelAndActIDs_IceCap1 },
	//{ LevelAndActIDs_IceCap2 },
	//{ LevelAndActIDs_IceCap3 },
	{ LevelAndActIDs_TwinklePark1 },
	//{ LevelAndActIDs_TwinklePark2 },
	{ LevelAndActIDs_SpeedHighway1 },
	//{ LevelAndActIDs_SpeedHighway3 },
	{ LevelAndActIDs_RedMountain1 },
	//{ LevelAndActIDs_RedMountain2 },
	{ LevelAndActIDs_SkyDeck1 },
	//{ LevelAndActIDs_SkyDeck2 },
	//{ LevelAndActIDs_SkyDeck3 },
	{ LevelAndActIDs_LostWorld1 },
	//{ LevelAndActIDs_LostWorld2 },
	{ LevelAndActIDs_FinalEgg1 },
	//{ LevelAndActIDs_FinalEgg2 },
	//{ LevelAndActIDs_FinalEgg3 },

};

NJS_POINT2 cursorLevel_PosArray[]{
	{ 50, 150 },
	{ 180, 150 },
	{ 310, 150 },
	{ 440, 150 },
	{ 50, 220 },
	{ 180, 220 },
	{ 310, 220 },
	{ 440, 220 },
	{ 50, 290 },
	{ 180, 290 },
};

NJS_POINT2 cursorCharPos_Array[8]{
	{ 200.0f, 150.0f },
	{ 260.0f, 150.0f },
	{ 320.0f, 150.0f },
	{ 380.0f, 150.0f },
	{ 200.0f, 220.0f },
	{ 260.0f, 220.0f },
	{ 320.0f, 220.0f },
	{ 380.0f, 220.0f },
};

NJS_VECTOR CursorColor[8] = {
	{ 1.0f, 1.0f, 1.0f },
	{ 0.145f, 0.501f, 0.894f }, //light blue
	{ 0.423f, 0.894f, 0.047f }, //light green
	{ 0.894f, 0.701f, 0.047f }, //yellow
	{ 0.976f, 0.525f, 0.862f }, //pink 
	{ 0.584f, 0.074f, 0.560f }, //purple
	{ 0.070f, 0.047f, 0.894f }, //dark blue
	{ 0.521f, 0.0f, 0.0f}, //dark red
};

void DrawMulti_LevelSelect(int i)
{
	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
	MultiLevel_Sprite.p.x = cursorLevel_PosArray[i].x;
	MultiLevel_Sprite.p.y = cursorLevel_PosArray[i].y;
	njDrawSprite2D_DrawNow(&MultiLevel_Sprite, AVAMULTIANM_STG + i, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
}

void SetPlayerToUnready()
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (playerReady[i] >= ready) 
			playerReady[i] = pressedStart;
	}
}

bool isEveryoneReady()
{
	int countRDY = 0;
	int countNotRDY = 0;

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (playerReady[i] <= disconnected)
		{
			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d Disconnected", i + 1);

			if (i > 0)
				cursorChar[i] = i;
		}

		if (playerReady[i] == ready)
		{
			countRDY++;

			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d READY!", i + 1);
		}

		if (playerReady[i] == pressedStart)
		{
			countNotRDY++;
	
			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d Select character...", i + 1);
		}
	}

	return countNotRDY <= 0 && countRDY >= 2;
}

void StartMulti_Game(task* tp, TrialActSelWk* wk)
{
	if (!tp->ctp)
		return;

	if (tp->ctp->twp->mode < (char)multiTask::levelSelect || !isStageSelected)
		return;

	LastLevel = CurrentLevel;
	LastAct = CurrentAct;

	CurrentCharacter = cursorChar[0];

	CurrentLevel = ConvertLevelActsID_ToLevel(multiLevelsSonicArray[cursorLvl]);
	CurrentAct = ConvertLevelActsID_ToAct(multiLevelsSonicArray[cursorLvl]);

	SeqTp->awp[1].work.ul[0] = 100;

	AvaStgActT stgact = { CurrentLevel, CurrentAct };
	AvaCmnPrm = stgact;
	AdvertiseWork.Stage = stgact.Stg;
	AdvertiseWork.Act = stgact.Act;
	wk->SelStg = stgact.Stg;

	wk->Stat = ADVA_STAT_FADEOUT;
	wk->T = 0.0f;
	njReleaseTexture(&AVA_MULTI_TEXLIST);
}

void DrawCharacterPortrait(int i)
{
	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
	MultiCharTexSprite.p.x = cursorCharPos_Array[i].x;
	MultiCharTexSprite.p.y = cursorCharPos_Array[i].y;
	njDrawSprite2D_DrawNow(&MultiCharTexSprite, AVAMULTIANM_CHARA + i, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
}

void DrawChar_Cursor(int pnum)
{
	if (playerReady[pnum] < pressedStart || pnum > PLAYER_MAX)
		return;

	SetMaterialAndSpriteColor_Float(1, CursorColor[pnum].x, CursorColor[pnum].y, CursorColor[pnum].z);
	MultiChar_CursorSprite[pnum].p.x = cursorCharPos_Array[cursorChar[pnum]].x;
	MultiChar_CursorSprite[pnum].p.y = cursorCharPos_Array[cursorChar[pnum]].y;

	njDrawSprite2D_DrawNow(&MultiChar_CursorSprite[pnum], AVAMULTIANM_CURSORCHAR, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	ClampGlobalColorThing_Thing();
}

void DrawLevel_Cursor()
{
	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
	MultiLevelCursor_Sprite.p.x = cursorLevel_PosArray[cursorLvl].x;
	MultiLevelCursor_Sprite.p.y = cursorLevel_PosArray[cursorLvl].y;

	njDrawSprite2D_DrawNow(&MultiLevelCursor_Sprite, AVAMULTIANM_CURSORSTG, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	ClampGlobalColorThing_Thing();
}

void MultiMenu_InputLevel(EntityData1* data)
{
	if (MenuBackButtonsPressed_r(0))
	{
		PlayMenuBackSound();
		SetPlayerToUnready();
		data->Action--;
		return;
	}

	if (MenuSelectButtonsPressed_r(0))
	{
		isStageSelected = true;
		PlayMenuEnterSound();
		data->Action++;
	}

	if ((PressedButtons[0] & Buttons_Up))
	{
		cursorLvl -= 4;
		PlayMenuBipSound();
	}

	if ((PressedButtons[0] & Buttons_Down))
	{
		cursorLvl += 4;
		PlayMenuBipSound();
	}

	if ((PressedButtons[0] & Buttons_Left))
	{
		cursorLvl--;
		PlayMenuBipSound();
	}

	if ((PressedButtons[0] & Buttons_Right))
	{
		cursorLvl++;
		PlayMenuBipSound();
	}

	if (cursorLvl < 0)
		cursorLvl = levelMax;

	if (cursorLvl > levelMax)
		cursorLvl = 0;
}

void MultiMenuChild_TaskDisplay(ObjectMaster* obj)
{
	if (!obj)
		return;

	EntityData1* data = obj->Data1;

	njColorBlendingMode(0, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);

	switch (multiTask(data->Action))
	{
	case multiTask::charSelect:
		DrawSADXText("MULTIPLAYER - BATTLE", 0, 20, 120, 40);
		DrawSADXText("Character Select", 0, 50, 120, 40);

		for (int i = 0; i < 8; i++) {
			DrawChar_Cursor(i);
			DrawCharacterPortrait(i);
		}
		MultiMenu_DrawControls();

		break;
	case multiTask::levelSelect:

		DrawSADXText("MULTIPLAYER - BATTLE\n    Level Select", 0, 20, 120, 40);
		DrawLevel_Cursor();

		for (int i = 0; i < LengthOfArray(multiLevelsSonicArray); i++)
		{
			DrawMulti_LevelSelect(i);
		}

		MultiMenu_DrawControls();
		break;
	}

	njColorBlendingMode(0, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
}

void __cdecl MultiMenuExec_Display(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_disp(tp);
		return;
	}

	ObjectMaster* parent = (ObjectMaster*)tp;

	if (!MissedFrames && TrialActStelTp)
	{
		auto wk = (TrialActSelWk*)tp->awp;

		if (wk->BaseCol != 0)
		{

			gHelperFunctions->PushScaleUI(uiscale::Align_Center, false, 1.0f, 1.0f);
			SetDefaultAlphaBlend();

			MultiMenuChild_TaskDisplay(parent->Child);


			ClampGlobalColorThing_Thing();
			gHelperFunctions->PopScaleUI();
		}

		float a1 = wk->BaseZ - 2.0;
		DrawTiledBG_AVA_BACK(a1);
	}
}

bool MultiMenu_CheckMoveInput(int button, int pNum)
{
	//lock control if player has the character selected.

	if ((PressedButtons[pNum] & button) != 0)
	{
		switch (button)
		{
		case Buttons_Up:
			cursorChar[pNum] -= 4;
			break;
		case Buttons_Down:
			cursorChar[pNum] += 4;
			break;
		case Buttons_Left:
			cursorChar[pNum]--;
			break;
		case Buttons_Right:
			cursorChar[pNum]++;
			break;
		default:
			return false;
		}

		cursorChar[pNum] %= 8;

		PlayMenuBipSound();
	}

	return false;
}

void MultiMenu_InputCheck(task* tp, TrialActSelWk* wk)
{
	if (tp->ctp)
	{
		if (tp->ctp->twp->mode > (char)multiTask::charSelect)
		{
			return;
		}
	}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (MenuSelectButtonsPressed_r(i))
		{
			PlayMenuEnterSound();

			if (playerReady[i] < ready)
				playerReady[i]++;

			if (playerReady[i] >= ready)
			{
				SetCurrentCharacter(i, cursorChar[i]);
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
}

void MultiMenuChild_TaskManager(ObjectMaster* obj)
{
	EntityData1* data = obj->Data1;

	switch (multiTask(data->Action))
	{
	case multiTask::init:
		memset(playerReady, 0, sizeof(playerReady));
		ResetCharactersArray();
		player_count = 0;
		MusicList[MusicIDs_Trial].Name = "btl_sel";
		PlayMenuMusicID(4);
		PlayVoice(40);
		LoadPVM("AVA_MULTI", &AVA_MULTI_TEXLIST);
		//LoadPVM("ava_chsel_e", &ava_chsel_e_TEXLIST);
		playerReady[0] = pressedStart;
		SetDebugFontSize(13.0f * (unsigned short)VerticalResolution / 480.0f);
		SetDebugFontColor(0x8e8e8e);
		data->Action++;
		break;
	case multiTask::charSelect:
		if (isEveryoneReady())
		{
			PlayVoice(48);

			if (++data->InvulnerableTime == 20) {

				data->Action++;
				data->InvulnerableTime = 0;
			}
		}
		break;
	case multiTask::levelSelect:
		MultiMenu_InputLevel(data);
		break;
	case multiTask::finish:
		isStageSelected = false;
		njReleaseTexture(&AVA_MULTI_TEXLIST);
		CheckThingButThenDeleteObject(obj);
		break;
	}
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

		LoadChildObject(LoadObj_Data1, MultiMenuChild_TaskManager, (ObjectMaster*)tp);
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
		StartMulti_Game(tp, wk);
		break;
	case ADVA_STAT_FADEOUT:
		wk->T += MissedFrames_B * 0.1f;
		wk->BaseCol = GetFadeInColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->Stat = ADVA_STAT_REQWAIT;
			SeqTp->awp[1].work.ub[15] = 1;
			njReleaseTexture(&AVA_MULTI_TEXLIST);
			MenuLaunchNext();
			MultiMenuEnabled = false;
			isStageSelected = false;

			if (wk->SelStg != -1)
			{
				SeqTp->awp[1].work.sl[1] = 100;
			}
		}

		break;
	default:
		return;
	}

	RunObjectChildren((ObjectMaster*)tp);
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