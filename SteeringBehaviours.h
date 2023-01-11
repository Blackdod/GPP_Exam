#pragma once
#include "Exam_HelperStructs.h"


#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	//Seek Functions
	void SetTarget(const EntityInfo& target) { m_Target = target; }
	void SetTarget(const Elite::Vector2& target) { m_Target.Location = target; }
	void SetFleeRadius(float radius) { m_FleeRadius = radius; };

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	EntityInfo m_Target;
	float m_FleeRadius;
};
#pragma endregion

class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetSlowRadius(float size) { m_SlowRadius = size; };
	void SetTargetRadius(float distance) { m_StopRadius = distance; };

private:
	float m_SlowRadius{ 2.f };
	float m_StopRadius{ 2.f };
};

class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

	//Arrive Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	float m_OffsetDistance{ 10.0f }; // Offset (agent direction)
	float m_Radius{ 4.0f }; // Wander radius
	float m_MaxAngleChange{ Elite::ToRadians(30) }; // Max wander angle change per frame
	float m_WanderAngle{}; //Internal
	//const float m_TimeBetweemDirectionChange{0.5f};
	//float m_CurrentTime{0};
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};