#pragma once
#include "Lizard.h"
#include "entity_control.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <chrono>
#include "socket.h"
#include "client.h"

std::vector<std::pair<std::string, Data::Entity*>> groupOne;
std::vector<std::pair<std::string, Data::Entity*>> groupTwo;

std::chrono::steady_clock::time_point lastPhaseChangeTime = std::chrono::steady_clock::now();

Data::Color groupOneColor = Data::Color{ 0, 0.58, 1, 1 };
Data::Color groupTwoColor = Data::Color{ 1, 0, 1, 1 };

int groupSize = 2;

float phaseTime = 7.5;

int phase = 0;

namespace Boss {
	namespace Lizard {
		void RandomizeColors(void) {
#ifdef  MASTER_CLIENT
			std::map<std::string, Data::Entity*> players = Entity::GetAllPlayers();

			// Get a random seed
			std::random_device rd;
			// Use the random seed to seed the random number generator
			std::mt19937 gen(rd());

			// First random selection
			std::sample(players.cbegin(), players.cend(), std::back_inserter(groupOne), groupSize * 2, gen);

			std::shuffle(groupOne.begin(), groupOne.end(), gen);

			for (int i = 0; i < groupSize; i++) {
				groupTwo.push_back(*groupOne.begin());
				groupOne.erase(groupOne.begin());
			}

			for (std::pair<std::string, Data::Entity*> player : groupOne) {
				Socket::SendData("4:" + player.first + ":" + std::to_string(groupOneColor.red) + 
					":" + std::to_string(groupOneColor.green) + 
					":" + std::to_string(groupOneColor.blue) + 
					":" + std::to_string(groupOneColor.alpha));
			}
			for (std::pair<std::string, Data::Entity*> player : groupTwo) {
				Socket::SendData("4:" + player.first + ":" + std::to_string(groupTwoColor.red) +
					":" + std::to_string(groupTwoColor.green) +
					":" + std::to_string(groupTwoColor.blue) +
					":" + std::to_string(groupTwoColor.alpha));
			}
#endif
		}

		void ClearColors(void) {
			groupOne.clear();
			groupTwo.clear();
			for (std::pair<std::string, Data::Entity*> player : Entity::GetAllPlayers()) {
				Entity::SetNameColor(player.first, Data::Color{ 0, 0, 0, 0 });
			}
		}

		void CheckForValidGroup(std::vector<std::pair<std::string, Data::Entity*>> group, std::vector<std::pair<std::string, Data::Entity*>> outGroup) {
			for (std::pair<std::string, Data::Entity*> player : group) {
				std::vector<std::string> nearPlayers =
					Entity::GetClosePlayers(reinterpret_cast<DWORD*>(Entity::GetEntity(player.first).pointer), 5);

				for (std::pair<std::string, Data::Entity*> in : group) {
					if (std::find(nearPlayers.begin(), nearPlayers.end(), in.first) == nearPlayers.end()) {
						Socket::SendData("2:" + player.first + ":1000000:0");
					}
				}

				for (std::pair<std::string, Data::Entity*> out : outGroup) {
					if (std::find(nearPlayers.begin(), nearPlayers.end(), out.first) != nearPlayers.end()) {
						Socket::SendData("2:" + player.first + ":1000000:0");
					}
				}
			}
		}

		void CheckForAllValidPositions(void) {
#ifdef MASTER_CLIENT
			CheckForValidGroup(groupOne, groupTwo);
			CheckForValidGroup(groupTwo, groupOne);
#endif
		}

		void PhaseChange(void) {
			switch (phase) {
			    case 0: RandomizeColors(); break;
				case 1: CheckForAllValidPositions(); ClearColors(); break;
			}
			phase = (phase + 1) % 2;
		}

		void Update(void) {
			if ((std::chrono::steady_clock::now() - lastPhaseChangeTime).count() / pow(10, 9) > phaseTime) {
				lastPhaseChangeTime = std::chrono::steady_clock::now();
				PhaseChange();
			}
		}

		void EnterCombat(void) {
			lastPhaseChangeTime = std::chrono::steady_clock::now();
		}

		void ExitCombat(void) {
			phase = 0;
			ClearColors();
		}
	}
}