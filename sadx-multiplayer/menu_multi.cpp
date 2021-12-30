#include "pch.h"
#include "menu_multi.h"


extern bool MultiMenuEnabled;

bool isStageSelected = false;

const char levelMax = 10;

uint8_t cursorChar[PLAYER_MAX];
uint8_t cursorLvl;

NJS_TEXNAME multicharTex[10];
NJS_TEXLIST multichar_Texlist = { arrayptrandlength(multicharTex) };

NJS_TEXNAME multiLevelTex[11];
NJS_TEXLIST multiLevel_Texlist = { arrayptrandlength(multiLevelTex) };

NJS_TEXNAME multiLegendTex[3];
NJS_TEXLIST multiLegend_Texlist = { arrayptrandlength(multiLegendTex) };

NJS_TEXANIM multiLegendTexAnim[]{
	{ 110, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, 0, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, 1, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, 2, 0x20},
};

NJS_SPRITE MultiLegendSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multiLegend_Texlist, multiLegendTexAnim };

void MultiMenu_DrawControls()
{
	gHelperFunctions->PushScaleUI((uiscale::Align)(Align_Bottom | Align_Center_Horizontal), false, 1.0f, 1.0f);

	MultiLegendSprite.p.y = 448.0f;

	MultiLegendSprite.p.x = 165.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, 0, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	MultiLegendSprite.p.x = 330.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, 1, -64, NJD_SPRITE_ALPHA);

	MultiLegendSprite.p.x = 495.0f;
	njDrawSprite2D_DrawNow(&MultiLegendSprite, 2, -64, NJD_SPRITE_ALPHA);

	gHelperFunctions->PopScaleUI();
}



enum class multiTask {
	init,
	charSelect,
	levelSelect,
	finish
};

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

NJS_TEXANIM MultiLevel_TexAnim[]{
	{ 128, 64, 0, 0, 0, 0, 255, 255, 0, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 1, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 2, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 3, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 4, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 5, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 6, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 7, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 8, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 9, 0x20 },
	{ 128, 64, 0, 0, 0, 0, 255, 255, 10, 0x20 },
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

NJS_SPRITE MultiLevel_Sprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multiLevel_Texlist, MultiLevel_TexAnim };

NJS_SPRITE MultiLevelCursor_Sprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multiLevel_Texlist, MultiLevel_TexAnim };


void DrawMulti_LevelSelect(char i) {

	SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
	MultiLevel_Sprite.p.x = cursorLevel_PosArray[i].x;
	MultiLevel_Sprite.p.y = cursorLevel_PosArray[i].y;
	njDrawSprite2D_DrawNow(&MultiLevel_Sprite, i + 1, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	return;
}

int playerReady[PLAYER_MAX];

enum playReadyState {
	disconnected,
	pressedStart,
	ready
};


void SetPlayerToUnready() {

	for (int i = 0; i < PLAYER_MAX; i++) {

		if (playerReady[i] >= ready) 
			playerReady[i] = pressedStart;
		
	}
	return;
}

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

NJS_POINT2 cursorCharPos_Array[8]{
	{ 200, 150 },
	{ 260, 150 },
	{ 320, 150 },
	{ 380, 150 },
	{ 200, 220 },
	{ 260, 220 },
	{ 320, 220 },
	{ 380, 220 },
};

NJS_SPRITE MultiCharTexSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim };

NJS_SPRITE MultiChar_CursorSprite[8] = {
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
			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d Disconnected", i + 1);
		}

		if (playerReady[i] == ready) {
			countRDY++;

			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d READY!", i + 1);
		}

		if (playerReady[i] == pressedStart) {
			countNotRDY++;

			DisplayDebugStringFormatted(NJM_LOCATION(2, 7 + i), "Player %d Select character...", i + 1);
		}
	}

	if (countNotRDY <= 0 && countRDY >= 2)
		return true;

	return false;
}

void StartMulti_Game(task* tp, TrialActSelWk* wk) {

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
	njReleaseTexture(&multichar_Texlist);
}

void DrawCharacterPortrait(char i) {

	SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
	MultiCharTexSprite.p.x = cursorCharPos_Array[i].x;
	MultiCharTexSprite.p.y = cursorCharPos_Array[i].y;
	njDrawSprite2D_DrawNow(&MultiCharTexSprite, i, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	return;
}

void DrawChar_Cursor(char pnum) {

	if (playerReady[pnum] < pressedStart || pnum > PLAYER_MAX)
		return;

	SetMaterialAndSpriteColor_Float(1, CursorColor[pnum].x, CursorColor[pnum].y, CursorColor[pnum].z);
	MultiChar_CursorSprite[pnum].p.x = cursorCharPos_Array[cursorChar[pnum]].x;
	MultiChar_CursorSprite[pnum].p.y = cursorCharPos_Array[cursorChar[pnum]].y;

	njDrawSprite2D_DrawNow(&MultiChar_CursorSprite[pnum], cursorIcon + 1, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	ClampGlobalColorThing_Thing();
}

void DrawLevel_Cursor() {

	SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
	MultiLevelCursor_Sprite.p.x = cursorLevel_PosArray[cursorLvl].x;
	MultiLevelCursor_Sprite.p.y = cursorLevel_PosArray[cursorLvl].y;

	njDrawSprite2D_DrawNow(&MultiLevelCursor_Sprite, 0, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
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


void MultiMenuChild_TaskDisplay(ObjectMaster* obj) {

	if (!obj)
		return;

	EntityData1* data = obj->Data1;

	njColorBlendingMode(0, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);


	switch (multiTask(data->Action))
	{
	case multiTask::charSelect:
		DrawSADXText("MULTIPLAYER - BATTLE\n     Character Select", 0, 20, 120, 40);

		for (int i = 0; i < 8; i++) {
			DrawChar_Cursor(i);
			DrawCharacterPortrait(i);
		}
		MultiMenu_DrawControls();

		break;
	case multiTask::levelSelect:

		DrawSADXText("MULTIPLAYER - BATTLE\n    Level Select", 0, 20, 120, 40);
		DrawLevel_Cursor();

		for (int i = 0; i < LengthOfArray(multiLevelsSonicArray); i++) {

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

bool MultiMenu_CheckMoveInput(int button, char pNum)
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

		if (cursorChar[pNum] < sonicIcon)
			cursorChar[pNum] = bigIcon;

		if (cursorChar[pNum] > msIcon)
			cursorChar[pNum] = sonicIcon;

		PlayMenuBipSound();

	}

	return false;
}

void MultiMenu_InputCheck(task* tp, TrialActSelWk* wk)
{

	if (tp->ctp)
	{
		if (tp->ctp->twp->mode > (char)multiTask::charSelect) {
			return;
		}
	}

	for (int i = 0; i < PLAYER_MAX; i++) {

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

	return;
}

void MultiMenuChild_TaskManager(ObjectMaster* obj) {
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
		LoadPVM("multichar", &multichar_Texlist);
		LoadPVM("multi_Levels", &multiLevel_Texlist);
		LoadPVM("MultiLegend", &multiLegend_Texlist);
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
		njReleaseTexture(&multiLevel_Texlist);
		njReleaseTexture(&multiLegend_Texlist);
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
			njReleaseTexture(&multichar_Texlist);
			njReleaseTexture(&multiLevel_Texlist);
			njReleaseTexture(&multiLegend_Texlist);
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