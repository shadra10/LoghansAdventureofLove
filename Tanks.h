#ifndef Tanks_H_
#define Tanks_H_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "resource.h"
#include "Enemy.h"

namespace game {

	// Abstraction of an asteroid
	class Tanks : public Enemy {

	public:
		// Create asteroid from given resources
		Tanks(const std::string name, const Resource *geometry, const Resource *material, const Resource *tex);

		// Destructor
		~Tanks();

	

		// Get/set attributes specific to asteroids
		glm::quat GetAngM(void) const;
		void SetAngM(glm::quat angm);

		// Update geometry configuration
		void Update(void);

		void die();


	private:
		// Angular momentum of asteroid
		glm::quat angm_;
		
	}; // class Asteroid

} // namespace game

#endif // ASTEROID_H_
