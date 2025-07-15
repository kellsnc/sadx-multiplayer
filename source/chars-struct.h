#pragma once


struct ModelWeightInfo
{
	WeightInfo* weights;
	int rightHandNode = -1;
	int leftHandNode = -1;
	int rightFootNode = -1;
	int leftFootNode = -1;
	int user0Node = -1;
	int user1Node = -1;
	int rightHandDir;
	int leftHandDir;
	int rightFootDir;
	int leftFootDir;
	int user0Dir;
	int user1Dir;
};

struct nodeData
{
	int modelID = -1;
	int rightHandNode = -1;
	std::string rightHandDir;
	int leftHandNode = -1;
	std::string leftHandDir;
	int rightFootNode = -1;
	std::string rightFootDir;
	int leftFootNode = -1;
	std::string leftFootDir;
	int user0Node = -1;
	std::string user0Dir;
};

struct BasicWeight
{
	uint8_t mode;
	std::vector<std::shared_ptr<ModelInfo>> mdlList;
	size_t mdlListSize = 0;
	std::unordered_map<NJS_OBJECT*, ModelWeightInfo> modelWeights;
	nodeData nodePos;

};

struct PVMEntryDX
{
	std::string Name;
	NJS_TEXLIST* TexList = nullptr;
};

enum GUITexID
{
	m_chnam,
	b_chnamEng,
	b_chnamJP,
	recapScreen,
	lifeIcon,
	OneUp
};

enum ModelType : uint8_t
{
	normal,
	welds,
	morph,
	weights
};

struct NewSkin
{
	bool isSkin = false;
	std::vector<NJS_OBJECT> Obj;
	std::vector<NJS_MODEL_SADX> Mdl;
	std::vector<NJS_MOTION> Mtn;
	std::vector<NJS_ACTION> Act;
};

struct CharSettings
{
	bool EnableWelds = false;
	bool EnableEventHead = false;
	bool EnableMorph = false;
	bool EnableUpgrades = false;
	bool EnableJumpBall = true;
	bool useCustomHierarchy = false;
	bool isAltCostume = false;
	bool useCustomAnim = false;
	bool EnableTail = false;
};

struct ModelsData
{
	std::string Name;
	ModelType Type = normal;
	std::string Path;
	uint32_t id;
	std::shared_ptr<ModelInfo> Mdl;
};

struct NewChar
{
	std::string name;
	Characters characterTemplate;
	uint8_t charID = 0;
	AvaCharEnum charSelID = AVA_SONIC; //used to identify character on CharSel
	NJS_ACTION* charSelAnim[4];
	Float charSelAnimSpd[4];
	uint16_t Index = AVA_CHAR_MAX + 1; //used to identify character during gameplay
	ModelsData* modelsList = nullptr;
	PVMEntryDX* textures = nullptr;
	PVMEntryDX* GUITex = nullptr;
	PL_JOIN_VERTEX* welds = nullptr;
	BasicWeight* weights = nullptr;
	PL_ACTION* anim = nullptr;
	Float posYOffset = 0.0f;
	CharSettings settings;
};
