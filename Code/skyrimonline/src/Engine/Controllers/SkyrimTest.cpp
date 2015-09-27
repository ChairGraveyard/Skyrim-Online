#pragma once

#include <stdafx.h>

#include <messages/Client/GameCli_Handler.h>
#include <Engine/Controllers/SkyrimTest.h>
#include <Engine/Controllers/SkyrimHelpers.h>
#include <skyscript.h>
#include <enums.h>
#include <GameRTTI.h>
#include <mhook.h>
#include <PapyrusVM.h>
#include <GameForms.h>
#include <GameObjects.h>

namespace Logic
{
	namespace Engine
	{
		namespace Controllers
		{
			SkyrimTest::SkyrimTest() : m_jumped(false)
			{
				ScriptDragon::TESObjectREFR* pMe = (ScriptDragon::TESObjectREFR*)ScriptDragon::Game::GetPlayer();
				Actor* pActor = (Actor*)ScriptDragon::Game::GetPlayer();
				ScriptDragon::CActor* pNPC = (ScriptDragon::CActor*)pActor->baseForm;
				IFormFactory* pFactory = IFormFactory::GetFactoryForType(kFormType_NPC);
				TESNPC* pNpc = (TESNPC*)pFactory->Create();
				pNpc->CopyFromEx((TESForm*)pNPC);

				Messages::Npc serializeStruct;
				Serialize((TESNPC*)pNPC, serializeStruct);
				Deserialize(pNpc, serializeStruct);

				pNpc->fullName.name = BSFixedString("Lol");
				
				m_pActor = (Actor*)ScriptDragon::ObjectReference::PlaceActorAtMe(pMe, (ScriptDragon::TESNPC*)pNpc, 4, NULL);
			}
			
			SkyrimTest::~SkyrimTest()
			{
			}

			void SkyrimTest::Update()
			{
				Actor* pActor = (Actor*)ScriptDragon::Game::GetPlayer();

				BSFixedString str("bAnimationDriven");
				bool success = m_pActor->animGraphHolder.SetVariableBool(&str, false);

				BSFixedString str2("bMotionDriven");
				success &= m_pActor->animGraphHolder.SetVariableBool(&str2, false);

				BSFixedString str3("bInMoveState");
				m_pActor->animGraphHolder.SetVariableBool(&str3, false);

				BSFixedString directionStr("Direction");
				float dir = 0.0f;
				pActor->animGraphHolder.GetVariableFloat(&directionStr, &dir);
				m_pActor->animGraphHolder.SetVariableFloat(&directionStr, dir);

				//if (m_jumped == false)
				{

					BSFixedString speedStr("Speed");
					float fSpeed = 0.0f;

					pActor->animGraphHolder.GetVariableFloat(&speedStr, &fSpeed);
					//success &= m_pActor->animGraphHolder.SetVariableFloat(&speedStr, fSpeed);

					std::ostringstream oss;
					oss << fSpeed;

					//TheController->GetUI()->Debug(oss.str());

					/*BSFixedString speedWalkStr("SpeedWalk");
					m_pActor->animGraphHolder.SetVariableFloat(&speedWalkStr, fSpeed);

					BSFixedString speedRunStr("SpeedRun");
					m_pActor->animGraphHolder.SetVariableFloat(&speedRunStr, fSpeed);

					BSFixedString SpeedAcc("SpeedAcc");
					m_pActor->animGraphHolder.SetVariableFloat(&SpeedAcc, fSpeed);

					BSFixedString animStr("MoveStart");
					m_pActor->animGraphHolder.SendAnimationEvent(&animStr);

					BSFixedString animSprintStr("SprintStart");
					m_pActor->animGraphHolder.SendAnimationEvent(&animSprintStr);*/

					BSFixedString iState("iState"), iState_NPCDefault("iState_NPCDefault"), iState_NPCSprinting("iState_NPCSprinting");
					UInt32 State, DefaultState, IsSprinting;

					pActor->animGraphHolder.GetVariableInt(&iState, &State);
					pActor->animGraphHolder.GetVariableInt(&iState_NPCDefault, &DefaultState);
					pActor->animGraphHolder.GetVariableInt(&iState_NPCSprinting, &IsSprinting);

					//ScriptDragon::ObjectReference::SetPosition((ScriptDragon::TESObjectREFR *)m_pActor, pActor->pos.x, pActor->pos.y + 50.0f, pActor->pos.z);

					if (State == DefaultState)
					{
						BSFixedString animStr("MoveStop"), animSprintStr("SprintStop");

						m_pActor->animGraphHolder.SendAnimationEvent(&animStr);
						m_pActor->animGraphHolder.SendAnimationEvent(&animSprintStr);
					}
					else if (State == IsSprinting)
					{
						BSFixedString animStr("MoveStart"), animSprintStr("SprintStart");

						m_pActor->animGraphHolder.SendAnimationEvent(&animStr);
						m_pActor->animGraphHolder.SendAnimationEvent(&animSprintStr);
					}

					/*if (m_pActor->animGraphHolder.SendAnimationEvent(&animStr) && success)
					{
						m_jumped = true;
					}*/
				}
			}
		}
	}
}