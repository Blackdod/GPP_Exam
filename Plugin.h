#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "SteeringBehaviours.h"
#include "EBlackboard.h"
#include "Behaviors.h"

class IBaseInterface;
class IExamInterface;


class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	UINT m_InventorySlot = 0;

	//Steering behaviours
	Arrive* m_pArrive{nullptr};
	Wander* m_pWander{nullptr};
	Seek* m_pSeek{};
	Flee* m_pFlee{};
	bool m_MovingToHouse{ false };
	Elite::Vector2 m_EntranceHouse{};
	float m_CurrentTimeInHouse{};
	bool m_EntraceSet{ false };
	bool m_LeavingHouse{ false };
	HouseInfo* m_pCurrentHouse{ new HouseInfo{} };
	HouseInfo* m_pPreviousHouse{ new HouseInfo{} };
	const float m_MaxTimeBeforeNewHouseSelect{ 15.f };
	float m_CurrentTimeAfterHouseSelection{m_MaxTimeBeforeNewHouseSelect};

	void PickupFunctionality(float dt, EntityInfo& entity, AgentInfo& agent, SteeringPlugin_Output& steering);
	void MedkitFunctionality(AgentInfo& agent);
	void FoodFunctionality(AgentInfo& agent);

	bool AgentInHouse(AgentInfo& agent, HouseInfo& houseInfo);
	void HouseFunctionality(float dt, SteeringPlugin_Output& steering, AgentInfo& agentInfo, Elite::Vector2& nextTargetPos);

	bool LookingAtEnemy(AgentInfo& agent, EntityInfo& enemy);

	void Plugin::UsePistol();

	Elite::Blackboard* Plugin::CreateBlackboard(AgentInfo& a);
	Elite::Blackboard* m_pBlackboard{};

	Elite::BehaviorTree* m_pBehaviourTree{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}