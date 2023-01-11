#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

using namespace std;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Sifitus";
	info.Student_FirstName = "Siebe";
	info.Student_LastName = "Boeckx";
	info.Student_Class = "2DAE07";

	//Set flee radius using agent view range
	m_pFlee->SetFleeRadius(m_pInterface->Agent_GetInfo().FOV_Range);

	AgentInfo* m_pAgent = &(m_pInterface->Agent_GetInfo());
	m_pBlackboard = CreateBlackboard(*m_pAgent);
	//m_pBlackboard->ChangeData("Arrive", m_pArrive);
	//m_pBlackboard->ChangeData("Wander", m_pWander);
	//m_pBlackboard->ChangeData("Seek", m_pSeek);
	//m_pBlackboard->ChangeData("Flee", m_pFlee);

	m_pBehaviourTree = new Elite::BehaviorTree(m_pBlackboard,
		new Elite::BehaviorSelector(
			{
				new Elite::BehaviorSelector({
				//Flee from enemies
				new Elite::BehaviorSequence(
					{
						new Elite::BehaviorConditional(BT_Conditions::IsEnemyInFOV),
						new Elite::BehaviorAction(BT_Actions::FleeEnemy),
						new Elite::BehaviorConditional(BT_Conditions::IsLookingAtEnemy),
						new Elite::BehaviorAction(BT_Actions::UsePistol)
					}),
					//Pickup items
						new Elite::BehaviorSequence(
						{
							new Elite::BehaviorConditional(BT_Conditions::IsItemInFOV),
							new Elite::BehaviorAction(BT_Actions::PickupFunctionality)
						}),
				}),
				new Elite::BehaviorSelector(
				{
				//Add house if in FOV
					new Elite::BehaviorSequence(
					{
						new Elite::BehaviorConditional(BT_Conditions::NewHouseInFOV),
					}),
				//Fallback to house movement/wander
				new Elite::BehaviorSequence(
					{
						new Elite::BehaviorConditional(BT_Conditions::ShouldMoveToHouse),
						new Elite::BehaviorAction(BT_Actions::HouseFunctionality),
						new Elite::BehaviorConditional(BT_Conditions::AgentInHouse)
					})
				})
			})
	);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	m_pPreviousHouse->Center = { 0,0 };
	m_pPreviousHouse->Size = { 0,0 };

	//Steering behaviours
	m_pArrive = new Arrive();
	m_pWander = new Wander();
	m_pSeek = new Seek();
	m_pFlee = new Flee();
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	delete m_pArrive;
	delete m_pWander;
	delete m_pSeek;
	delete m_pFlee;
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = false;
	params.SpawnDebugShotgun = false;
	params.SpawnPurgeZonesOnMiddleClick = false;
	params.PrintDebugMessages = false;
	params.ShowDebugItemNames = true;
	params.Seed = 36;
	params.SpawnZombieOnRightClick = false;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	m_pBlackboard->ChangeData("dt", dt);
	
	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();


	//Use the navmesh to calculate the next navmesh point
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	//OR, Use the mouse target
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target); //Uncomment this to use mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	std::vector<HouseInfo*> housesInFOV{};
	std::vector<EntityInfo*> enemiesInFOV{};
	std::vector<EntityInfo*> itemsInFOV{};

	for (auto house : vHousesInFOV)
	{
		housesInFOV.push_back(&house);
	}
	m_pBlackboard->ChangeData("HousesInFOV", housesInFOV);

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			//std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
		else if (e.Type == eEntityType::ENEMY)
		{
			enemiesInFOV.push_back(&e);
		}
		else if (e.Type == eEntityType::ITEM)
		{
			itemsInFOV.push_back(&e);
		}
	}

	m_pBlackboard->ChangeData("EnemiesInFOV", enemiesInFOV);
	m_pBlackboard->ChangeData("ItemsInFOV", itemsInFOV);

	////INVENTORY USAGE DEMO
	////********************
	//
	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (m_pInterface->Item_Grab({}, item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		m_pInterface->Inventory_AddItem(m_InventorySlot, item);
	//	}
	//}
	//
	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	m_pInterface->Inventory_UseItem(m_InventorySlot);
	//}
	//
	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	m_pInterface->Inventory_RemoveItem(m_InventorySlot);
	//}
	//
	////Simple Seek Behaviour (towards Target)
	//steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	//
	//if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	//{
	//	steering.LinearVelocity = Elite::ZeroVector2;
	//}
	//
	////steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrient to TRue overrides the AngularVelocity
	//
	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	//
	////SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

//@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	//MedkitFunctionality(agentInfo);
	//FoodFunctionality(agentInfo);

	m_pBehaviourTree->Update(dt);

	SteeringPlugin_Output steering{};
	m_pBlackboard->GetData("SteeringBehaviour", steering);
	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}


void Plugin::PickupFunctionality(float dt, EntityInfo& entity,  AgentInfo& agent, SteeringPlugin_Output& steering)
{
	m_pArrive->SetTarget(entity);
	steering = m_pArrive->CalculateSteering(dt, &agent);

	if (entity.Location.DistanceSquared(agent.Position) <= agent.GrabRange * agent.GrabRange)
	{
		ItemInfo info{};
		ItemInfo tempInfo{};
		m_pInterface->Item_GetInfo(entity, info);

		switch (info.Type)
		{
		case eItemType::GARBAGE:
			m_pInterface->Item_Destroy(entity);
			break;
		case eItemType::PISTOL:
		{
			if (m_pInterface->Inventory_GetItem(0, tempInfo))
			{
				if (m_pInterface->Weapon_GetAmmo(tempInfo) < m_pInterface->Weapon_GetAmmo(info))
				{
					m_pInterface->Inventory_RemoveItem(0);
					m_pInterface->Item_Grab(entity, info);
					m_pInterface->Inventory_AddItem(0, info);
					return;
				}
				else //destroy item if worse to clear item spawn point
				{
					m_pInterface->Item_Destroy(entity);
					return;
				}
			}
			else
			{
				m_pInterface->Item_Grab(entity, info);
				m_pInterface->Inventory_AddItem(0, info);
				return;
			}
		}
		case eItemType::SHOTGUN:
		{
			if (m_pInterface->Inventory_GetItem(1, tempInfo))
			{
				if (m_pInterface->Weapon_GetAmmo(tempInfo) < m_pInterface->Weapon_GetAmmo(info))
				{
					m_pInterface->Inventory_RemoveItem(1);
					m_pInterface->Item_Grab(entity, info);
					m_pInterface->Inventory_AddItem(1, info);
					return;
				}
				else //destroy item if worse to clear item spawn point
				{
					m_pInterface->Item_Destroy(entity);
					return;
				}
			}
			else
			{
				m_pInterface->Item_Grab(entity, info);
				m_pInterface->Inventory_AddItem(1, info);
				return;
			}
		}

		case eItemType::MEDKIT:
		{
			if (m_pInterface->Inventory_GetItem(2, tempInfo))
			{
				if (m_pInterface->Medkit_GetHealth(tempInfo) < m_pInterface->Medkit_GetHealth(info))
				{
					m_pInterface->Inventory_RemoveItem(2);
					m_pInterface->Item_Grab(entity, info);
					m_pInterface->Inventory_AddItem(2, info);
					return;
				}
				//Don't destroy item here because there's a second slot to check
			}
			else
			{
				m_pInterface->Item_Grab(entity, info);
				m_pInterface->Inventory_AddItem(2, info);
				return;
			}

			if (m_pInterface->Inventory_GetItem(3, tempInfo))
			{
				if (m_pInterface->Medkit_GetHealth(tempInfo) < m_pInterface->Medkit_GetHealth(info))
				{
					m_pInterface->Inventory_RemoveItem(3);
					m_pInterface->Item_Grab(entity, info);
					m_pInterface->Inventory_AddItem(3, info);
					return;
				}
				else //destroy item if worse to clear item spawn point
				{
					m_pInterface->Item_Destroy(entity);
					return;
				}
			}
			else
			{
				m_pInterface->Item_Grab(entity, info);
				m_pInterface->Inventory_AddItem(3, info);
				return;
			}
		}
		case eItemType::FOOD:
		{
			if (m_pInterface->Inventory_GetItem(4, tempInfo))
			{
				if (m_pInterface->Food_GetEnergy(tempInfo) < m_pInterface->Food_GetEnergy(info))
				{
					m_pInterface->Inventory_RemoveItem(4);
					m_pInterface->Item_Grab(entity, info);
					m_pInterface->Inventory_AddItem(4, info);
					return;
				}
				else //destroy item if worse to clear item spawn point
				{
					m_pInterface->Item_Destroy(entity);
					return;
				}
			}
			else
			{
				m_pInterface->Item_Grab(entity, info);
				m_pInterface->Inventory_AddItem(4, info);
				return;
			}
		}
		default:
			return;
		}
	}
}

void Plugin::MedkitFunctionality(AgentInfo& agent)
{
	ItemInfo info{};

	const float maxHealth{ 10.f };

	bool useSecondMedkit{ false };

	if (m_pInterface->Inventory_GetItem(2, info)) //Use second medkit if first is non-existant or empty
	{
		if (m_pInterface->Medkit_GetHealth(info) <= 0)
		{
			useSecondMedkit = true;
		}
	}
	else //case non-existant
	{
		useSecondMedkit = true;
	}
	

	if (m_pInterface->Inventory_GetItem(3, info) == false && useSecondMedkit)
	{
		std::cout << "No medkits\n";
		return;
	}

	if (useSecondMedkit)
	{
		std::cout << "Using second\n";
	}

	if (agent.Health < maxHealth )
	{
	
		if (maxHealth - agent.Health > m_pInterface->Medkit_GetHealth(info))
		{
			if (!useSecondMedkit)
			{
				m_pInterface->Inventory_UseItem(2);
			}
			else
			{
				m_pInterface->Inventory_UseItem(3);
			}
		}
	}
}

void Plugin::FoodFunctionality(AgentInfo& agent)
{
	ItemInfo info{};

	const float maxEnergy{ 10.f };

	if (m_pInterface->Inventory_GetItem(4, info) == false)
	{
		std::cout << "No food\n";
		return;
	}

	if (agent.Energy < maxEnergy)
	{
		if ((maxEnergy - agent.Energy) >= m_pInterface->Food_GetEnergy(info))
		{
			m_pInterface->Inventory_UseItem(4);
		}
	}
}

bool Plugin::AgentInHouse(AgentInfo& agent, HouseInfo& houseInfo)
{
	const Elite::Vector2 agentPos = agent.Position;

	//House corners
	const Elite::Vector2 bottomLeft = Elite::Vector2{ houseInfo.Center.x - houseInfo.Size.x / 2,
													  houseInfo.Center.y - houseInfo.Size.y / 2 };

	const Elite::Vector2 topRight = Elite::Vector2{ houseInfo.Center.x + houseInfo.Size.x / 2,
													houseInfo.Center.y + houseInfo.Size.y / 2 };
	
	if (agentPos.x > bottomLeft.x && agentPos.x < topRight.x)
	{
		if (agentPos.y > bottomLeft.y && agentPos.y < topRight.y)
		{
			return true;
		}
	}

	return false;
}

void Plugin::HouseFunctionality(float dt, SteeringPlugin_Output& steering, AgentInfo& agentInfo, Elite::Vector2& nextTargetPos)
{
	const float maxTimeIdleInHouse{ 3.f };
	nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	m_pInterface->Draw_Circle(nextTargetPos, 1, { 0,0,1 }, m_pInterface->NextDepthSlice());
	m_pInterface->Draw_Circle(m_EntranceHouse, 1, { 0,1,0 }, m_pInterface->NextDepthSlice());

	if (AgentInHouse(agentInfo, *m_pCurrentHouse) && !m_EntraceSet && !m_LeavingHouse) //set and remember previously seen house
	{
		m_EntranceHouse = agentInfo.Position;
		m_EntraceSet = true;
		m_pPreviousHouse = m_pCurrentHouse;
	}

	if (!AgentInHouse(agentInfo, *m_pCurrentHouse)) //go to the house
	{
		if (m_Target == m_EntranceHouse) //Wander when outside the house after leaving
		{
			steering = m_pWander->CalculateSteering(dt, &agentInfo);
			return;
		}
		m_pSeek->SetTarget(nextTargetPos);
		steering = m_pSeek->CalculateSteering(dt, &agentInfo);
		m_MovingToHouse = true;
		m_LeavingHouse = false;
		m_CurrentTimeInHouse = 0;
		return;
	}
	
	if (AgentInHouse(agentInfo, *m_pCurrentHouse))
	{
		if (m_CurrentTimeInHouse > maxTimeIdleInHouse) //Seek to exit house after a while
		{
			m_Target = m_EntranceHouse;
			m_pSeek->SetTarget(nextTargetPos);
			steering = m_pSeek->CalculateSteering(dt, &agentInfo);
			m_EntraceSet = false;
			m_LeavingHouse = true;
			m_MovingToHouse = false;
			return;
		}
		else if (!m_LeavingHouse) //Look around the house
		{
			steering = m_pWander->CalculateSteering(dt, &agentInfo);
			m_CurrentTimeInHouse += dt;
			return;
		}
	}
	
	//if (m_pPreviousHouse->Size != Elite::Vector2{0,0}) //Only possible if remembering house
	//{
	//	if (m_MovingToHouse) //Move to said house even if you don't see it
	//	{
	//		nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	//		m_pInterface->Draw_Circle(nextTargetPos, 1, { 0,0,1 }, m_pInterface->NextDepthSlice());
	//		m_pInterface->Draw_Circle(m_EntranceHouse, 1, { 0,1,0 }, m_pInterface->NextDepthSlice());
	//
	//
	//		if (!AgentInHouse(agentInfo, *m_pPreviousHouse))
	//		{
	//			m_pSeek->SetTarget(nextTargetPos);
	//			steering = m_pSeek->CalculateSteering(dt, &agentInfo);
	//		}
	//	}
	//	else
	//	{
	//		steering = m_pWander->CalculateSteering(dt, &agentInfo);
	//	}
	//}
	//else
	//{
	//	steering = m_pWander->CalculateSteering(dt, &agentInfo);
	//	m_CurrentTimeInHouse = 0;
	//}
}

bool Plugin::LookingAtEnemy(AgentInfo& agent, EntityInfo& enemy)
{
	const Elite::Vector2 desiredVector = Elite::Vector2(enemy.Location - agent.Position);
	const Elite::Vector2 lookVector{ std::cosf(agent.Orientation),std::sinf(agent.Orientation) };

	bool returnBool{ false };

	if (fabsf(Elite::AngleBetween(desiredVector, lookVector)) < 0.1f)
	{
		returnBool = true;
	}

	return returnBool;
}

void Plugin::UsePistol()
{
	ItemInfo gunInfo{};
	if (m_pInterface->Inventory_GetItem(0, gunInfo))
	{
		if (m_pInterface->Weapon_GetAmmo(gunInfo) > 0)
		{
			m_pInterface->Inventory_UseItem(0);
		}
	}
}

Elite::Blackboard* Plugin::CreateBlackboard(AgentInfo& a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("Target", Elite::Vector2{});
	pBlackboard->AddData("dt", 0.0f);
	pBlackboard->AddData("Interface", static_cast<IExamInterface*>(m_pInterface));

	//Steering behaviours
	pBlackboard->AddData("SteeringBehaviour", SteeringPlugin_Output{});
	pBlackboard->AddData("Seek", Seek{});
	pBlackboard->AddData("Wander", Wander{});
	pBlackboard->AddData("Flee", Flee{});
	pBlackboard->AddData("Arrive", Arrive{});

	//House stuff
	pBlackboard->AddData("HousesInFOV", std::vector<HouseInfo*>{});
	pBlackboard->AddData("FoundHouses", std::vector<std::pair<HouseInfo*, float>>{});
	pBlackboard->AddData("TargetHouse", std::pair<HouseInfo*, float>{});
	pBlackboard->AddData("HouseEntrance", Elite::Vector2{});
	pBlackboard->AddData("EntranceSet", bool{ false });
	pBlackboard->AddData("LeavingHouse", bool{ false });
	pBlackboard->AddData("MovingToHouse", bool{ false });
	pBlackboard->AddData("CurrentTimeInHouse", float{});
	pBlackboard->AddData("AgentInHouse", bool{ false });

	//Item and enemy stuff (entities)
	pBlackboard->AddData("EnemiesInFOV", std::vector<EntityInfo*>{});
	pBlackboard->AddData("TargetEnemy", EntityInfo{});
	pBlackboard->AddData("ItemsInFOV", std::vector<EntityInfo*>{});

	//pBlackboard->AddData("AgentFleeTarget", static_cast<EntityInfo*>(nullptr)); // Needs the cast for the type
	return pBlackboard;
}