#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material {
	glm::vec3 Albedo{1.0f}; // Color of material without light
	float Roughness = 1.0f; // How much diffuse reflectiveness the material has
	float Metallic = 0.0f; // How much specular reflectiveness the material has

	glm::vec3 EmissionColor{0.0f}; // Color of light emitted
	float EmissionPower = 0.0f; // Brightness of light emitted

	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
};

struct Sphere {
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	float Radius = 0.5f;
	Material SphereMaterial;
};

struct Scene {
	std::vector<Sphere> Spheres;
};