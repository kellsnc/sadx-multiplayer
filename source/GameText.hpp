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
	bool m_initialized = false;

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
		if (m_initialized)
		{
			MSG_Close(&msgc);
			msgc = {};
			m_text.empty();
			m_initialized = false;
		}
	}

	void Initialize(const char* text, int x, int y)
	{
		Free();
		MSG_Open(&msgc, x, y, 640, 40, gbix);
		MSG_Cls(&msgc);
		SetText(text);
		m_initialized = true;
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