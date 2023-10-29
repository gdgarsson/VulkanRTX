#pragma once
#include "PrxDevice.hpp"
#include "PrxSwapChain.hpp"
#include "PrxTexture.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

// std
#include <string>
#include <vector>
#include <memory>

namespace prx {

	class PrxMaterial
	{
	public:
		PrxMaterial(PrxDevice& device, const std::string& mat_name,
			glm::vec3 mat_diffuse = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 mat_specular = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 mat_ambient = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 mat_emission = glm::vec3(1, 1, 1),
			glm::vec3 mat_transmission = glm::vec3(0.0f, 0.0f, 0.0f),
			float mat_ior = 1.0f, float mat_shininess = 1.0f, float mat_opacity = 1.0f);
		~PrxMaterial();

		// delete the copy constructors
		PrxMaterial(const PrxMaterial&) = delete;
		PrxMaterial& operator=(const PrxMaterial&) = delete;
		PrxMaterial(PrxMaterial&&) = delete;
		PrxMaterial& operator=(PrxMaterial&&) = delete;

		glm::vec3 getDiffuse() { return diffuse; }

		bool hasTexture = false;
		std::string name;

		void makeDiffuseTexture(const std::string& tex_filepath = "");

		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 ambient;
		glm::vec3 emission;
		glm::vec3 transmittance;
		float ior; // index of refraction
		float shininess;
		float opacity; // how opaque is this material

		std::unique_ptr<PrxTexture> diffuseTex;

	private:
		PrxDevice& prxDevice;

		std::string diffuseTexFilePath;
		//std::string specularTexFilePath;
		//std::string ambientTexFilePath;
		//std::string normalTexFilePath;
		

	};
}