#include <sc2api/sc2_api.h>

#include <iostream>

using namespace sc2;
using namespace std;

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		cout << "Hello, World!" << endl;
	}

	virtual void OnUnitCreated(const Unit& unit)final {

		if (unit.unit_type == UNIT_TYPEID::ZERG_DRONE)
		{
			cout << unit.unit_type.to_string() << "has created!" << endl;
			cout << "¹Ì³×¶ö: " << Observation()->GetMinerals() << endl;
			cout << "°¡½º: " << Observation()->GetVespene() << endl;
		}
	}
};

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Zerg, &bot),
		CreateComputer(Race::Terran)
	});

	coordinator.LaunchStarcraft();
	coordinator.StartGame(sc2::kMapBelShirVestigeLE);

	while (coordinator.Update()) {
	}

	return 0;
}