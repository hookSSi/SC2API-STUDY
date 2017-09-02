#include <sc2api/sc2_api.h>

#include <iostream>

using namespace sc2;
using namespace std;

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		cout << "Hello, World!" << endl;
	}

	virtual void OnUnitCreated(const Unit& unit) final {

		if (unit.unit_type == UNIT_TYPEID::ZERG_DRONE)
		{
			cout << unit.unit_type.to_string() << "has created!" << endl;
			cout << "�̳׶�: " << Observation()->GetMinerals() << endl;
			cout << "����: " << Observation()->GetVespene() << endl;
		}
	}

	virtual void OnUnitIdle(const Unit& unit) final {
		switch (unit.unit_type.ToType()) {
			case UNIT_TYPEID::ZERG_LARVA: {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
				break;
			}
			default: {
				break;
			}

		}
	}

	virtual void OnStep() {
		TryTrainOverlord();
	}
private:
	bool TryTrainOverlord()
	{
		const ObservationInterface* observation = Observation();

		// �α����� ������ �ʴ´ٸ� �����ε带 �������� �ʽ��ϴ�.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
			return false;

		return TryTrainZergUnit(ABILITY_ID::TRAIN_OVERLORD);
	}

	bool TryTrainZergUnit(ABILITY_ID ability_type_for_train, UNIT_TYPEID unit_type = UNIT_TYPEID::ZERG_LARVA)
	{
		const ObservationInterface* observation = Observation();

		Unit unit_to_train;
		Units units = observation->GetUnits(Unit::Alliance::Self);

		// ���ϴ� ������ �̹� ���� ���̶�� ������ �Ӵϴ�.
		for (const auto& unit : units) {
			for (const auto& order : unit.orders) {
				if (order.ability_id == ability_type_for_train)
				{
					return false;
				}
			}

			if (unit.unit_type == unit_type) {
				unit_to_train = unit;
			}
		}

		Actions()->UnitCommand(unit_to_train,
			ability_type_for_train);

		return true;
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