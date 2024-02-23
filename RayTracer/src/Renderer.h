#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include <memory>
#include <glm/glm.hpp>

class Renderer {

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

private:
	glm::vec4 PerPixel(glm::vec2 coord);

private:
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;

};