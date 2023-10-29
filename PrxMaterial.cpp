#include "PrxMaterial.hpp"

prx::PrxMaterial::PrxMaterial(PrxDevice& device, const std::string& mat_name,
	glm::vec3 mat_diffuse = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 mat_specular = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 mat_ambient = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 mat_emission = glm::vec3(1, 1, 1),
	glm::vec3 mat_transmission = glm::vec3(0.0f, 0.0f, 0.0f),
	float mat_ior = 1.0f, float mat_shininess = 1.0f, float mat_opacity = 1.0f) : prxDevice{device}
{
	name = mat_name;
	diffuse = mat_diffuse;
	specular = mat_specular;
	ambient = mat_ambient;
	emission = mat_emission;
	transmittance = mat_transmission;
	ior = mat_ior;
	shininess = mat_shininess;
	opacity = mat_opacity;

}

prx::PrxMaterial::~PrxMaterial()
{
	if (hasTexture) {
		diffuseTex.reset();
	}
}


void prx::PrxMaterial::makeDiffuseTexture(const std::string& tex_filepath = "") {
	diffuseTex = std::make_unique<PrxTexture>(prxDevice, tex_filepath);
	hasTexture = true;
}
