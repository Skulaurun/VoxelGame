#pragma once

#include <Engine/Core/Core.hpp>

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

enum class ShaderType : uint8_t {
	Vertex, Fragment, Compute
};

struct ShaderSource {
	ShaderType type;
	std::string_view text;
};

class Shader {

public:
	Shader(const std::vector<ShaderSource>& sources);
	~Shader();

	void bind() const;
	void unbind() const;

	void setUniform(std::string_view name, int value);
	void setUniform(std::string_view name, float value);

	void setUniform(std::string_view name, const glm::vec2& value);
	void setUniform(std::string_view name, const glm::vec3& value);
	void setUniform(std::string_view name, const glm::vec4& value);
		
	void setUniform(std::string_view name, const glm::mat2& value);
	void setUniform(std::string_view name, const glm::mat3& value);
	void setUniform(std::string_view name, const glm::mat4& value);

private:
	int32_t getUniformLocation(std::string_view name);
	bool compileShader(uint32_t shader) const;

private:
	int32_t id;
	std::unordered_map<std::string, int32_t, StringHasher, std::equal_to<>> uniformLocationCache;

};
