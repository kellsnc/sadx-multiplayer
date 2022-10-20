#pragma once

#include "SADXVariables.h"
#include "SADXStructsNew.h"
#include "SADXFunctionsNew.h"

static void* gPreviousText = nullptr;

class GameText
{
private:
	MSGC msgc = {};
	int gbix = 0xD0000000 + ((int)this % 0xFFFFFFF);
	std::string m_text;

public:
	void Draw()
	{
		// We have to recalculate the texture if we draw more than one text because of HD GUI
		if (gPreviousText != this)
		{
			MSG_Cls(&msgc);
			MSG_Puts(&msgc, m_text.c_str());
			MSG_LoadTexture(&msgc);
			gPreviousText = this;
		}

		MSG_Disp(&msgc);
	}

	void SetText(const char* text)
	{
		m_text = text;
		gPreviousText = this;

		MSG_Cls(&msgc);
		MSG_Puts(&msgc, text);
		MSG_LoadTexture(&msgc);
	}

	void SetText(std::string& text)
	{
		SetText(text.c_str());
	}

	void Free()
	{
		MSG_Close(&msgc);
		msgc = {};
	}

	void Initialize(const char* text, int x, int y)
	{
		Free();
		MSG_Open(&msgc, x, y, 640, 40, gbix);
		MSG_Cls(&msgc);
		SetText(text);
	}

	void Initialize(std::string& text, int x, int y)
	{
		Initialize(text.c_str(), x, y);
	}

	GameText() = default;

	GameText(const char* text, int x, int y)
	{
		Initialize(text, x, y);
	}

	~GameText()
	{
		Free();
	}
};