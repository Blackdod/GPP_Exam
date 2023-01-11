#include "stdafx.h"
#include "SteeringBehaviours.h"
#include "EliteMath/EVector2.h"
#include "Box2D/Collision/Shapes/b2CircleShape.h"

SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	steering.LinearVelocity = m_Target.Location - pAgent->Position;
	steering.LinearVelocity.Normalize();

	const Elite::Vector2 toTarget{ m_Target.Location - pAgent->Position };
	const float distanceSquared{ toTarget.MagnitudeSquared() };

	if (distanceSquared < m_StopRadius * m_StopRadius)
	{
		steering.LinearVelocity = Elite::Vector2{ 0.f,0.f };
		return steering;
	}

	if (distanceSquared < m_SlowRadius * m_SlowRadius)
	{
		steering.LinearVelocity *= pAgent->MaxLinearSpeed * distanceSquared / (m_SlowRadius * m_SlowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}

	//LookAt
	const Elite::Vector2 desiredVector = Elite::Vector2(m_Target.Location - pAgent->Position);
	const Elite::Vector2 lookVector{ std::cosf(pAgent->Orientation),std::sinf(pAgent->Orientation) };

	float angle{ Elite::AngleBetween(desiredVector, lookVector) };

	if (!(fabsf(Elite::AngleBetween(lookVector, desiredVector)) < 0.1f))
	{
		if (Elite::AngleBetween(lookVector, desiredVector) > 0)
		{
			steering.AngularVelocity = pAgent->MaxAngularSpeed;
		}
		else
		{
			steering.AngularVelocity = -pAgent->MaxAngularSpeed;
		}

	}

	return steering;
}

SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	steering.LinearVelocity = m_Target.Location - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	steering.AngularVelocity = pAgent->MaxAngularSpeed;

	return steering;
}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	b2CircleShape wanderCircle{};
	//const Elite::Vector2 directionVector{ pAgent->GetPosition() - m_Target.Position}; //desired direction
	Elite::Vector2 circleCenter = pAgent->LinearVelocity.GetNormalized() * m_OffsetDistance + pAgent->Position; //circle center
	wanderCircle.m_p.Set(circleCenter.x, circleCenter.y);
	wanderCircle.m_radius = m_Radius;
	
	const float addedAngle{ Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange) }; //random angle withing maxAngle interval
	
	m_WanderAngle += addedAngle; //add direction change
	//std::cout << m_WanderAngle << '\n';
	
	const Elite::Vector2 targetPoint{ circleCenter + Elite::Vector2{cosf(m_WanderAngle), sinf(m_WanderAngle)}.GetNormalized() * m_Radius }; //calculate target point
	
	m_Target.Location = targetPoint;
	steering = Seek::CalculateSteering(deltaT, pAgent);
	
	return steering;
}

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	const Elite::Vector2 toTarget{ m_Target.Location - pAgent->Position };
	const float distanceSquared{ toTarget.MagnitudeSquared() };

	if (distanceSquared < m_FleeRadius * m_FleeRadius)
	{
		steering.LinearVelocity = pAgent->Position - m_Target.Location;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}

	//LookAt
	const Elite::Vector2 desiredVector = Elite::Vector2(m_Target.Location - pAgent->Position);
	const Elite::Vector2 lookVector{ std::cosf(pAgent->Orientation),std::sinf(pAgent->Orientation) };

	float angle{ Elite::AngleBetween(desiredVector, lookVector) };

	if (!(fabsf(Elite::AngleBetween(lookVector, desiredVector)) < 0.1f))
	{
		if (Elite::AngleBetween(lookVector, desiredVector) > 0)
		{
			steering.AngularVelocity = pAgent->MaxAngularSpeed;
		}
		else
		{
			steering.AngularVelocity = -pAgent->MaxAngularSpeed;
		}

	}

	return steering;
}