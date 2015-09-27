#pragma once

#include <stdafx.h>

#include <Engine/Controllers/SkyrimController.h>
#include <Engine/Controllers/SkyrimHelpers.h>
#include <skyscript.h>
#include <GameForms.h>
#include <GameReferences.h>
#include <GameObjects.h>
#include <GameRTTI.h>
#include <cmath>
#include <Overlay\Chat.h>
#include "easylogging++.h"
#include <chrono>

namespace Logic
{
	namespace Engine
	{
		namespace Controllers
		{
			SkyrimPlayer::SkyrimPlayer(bool aLocalPlayer)
				: m_pCharacter(nullptr)
				, m_pHorse(nullptr)
				, m_positionTimer(0)
				, m_localPlayer(aLocalPlayer)
			{
				if (m_localPlayer)
				{
					LOG(INFO) << "event=skyrim_player_create type=local";
					m_pCharacter = (Actor*)ScriptDragon::Game::GetPlayer();
				}
				else
				{
					LOG(INFO) << "event=skyrim_player_create type=remote";
				}
			}

			SkyrimPlayer::~SkyrimPlayer()
			{
				LOG(INFO) << "event=skyrim_player_delete";
				if (!m_localPlayer)
				{
					TESNPC* pNpc = (TESNPC*)m_pCharacter->baseForm;

					ScriptDragon::ObjectReference::Disable((ScriptDragon::TESObjectREFR*)m_pCharacter, false);
					ScriptDragon::ObjectReference::Delete((ScriptDragon::TESObjectREFR*)m_pCharacter);

					m_pCharacter = nullptr;
				}
			}

			void SkyrimPlayer::Create(const Messages::GameCli_PlayerAddRecv& acMsg)
			{
				ScriptDragon::TESObjectREFR* pLocalPlayer = (ScriptDragon::TESObjectREFR*)ScriptDragon::Game::GetPlayer();

				Actor* pActor = (Actor*)pLocalPlayer;
				ScriptDragon::TESNPC* pNPC = (ScriptDragon::TESNPC*)pActor->baseForm;
				IFormFactory* pFactory = IFormFactory::GetFactoryForType(kFormType_NPC);
				TESNPC* pNpc = (TESNPC*)pFactory->Create();
				pNpc->CopyFromEx((TESForm*)pNPC);

				//Deserialize(pNpc, acMsg.player_npc);

				pNpc->fullName.name = BSFixedString(acMsg.name.c_str());
				
				ScriptDragon::CActor* test = ScriptDragon::ObjectReference::PlaceActorAtMe(pLocalPlayer, (ScriptDragon::TESNPC*)pNpc, 4, NULL);
				//LOG(INFO) << "DEBUG: test " << test;
				m_pCharacter = (Actor*)test;
			
				m_positionTimer = acMsg.movement.time;
				m_futurePosition = acMsg.movement;

				if (m_pCharacter)
				{
					LOG(INFO) << "event=m_pCharacter exist";
					m_pCharacter->pos.x = m_futurePosition.pos.x;
					m_pCharacter->pos.y = m_futurePosition.pos.y;
					m_pCharacter->pos.z = m_futurePosition.pos.z;
					m_pCharacter->rot.x = m_futurePosition.rotX;
					m_pCharacter->rot.y = m_futurePosition.rotY;
					m_pCharacter->rot.z = m_futurePosition.rotZ;

					BSFixedString str2("bMotionDriven");
					m_pCharacter->animGraphHolder.SetVariableBool(&str2, false);
				}

				LOG(INFO) << "event=skyrim_player_spawn";
			}
			std::fstream f("test.log", std::ios::trunc | std::ios::out);

			void SkyrimPlayer::Update(uint32_t aDeltaClock)
			{
				if (m_pCharacter == NULL || m_pCharacter == nullptr)
					return;

				/*BSFixedString IsNPC("IsNPC");
				m_pCharacter->animGraphHolder.SetVariableBool(&IsNPC, false);

				BSFixedString IsPlayer("IsPlayer");
				m_pCharacter->animGraphHolder.SetVariableBool(&IsPlayer, true);*/

				//LOG(INFO) << "event=Update_start";
				m_positionTimer += aDeltaClock;
				if (m_localPlayer)
				{
					if (m_positionTimer >= kPositionDelay)
					{
						m_positionTimer = 0;
						SendMovementUpdate();
					}
				}
				else
				{
					//f << "Position time: " << m_positionTimer << " Future Position: " << m_futurePosition.time << '\n';
					if (m_positionTimer < m_futurePosition.time)
					{
						// Linear interpolation stuff
						float aLerp = float(aDeltaClock) / (float(m_futurePosition.time - m_positionTimer));
						aLerp = aLerp > 1.0 ? 1.0 : aLerp;
						float bLerp = 1.0f - aLerp;

						/*m_pCharacter->pos.x = m_pCharacter->pos.x * bLerp + m_futurePosition.pos.x * aLerp;
						m_pCharacter->pos.y = m_pCharacter->pos.y * bLerp + m_futurePosition.pos.y * aLerp;
						m_pCharacter->pos.z = m_pCharacter->pos.z * bLerp + m_futurePosition.pos.z * aLerp;*/
						ScriptDragon::ObjectReference::SetPosition((ScriptDragon::TESObjectREFR *)m_pCharacter, m_pCharacter->pos.x * bLerp + m_futurePosition.pos.x * aLerp,
							m_pCharacter->pos.y * bLerp + m_futurePosition.pos.y * aLerp, m_pCharacter->pos.z * bLerp + m_futurePosition.pos.z * aLerp);

						time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

						f << ctime(&tt) << " aLerp: " << aLerp << " bLerp: " << bLerp << '\n';
						//f << aDeltaClock << " Position X: " << m_pCharacter->pos.x << " Future position X: " << m_pCharacter->pos.x * bLerp + m_futurePosition.pos.x * aLerp << '\n';

						m_pCharacter->rot.x = m_pCharacter->rot.x * bLerp + m_futurePosition.rotX * aLerp;
						m_pCharacter->rot.y = m_pCharacter->rot.y * bLerp + m_futurePosition.rotY * aLerp;
						m_pCharacter->rot.z = m_pCharacter->rot.z * bLerp + m_futurePosition.rotZ * aLerp;
						/*ScriptDragon::ObjectReference::SetAngle((ScriptDragon::TESObjectREFR *)m_pCharacter, m_pCharacter->rot.x * bLerp + m_futurePosition.rotX * aLerp,
							m_pCharacter->rot.y * bLerp + m_futurePosition.rotY * aLerp, m_pCharacter->rot.z * bLerp + m_futurePosition.rotZ * aLerp);*/

						m_positionTimer = min(m_positionTimer, m_futurePosition.time);
						m_positionTimer = m_positionTimer > m_futurePosition.time ? 0 : m_positionTimer;
					}
					else
					{
						/*m_pCharacter->pos.x = m_futurePosition.pos.x;
						m_pCharacter->pos.y = m_futurePosition.pos.y;
						m_pCharacter->pos.z = m_futurePosition.pos.z;*/
						ScriptDragon::ObjectReference::SetPosition((ScriptDragon::TESObjectREFR *)m_pCharacter, m_futurePosition.pos.x, m_futurePosition.pos.y, m_futurePosition.pos.z);

						m_pCharacter->rot.x = m_futurePosition.rotX;
						m_pCharacter->rot.y = m_futurePosition.rotY;
						m_pCharacter->rot.z = m_futurePosition.rotZ;
						//ScriptDragon::ObjectReference::SetAngle((ScriptDragon::TESObjectREFR *)m_pCharacter, m_futurePosition.rotX, m_futurePosition.rotY, m_futurePosition.rotZ);
					}

					f.flush();

					//m_pCharacter->actorState.flags04 = m_futurePosition.animationFlags04;

					switch (m_futurePosition.animationFlags04)
					{
						case 0:
						{
							BSFixedString iIsInSneak("iIsInSneak"), animStr("MoveStop"), animSprintStr("SprintStop");

							//m_pCharacter->animGraphHolder.SetVariableInt(&iIsInSneak, false);

							m_pCharacter->animGraphHolder.SendAnimationEvent(&animStr);
							m_pCharacter->animGraphHolder.SendAnimationEvent(&animSprintStr);

							m_isWalking = false;
						}
						case 166: // Walking normaly.
						{
							if (m_isWalking == false)
							{
								m_isWalking = true;

								/*BSFixedString str2("bMotionDriven");
								success &= m_pCharacter->animGraphHolder.SetVariableBool(&str2, false);*/

								/*BSFixedString str("bAnimationDriven");
								bool success = m_pCharacter->animGraphHolder.SetVariableBool(&str, true);*/

								BSFixedString str3("bInMoveState");
								m_pCharacter->animGraphHolder.SetVariableBool(&str3, true);

								/*BSFixedString directionStr("Direction");
								m_pCharacter->animGraphHolder.SetVariableFloat(&directionStr, m_futurePosition.dir);*/

								BSFixedString speedFloat("Speed");
								m_pCharacter->animGraphHolder.SetVariableFloat(&speedFloat, m_futurePosition.velocity);

								/*BSFixedString animStr("MoveStart");
								m_pCharacter->animGraphHolder.SendAnimationEvent(&animStr);*/
							}
						}
						case 422: // Sprinting.
						{
							/*BSFixedString str("bAnimationDriven");
							m_pCharacter->animGraphHolder.SetVariableBool(&str, true);*/

							BSFixedString str3("bInMoveState");
							m_pCharacter->animGraphHolder.SetVariableBool(&str3, true);

							/*BSFixedString directionStr("Direction");
							m_pCharacter->animGraphHolder.SetVariableFloat(&directionStr, m_futurePosition.dir);
*/
							BSFixedString speedFloat("Speed");
							m_pCharacter->animGraphHolder.SetVariableFloat(&speedFloat, m_futurePosition.velocity);

							BSFixedString animStr("MoveStart"), animSprintStr("SprintStart");

							m_pCharacter->animGraphHolder.SendAnimationEvent(&animStr);
							m_pCharacter->animGraphHolder.SendAnimationEvent(&animSprintStr);
						}
						case 512: // Sneaking state.
						{
							/*BSFixedString iIsInSneak("iIsInSneak");
							m_pCharacter->animGraphHolder.SetVariableInt(&iIsInSneak, true);*/
						}
					}

					
				}

				//LOG(INFO) << "event=Update_end";
			}

			void SkyrimPlayer::SendMovementUpdate()
			{
				//LOG(INFO) << "event=SendMovementUpdate_start";

				Messages::CliGame_PositionSend* pPacket = new Messages::CliGame_PositionSend;
				pPacket->movement.pos.x = m_pCharacter->pos.x;
				pPacket->movement.pos.y = m_pCharacter->pos.y;
				pPacket->movement.pos.z = m_pCharacter->pos.z;
				pPacket->movement.rotX = m_pCharacter->rot.x;
				pPacket->movement.rotY = m_pCharacter->rot.y;
				pPacket->movement.rotZ = m_pCharacter->rot.z;
				pPacket->movement.animationFlags04 = m_pCharacter->actorState.flags04;
				pPacket->movement.animationFlags08 = m_pCharacter->actorState.flags08;
				pPacket->movement.time = clock();

				BSFixedString speedStr("Speed");
				m_pCharacter->animGraphHolder.GetVariableFloat(&speedStr, &pPacket->movement.velocity);

				BSFixedString directionStr("Direction");
				m_pCharacter->animGraphHolder.GetVariableFloat(&directionStr, &pPacket->movement.dir);

				TheController->Send(pPacket);

				//LOG(INFO) << "event=SendMovementUpdate_send";
			}

			const std::string SkyrimPlayer::GetName()
			{
				TESNPC* pNpc = DYNAMIC_CAST(m_pCharacter->baseForm, TESForm, TESNPC);

				if (pNpc)
				{
					return pNpc->fullName.name.data;
				}

				return "ERROR";
			}

			void SkyrimPlayer::InitializeServerNode()
			{
				//LOG(INFO) << "event=skyrim_player_initialize start";
				Messages::CliGame_PlayerInitializeSend* pPacket = new Messages::CliGame_PlayerInitializeSend;

				Actor* pHorse = (Actor*)ScriptDragon::Game::GetPlayersLastRiddenHorse();
				pPacket->horseId = pHorse ? pHorse->baseForm->formID : 0;
				
				Serialize((TESNPC*)m_pCharacter->baseForm, pPacket->player_npc);
				pPacket->movement.pos.x = m_pCharacter->pos.x;
				pPacket->movement.pos.y = m_pCharacter->pos.y;
				pPacket->movement.pos.z = m_pCharacter->pos.z;
				pPacket->movement.rotX = m_pCharacter->rot.x;
				pPacket->movement.rotY = m_pCharacter->rot.y;
				pPacket->movement.rotZ = m_pCharacter->rot.z;
				pPacket->movement.animationFlags04 = m_pCharacter->actorState.flags04;
				pPacket->movement.animationFlags08 = m_pCharacter->actorState.flags08;
				pPacket->movement.time = clock();

				BSFixedString speedStr("Speed");
				m_pCharacter->animGraphHolder.GetVariableFloat(&speedStr, &pPacket->movement.velocity);

				TheController->SendReliable(pPacket);

				//LOG(INFO) << "event=skyrim_player_initialize";
			}

			void SkyrimPlayer::PushMovement(const Messages::Movement& acMovement)
			{
				//LOG(INFO) << "event=PushMovement start";

				if (m_positionTimer > acMovement.time)
				{
					return;
				}
				// If the packet received is more than 1 second ahead in the future, sync to this one
				else if (acMovement.time - m_positionTimer > CLOCKS_PER_SEC)
				{
					// Induce 200ms lag to allow smooth interpolations
					m_positionTimer = acMovement.time - CLOCKS_PER_SEC / 5;
				}

				m_futurePosition = acMovement;

				//LOG(INFO) << "event=PushMovement";
			}
		}
	}
}