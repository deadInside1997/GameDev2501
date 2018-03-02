#pragma once
#include <glm/glm.hpp>
class Player {
	//stats governing performance
	int energy;
	float topSpeed;
	float acceleration; //acceleration in this case refers to the vehicle's ability to accelerate
	int mass;
	float boostMultiplier;

	//parameters relating to motion
	glm::vec2 position;
	float momentum; //momentum only applies to our y axis, since lateral movement will function differently, with little to no acceleration time. 

					//parameters affected by player action
	bool engine;
	bool boosting;
	bool braking;

private:

	void burnout();
public:
	void movementPhysics();
	void update();
	void startEngine();
	void stopEngine();
	void brake(); 
	void boost();
	void holdBoost();
	void changePosition(glm::vec2 pos);
	glm::vec2 getPosition();
	int getEnergy();
	Player(int nrg, float topspd, float acc, int mas, float boostMul);
};

