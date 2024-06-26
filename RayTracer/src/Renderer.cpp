#include "Renderer.h"
#include <execution>

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		// RGBA is 4 bytes or 32 bits (1 byte or 8 bits per channel)
		// The vec4 values are in a 0 to 1 range, they need to be converted to RGBA values (0 to 255)
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		// RGBA has the format 0xAABBGGRR
		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height) 
{
	// Create the image on the first frame and recreate it when the image window is resized

	if (finalImage) {
		if (finalImage->GetWidth() == width && finalImage->GetHeight() == height) return;
		// Walnut::Image has a resize function that reallocates its own memory (it also only runs if the size changes)
		finalImage->Resize(width, height);
	}
	else {
		finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	// Reset image data, accumulation data and image iterators

	delete[] imageData;
	imageData = new uint32_t[width * height];

	delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];

	imageXIterator.resize(width);
	imageYIterator.resize(height);
	for (uint32_t i = 0; i < width; i++) {
		imageXIterator[i] = i;
	}
	for (uint32_t i = 0; i < height; i++) {
		imageYIterator[i] = i;
	}
}

void Renderer::Render(const Scene& scene)
{
	activeScene = &scene;

	// On the first frame of accumulation make the accumulation data default to 0
	if (frameIndex == 1) memset(accumulationData, 0, finalImage->GetWidth() * finalImage->GetHeight() * sizeof(glm::vec4));

	// The outer loop uses y to be more memory friendly (row vs column)
	// std::for_each is used instead of a for loop because it can utilize multithreading
	// The std::execution::par argument runs iterations in parallel
	std::for_each(std::execution::par, imageYIterator.begin(), imageYIterator.end(),
	[this](uint32_t y) { // this needs to be captured so the x iterator is available in the lambda function
		std::for_each(std::execution::par, imageXIterator.begin(), imageXIterator.end(),
		[this, y](uint32_t x) {

			glm::vec4 color = PerPixel(x, y); // Get the color value for this pixel

			accumulationData[x + y * finalImage->GetWidth()] += color; // The color is added to the accumulated data
			glm::vec4 accumulatedColor = accumulationData[x + y * finalImage->GetWidth()] / (float)frameIndex; // The accumulated color is divided by the number of frames past

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f)); // Clamp the color values to between 0 and 1
			imageData[x + y * finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);

		});
	});

	finalImage->SetData(imageData);

	if (settings.Accumulate) frameIndex++;
	else frameIndex = 1;

}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	// To get the color value at this pixel we need to solve where the ray intersects a sphere (if it does)

	// Ray equation: a + bt
	// ------
	// a = ray origin
	// b = ray direction (from 0,0 to coord)
	// t = distance where ray hits sphere
	Ray ray;
	ray.Origin = { 0.0f, 0.0f, 1.0f };
	// x and y coords are divided by the image resolution to map every pixel from 0 to 1
	glm::vec2 coord = { x / (float)finalImage->GetWidth() , y / (float)finalImage->GetHeight() };
	coord = coord * 2.0f - 1.0f; // remap coords to go from -1 to 1, so that the ray origin can sit at 0,0
	ray.Direction = { coord.x, coord.y, -1.0f };

	glm::vec3 light(0.0f); // The color of the light at this pixel
	glm::vec3 contribution(1.0f); // The color of the light of the ray as it bounces around
	glm::vec3 backgroundColor = glm::vec3(0.8f, 0.8f, 0.8f);

	// The ray will bounce off of objects while travelling to the end point (this pixel)
	// The color of every intersected object will contribute to the color of the ray
	int bounces = 5;
	for (int i = 0; i < bounces; i++) {

		Renderer::HitObject hitObj = TraceRay(ray); // Get the ray hit information including t

		// By default t = -1, which means no sphere was intersected
		// Once the ray no longer has objects to bounce off of, its accumulated color can be added to this pixel
		if (hitObj.HitDistance < 0.0f) {
			light += backgroundColor * contribution; // The color of the ray is affected by the background (sky) color
			break;
		}

		// Get the sphere that was intersected by the ray
		const Sphere& hitSphere = activeScene->Spheres[hitObj.ObjectIndex];
		// The color of the sphere is reflected (the ray is multiplied by the sphere's colors)
		// Each bounce will diminish the light of the ray (light is absorbed into the sphere) because color values are <= 1
		contribution *= hitSphere.SphereMaterial.Albedo;

		// If the intersected sphere is emissive, its light will get added to this pixel's light
		light += hitSphere.SphereMaterial.GetEmission();

		ray.Origin = hitObj.HitPosition + hitObj.HitNormal * 0.0001f;
		ray.Direction = glm::normalize(hitObj.HitNormal + Walnut::Random::InUnitSphere()); // Walnut function to generate a random vector in a unit sphere, added to the ray hit normal to get a random bounce
	}

	return glm::vec4(light, 1.0f);
}

Renderer::HitObject Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1; // Only the sphere closest to the camera is used (this var stores its index)
	float hitDistance = FLT_MAX; // The closest distance at which the ray intersects with a sphere

	for (size_t i = 0; i < activeScene->Spheres.size(); i++) { // Checking each sphere in the scene

		const Sphere& sphere = activeScene->Spheres[i];

		glm::vec3 origin = ray.Origin - sphere.Position; // The spheres position is subtracted from the ray origin to give the appearance of moving the sphere

		// Ray sphere intersection equation:
		// (bx^2 + by^2 + bz^2)t^2 + 2(axbx + ayby)t + (ax^2 + ay^2 - r^2) = 0
		// ------
		// r = radius of sphere
		// We need to find a,b,c for the quadratic formula where:
		// a is the coefficient of t^2, b is the coefficient of t, c is the constant
		float a = glm::dot(ray.Direction, ray.Direction); // (bx^2 + by^2 + bz^2) = dot product of ray direction with itself
		float b = 2.0f * glm::dot(origin, ray.Direction); // (axbx + ayby) = dot product of ray direction and ray origin
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius; // (ax^2 + ay^2) = dot product of ray origin with itself

		// Quadratic formula discriminant: b^2 - 4ac
		// ------
		// If the discriminant is > 0 there are 2 solutions, = 0 there is 1 solution, < 0 there are no solutions
		float disc = b * b - 4.0f * a * c;
		if (disc < 0.0f) continue; // If there are no solutions exit the loop and go to the next sphere

		// Full quadratic formula:
		// ( -b +- sqrt(discriminant) ) / 2a = t
		// ------
		// t is the distance at which the ray intersects with the sphere
		// The smaller t is where the ray enters the sphere, the bigger t is where the ray exits the sphere
		// t1 is always smaller assuming a is positive (which it must be because its a sum of squares), t0 never needs to be used
		// float t0 = (-b + glm::sqrt(disc)) / (2.0f * a);
		float t1 = (-b - glm::sqrt(disc)) / (2.0f * a);

		if (t1 > 0.0f && t1 < hitDistance) {
			hitDistance = t1;
			closestSphere = (int)i;
		}

	}

	// No sphere was intersected, return background color
	if (closestSphere == -1) return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitObject Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	// Create a hit object which stores the position and normal of where the ray intersects with a sphere, and the intersected sphere's index
	Renderer::HitObject hitObj;
	hitObj.HitDistance = hitDistance;
	hitObj.ObjectIndex = objectIndex;

	const Sphere& closestSphere = activeScene->Spheres[objectIndex];

	// Finding the coordinate of the ray sphere intersection point
	glm::vec3 origin = ray.Origin - closestSphere.Position;
	hitObj.HitPosition = origin + ray.Direction * hitDistance;
	// The normal vector is perpindicular to the surface where the ray intersects the sphere
	hitObj.HitNormal = glm::normalize(hitObj.HitPosition); // Normals can go from -1 to 1

	// The sphere's position is subtracted from the ray's position to make it appear as if the sphere has a different position in the image
	// The sphere's position needs to be added back to get the true position of the hit
	hitObj.HitPosition += closestSphere.Position; 

	return hitObj;
}

Renderer::HitObject Renderer::Miss(const Ray& ray)
{
	// If the ray does not intersect with a sphere return a hit object with hit distance = -1
	Renderer::HitObject missObj;
	missObj.HitDistance = -1.0f;
	return missObj;
}
