#include"player.h"
#include<algorithm>
Player::Player(int nrg, float topspd, float acc, int mas, float boostMul)
{
	energy = nrg;
	topSpeed = topspd;
	acceleration = acc;
	mass = mas;
	boostMultiplier = boostMul;

	//intitalize parameters that default to zero
	engine = 0;
	boosting = 0;
	braking = 0;
	momentum = 0;
	position = glm::vec2(0, 0);
}

void Player::update()
{
	movementPhysics();
}

void Player::movementPhysics()
{
	//recall: p=mv (momentum is equal to velocity times mass), so V = P/M
	if (boosting)
	{
		energy--;
		if (energy <= 0)
		{
			burnout();
		}
		position.y += boostMultiplier * (momentum / mass); //We divide momentum by mass, and add that to our velocity
		momentum += (acceleration * mass); //We increment our momentum equal to acceleration times mass
	}
	else if (engine == true/*If we're holding forward*/) {
		position.y += (momentum / mass); //We divide momentum by mass, and add that to our velocity
		momentum += (acceleration * mass); //We increment our momentum equal to acceleration times mass
	}
	else if (braking)
	{
		position.y += (momentum / mass); //We divide momentum by mass, and add that to our velocity
		momentum =  std::max(0.0, (momentum - (mass * 0.5)));
	}
	else
	{
		position.y += (momentum / mass); //We divide momentum by mass, and add that to our velocity
		momentum = std::max(0.0, (momentum - (mass * 0.0005)));
	}
}

void Player::burnout()
{
	//crash and burn... implement this later
}

void Player::boost()
{
	boosting = true;
}

void Player::brake()
{
	braking = true;
	boosting = false;
	engine = false;
}

void Player::holdBoost()
{
	boosting = false;
}


void Player::startEngine()
{
	engine = true;
	braking = false;
}

void Player::stopEngine()
{
	engine = false;
}
void Player::changePosition(glm::vec2 pos)
{
	position = pos;
}

int Player::getEnergy()
{
	return energy;
}

glm::vec2 Player::getPosition()
{
	return position;
}