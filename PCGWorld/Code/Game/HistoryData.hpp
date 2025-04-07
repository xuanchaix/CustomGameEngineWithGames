#pragma once
#include "Game/GameCommon.hpp"
class Map;

struct HistoryProvinceData {
	bool m_isWater = false;
	int m_population = 0;
	std::vector<std::pair<int, float>> m_cultures;
	std::vector<std::pair<int, float>> m_religions;
	int m_ownerCountryID = -1;
	std::vector<int> m_legalCountriesID;
};

struct HistoryCountryData {
	bool m_exist = true;
	int m_funds = 0;
	int m_countryCultureID = -1;
	int m_capitalProvID = -1;
	int m_countryReligionID = -1;
	int m_capitalCityID = -1;
	std::vector<int> m_friendlyCountriesID;
	std::vector<int> m_allianceCountriesID;
	std::vector<int> m_hostileCountriesID;
	std::vector<std::pair<int, int>> m_warCountriesID;
	int m_suzerainCountryID = -1;
	std::vector<int> m_vassalCountriesID;
	int m_celestialCountryID = -1;
	std::vector<int> m_tributaryCountriesID;
	bool m_isCelestial = false;
	int m_governmentType = (int)CountryGovernmentType::None;
};

struct HistoryArmyData {
	HistoryArmyData() {};
	HistoryArmyData( int size, float combatValue, int provInID, int globalID, int ownerID, int targetProvID );
	int m_size = 0;
	float m_combatValue = 0.5f;
	int m_provInID = -1;
	int m_globalID = -1;
	int m_ownerID = -1;
	int m_targetProvID = -1;
};

struct HistoryTownData {
	int m_population = 0;
	std::vector<std::pair<int, float>> m_cultures;
	std::vector<std::pair<int, float>> m_religions;
	int m_ownerID = -1;
	float m_defenseValue = 0.f;
};

struct HistoryCityData : public HistoryTownData {
	uint16_t m_type = 0;
};

struct HistoryCrisisData {
	int m_type = 3;
	float m_progress = 0.f;
	int m_countryID = -1;
	int m_cultureOrReligionID = -1;
	unsigned int m_globalID = (unsigned int)-1;
};

struct HistoryData {
	HistoryData() {};
	HistoryData( Map* map );
	std::vector<HistoryProvinceData> m_provinceData;
	std::vector<HistoryCountryData> m_countryData;
	std::vector<HistoryArmyData> m_armyData;
	std::vector<HistoryCityData> m_cityData;
	std::vector<HistoryTownData> m_townData;
	std::vector<HistoryCrisisData> m_crisisData;

	void PrintOutHistory( XmlDocument* document,  XmlElement* rootElem, HistoryData* provMonth ) const;
	void DumpToBinaryFormat( std::vector<uint8_t>& bin ) const;
	void LoadFromBinaryFormat( std::vector<uint8_t> const& bin );
};


class HistorySavingSolver {
public:
	HistorySavingSolver( Map* map );
	void StartSave();
	bool IsSaving() const;
	void GetSavingProgress( int& curHistory, int& totalHistory );
	void Update();

	Map* m_map = nullptr;
	std::atomic<bool> m_isSaving = false;
	std::vector<bool> m_savingFlag;
	std::deque<Job*> m_jobList;
	std::mutex m_mutex;
};