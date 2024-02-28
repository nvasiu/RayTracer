#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include <memory>
#include <glm/glm.hpp>
#include "Scene.h"
#include "Ray.h"

class Renderer {

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

private:
	struct HitObject {
		float HitDistance;
		glm::vec3 HitPosition;
		glm::vec3 HitNormal;

		int ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y);
	HitObject TraceRay(const Ray& ray);
	HitObject ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitObject Miss(const Ray& ray);

private:
	const Scene* activeScene = nullptr;
	const Ray* cameraRay = nullptr;

	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;

};