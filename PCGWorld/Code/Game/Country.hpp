#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;
class Culture;
class Religion;
class Town;
class City;
class Army;

enum class CountryRelationType {
	Friendly, Alliance, Hostile, War, Tributary, Suzerain, Vassal, Celestial, TribeUnion, None, Self
};

class Country {
public:
	Country();
	~Country();
	void Reinitialize();
	void GainProvince( MapPolygonUnit* prov, bool resetCapital = true );
	void LoseProvince( MapPolygonUnit* prov, bool resetCapital = false );
	void AnnexCountry( Country* country );
	void GetAdjacentCountries(std::vector<Country*>& out_countries) const;

	bool IsExist() const;

	// diplomacy helper functions
	bool IsVassal() const;
	bool IsTributary() const;
	bool IsIndependent() const;
	bool IsCelestial() const;
	bool IsInWar() const;
	bool IsInWarWith( Country* country ) const;
	bool HasVassal() const;
	bool HasTributary() const;
	bool IsMemberOfUnion() const;
	bool IsAdjacentToCountry( Country* country ) const;
	CountryRelationType GetRelationTo( Country* country ) const;
	bool HasNoRelationToCountry( Country* country ) const;
	bool HasNoRelationToCountryList( std::vector<Country*> const& countries )const;

	// will also remove the other side
	void RemoveAllRelations();
	void RemoveAllRelationWithCountry(Country* country);
	void AddRelationWithCountry( Country* country, CountryRelationType relation );
	void AddFriendlyRelation( Country* country );
	void RemoveFriendlyRelation( Country* country );
	void AddAllianceRelation( Country* country );
	void RemoveAllianceRelation( Country* country );
	void AddHostileRelation( Country* country );
	void RemoveHostileRelation( Country* country );
	void AddWarRelation( Country* country );
	void RemoveWarRelation( Country* country );
	void AddVassalRelation( Country* country );
	void RemoveVassalRelation( Country* country );
	void AddBeingVassalRelation( Country* country );
	void RemoveBeingVassalRelation( Country* country );
	void AddTribeUnionRelation( Country* country );
	void RemoveTribeUnionRelation( Country* country );
	void AddTributaryRelation( Country* country );
	void RemoveTributaryRelation( Country* country );
	void AddBeingTributaryRelation( Country* country );
	void RemoveBeingTributaryRelation( Country* country );

	int GetCulturePopulation( Culture* culture ) const;
	int GetReligionPopulation( Religion* religion ) const;
	float GetCultureInfluence( Culture* culture ) const;
	float GetReligionInfluence( Religion* religion ) const;

	// simulation
	void SetUpSimulation();
	void ExecuteCountryBehavior();
	void BeginTurn();
	void EndTurn();
	void ReCalculateCultureAndReligion();
	void CalculateMilitaryStats();
	void CalculateEconomicValue();
	float CompareStrengthToCountry( Country* country ) const;
	float CompareStrengthToCountryDuel( Country* country ) const;
	bool ReadyToWarWithCountry( Country* country ) const;
	bool CannotAffordWarWithCountry( Country* country ) const;
	float GetAggressiveValue();
	float GetCountryMilitaryStatus() const;
	int GetMonthlyIncome() const;
	//int GetMonthlyCost() const;
	void GetAllCountriesWithMilitaryAccess( std::vector<Country*> out_countries );
	bool DoesCountryHaveMilitaryAccess( Country* country ) const;
	float GetCombatValueMultiplier() const;
	float GetEconomicValueMultiplier() const;

	void RemoveArmyFromList( Army* army );

	bool ChangeNameTitle();
public:
	void AddCulturePopulation( int totalPopulation, std::vector<std::pair<Culture*, float>> const& cultureInfluence );
	void AddReligionPopulation( int totalPopulation, std::vector<std::pair<Religion*, float>> const& religionInfluence );
	void CalculateMajorCulture();
	void CalculateMajorReligion();

	void ResetCapitalProvince(); // only reset capital province, may reset capital city if there is one in that province

	void GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const;
	Vec2 GetGeometricCenter() const;

	int GetWarTimeWith( Country* country ) const;

	int m_funds = 0;
	int m_economyValue = 0;
	int m_totalMilitaryStrength = 0;

	int m_id = 0;
	Rgba8 m_color;
	std::string m_name;
	CountryGovernmentType m_governmentType = CountryGovernmentType::None;
	MapPolygonUnit* m_capitalProv = nullptr;
	MapPolygonUnit* m_originProv = nullptr;
	City* m_capitalCity = nullptr;
	std::vector<MapPolygonUnit*> m_provinces;
	std::vector<City*> m_cities;
	std::vector<Town*> m_towns;
	std::vector<std::pair<Culture*, int>> m_cultures;
	std::vector<std::pair<Religion*, int>> m_religions;
	Culture* m_majorCulture = nullptr;
	Religion* m_majorReligion = nullptr;
	int m_totalPopulation = 0;
	Culture* m_countryCulture = nullptr;
	Religion* m_countryReligion = nullptr;
	std::vector<Army*> m_armies;
	VertexBuffer* m_edgeShowingBuffer = nullptr;
	VertexBuffer* m_edgeShowingBuffer3D = nullptr;

	std::vector<Country*> m_relationFriendlyCountries;
	std::vector<Country*> m_relationAllianceCountries;
	std::vector<Country*> m_relationHostileCountries;
	std::vector<Country*> m_relationWarCountries;
	std::vector<std::pair<Country*, int>> m_warTime;

	Country* m_relationSuzerain = nullptr;
	std::vector<Country*> m_relationVassals;

	bool m_isCelestial = false;
	Country* m_relationCelestialEmpire = nullptr;
	std::vector<Country*> m_relationTributaries;

	std::vector<Country*> m_tribeUnions;
protected:
	bool IsFriendlyWith( Country* country ) const;
	bool IsHostileWith( Country* country ) const;
	bool IsAlliedWith( Country* country ) const;
	bool IsInTribeUnionWith( Country* country ) const;
	bool IsTributaryOf( Country* country ) const;
	bool IsSuzerainOf( Country* country ) const;
	bool IsVassalOf( Country* country ) const;
	bool IsCelestialOf( Country* country ) const;

	void SpendMoney( int amount );
	void DeleteSmallCultureAndReligion();
	MapPolygonUnit* GetNearestProvinceToEnemyProvince( MapPolygonUnit* enemyProv ) const;
	std::mutex m_mutex;
};