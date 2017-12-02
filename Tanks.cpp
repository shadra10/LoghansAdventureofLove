#include "Tanks.h"

namespace game {

	Tanks::Tanks(const std::string name, const Resource *geometry, const Resource *material, const Resource * tex) : Enemy(name, geometry, material, tex) {
		speed = 0.5;
		radius = 1;
		damage = 10;
	}


	Tanks::~Tanks() {
	}


	glm::quat Tanks::GetAngM(void) const {

		return angm_;
	}


	void Tanks::SetAngM(glm::quat angm) {

		angm_ = angm;
	}


	void Tanks::Update(void) {

		if (shootTimer <= 0) {
			shootTimer = 120;
			shooting = true;
		}
		else
			shooting = false;

		shootTimer--;

		position_.y-=0.3;

		if (position_.y < -13) position_.y = -13;


	}

	void Tanks::die() {
		draw = false;
	}



} // namespace game
