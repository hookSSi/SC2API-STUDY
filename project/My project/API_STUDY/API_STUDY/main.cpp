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
		switch (unit.unit_type.ToType())
		{
		case UNIT_TYPEID::ZERG_DRONE:
			cout << "일벌레가 만들어졌습니다!" << endl;
			// 추출장을 캡니다.
			MineExtractor(unit);
			break;
		case UNIT_TYPEID::ZERG_OVERLORD:
			cout << "대군주가 만들어졌습니다!" << endl;
			break;
		case UNIT_TYPEID::ZERG_EXTRACTOR:
			cout << "추출소가 만들어졌습니다!" << endl;
			break;
		default:
			break;
		}
	}

	virtual void OnUnitIdle(const Unit& unit) final {
		switch (unit.unit_type.ToType()) {
			// 일벌레 생산
			case UNIT_TYPEID::ZERG_LARVA: {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
				break;
			}
			// 놀고있는 일벌레 일 시킴
			case UNIT_TYPEID::ZERG_DRONE: {
				// 추출장을 캡니다.
				if (MineExtractor(unit)) {
					break;
				}
				// 미네랄을 찾습니다.
				uint64_t mineral_target;

				if (!FindNearestUnitPatch(unit.pos, UNIT_TYPEID::NEUTRAL_MINERALFIELD, Unit::Alliance::Neutral, mineral_target)) {
					break;
				}
				Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
				break;
				
			}
			default: {
				break;
			}
		}
	}

	virtual void OnStep() {
		// 추출장 건설
		BuildExtractor();

		// 대군주 생산
		TryTrainOverlord();
	}
private:
	// 오버로드 훈련
	bool TryTrainOverlord()
	{
		const ObservationInterface* observation = Observation();

		Units units = observation->GetUnits(Unit::Alliance::Self, 
			[](const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_EGG); });

		float foodUseCount = 0; // 사용될 인구수
		float foodGetCount = 0; // 생산될 인구수

		for (const auto& unit : units){
			for (const auto& order : unit.orders) {
				switch (order.ability_id.ToType()) {
				case ABILITY_ID::TRAIN_ZERGLING:
				case ABILITY_ID::TRAIN_DRONE:
				case ABILITY_ID::TRAIN_HYDRALISK:
				case ABILITY_ID::TRAIN_ROACH:
					foodUseCount += 1;
					break;
				case ABILITY_ID::TRAIN_QUEEN:
				case ABILITY_ID::TRAIN_MUTALISK:
				case ABILITY_ID::TRAIN_CORRUPTOR:
				case ABILITY_ID::TRAIN_INFESTOR:
					foodUseCount += 2;
					break;
				case ABILITY_ID::TRAIN_ULTRALISK:
					foodUseCount += 6;
					break;
				case ABILITY_ID::TRAIN_OVERLORD:
					foodGetCount += 8;
					break;
				default:
					break;
				}
			}
		}

		int resultNeedFood = (observation->GetFoodCap()+ foodGetCount) - (observation->GetFoodUsed() + foodUseCount);

		// 인구수가 막히지 않는다면 오버로드를 생산하지 않습니다.
		if (resultNeedFood > 0)
			return false;

		return TryTrainZergUnit(ABILITY_ID::TRAIN_OVERLORD, -1 * resultNeedFood);
	}
	// 추출장 건설
	bool BuildExtractor() {
		const ObservationInterface* observation = Observation();

		Units units = observation->GetUnits(Unit::Alliance::Self, 
			[](const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_EXTRACTOR);});

		char count = 0;

		for (const auto& unit : units) {
			count++;
		}

		// 추출장이 2개 이상
		if (count >= 2)
			return false;
		else
			return TryBuildZergBuilding(ABILITY_ID::BUILD_EXTRACTOR);
	}
	// 추출장을 캐도록 합니다.
	bool MineExtractor(const Unit& drone) {
		const ObservationInterface* observation = Observation();

		Units extractors = observation->GetUnits(Unit::Alliance::Self,
			[](const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_EXTRACTOR); });

		for (const auto& extractor : extractors) {
			if (extractor.assigned_harvesters < 3 && extractor.build_progress >= 1.0f) {
				Actions()->UnitCommand(drone, ABILITY_ID::SMART, extractor.tag);
				return true;
			}
		}

		return false;
	}
	// 원하는 저그 유닛을 만듭니다.
	bool TryTrainZergUnit(ABILITY_ID ability_type_for_train, char num)
	{
		for (int i = 0; i < num; i++)
		{
			const ObservationInterface* observation = Observation();

			Unit unit_to_train;
			Units units = observation->GetUnits(Unit::Alliance::Self,
				[](const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_LARVA); });

			for (const auto& unit : units) {
				unit_to_train = unit;
				break;
			}

			Actions()->UnitCommand(unit_to_train,
				ability_type_for_train);
		}

		return true;
	}
	// 원하는 저그 건물을 만듭니다.
	bool TryBuildZergBuilding(ABILITY_ID ability_type_for_build, const Point2D& point = Point2D(-1,-1)){
		const ObservationInterface* observation = Observation();

		Unit unit_to_build;
		Units units = observation->GetUnits(Unit::Alliance::Self,
			[](const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_DRONE); });

		for (const auto& unit : units) {
			/*for (const auto& order : unit.orders) {
				
				if (order.ability_id == ability_type_for_build) {
					return false;
				}
							
					else if (ABILITY_ID::BUILD_ARMORY <= order.ability_id && order.ability_id <= ABILITY_ID::BUILD_ULTRALISKCAVERN) {
					break;
				}
			}
			*/
			unit_to_build = unit;
			break;
		}

		// 가스 건물을 지을 경우
		if (ability_type_for_build == ABILITY_ID::BUILD_EXTRACTOR) {
			uint64_t targetTag;

			FindNearestUnitPatch(unit_to_build.pos, UNIT_TYPEID::NEUTRAL_VESPENEGEYSER, Unit::Alliance::Neutral, targetTag);

			Actions()->UnitCommand(unit_to_build,
				ability_type_for_build, targetTag);

			cout << "추출소를 지을 명령을 받았습니다." << endl;
		}
		else {
			if (point.x < 0 || point.y < 0)
				return false;

			Actions()->UnitCommand(unit_to_build,
				ability_type_for_build, point);
		}

		return true;
	}
	// 특정 위치에서 가장 가까운 유닛을 찾습니다.
	bool FindNearestUnitPatch(const Point2D& start, UNIT_TYPEID unitToFind, Unit::Alliance alliance,uint64_t& targetOutput) {
		Units units = Observation()->GetUnits(alliance);
		float distance = std::numeric_limits<float>::max();
		for (const auto& unit : units) {
			if (unit.unit_type == unitToFind) {
				float d = DistanceSquared2D(unit.pos, start);
				if (d < distance) {
					distance = d;
					targetOutput = unit.tag;
				}
			}
		}

		if (distance == std::numeric_limits<float>::max()) {
			return false;
		}

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