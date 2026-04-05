#include <Engine/Renderer/OpenGL/Shader.hpp>
#include <Engine/Core/Log.hpp>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

static GLenum getEnum(const ShaderType& type) {
	switch (type) {
		case ShaderType::Vertex: return GL_VERTEX_SHADER;
		case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
		default: return GL_NONE;
	}
}

Shader::Shader(const std::vector<ShaderSource>& sources) {
	id = glCreateProgram();

	std::vector<uint32_t> shaders = {};
	shaders.reserve(sources.size());
	for (const auto& source : sources) {
		uint32_t shader = glCreateShader(getEnum(source.type));
		shaders.emplace_back(shader);

		const char* data = source.text.data();
		glShaderSource(shader, 1, &data, NULL);

		compileShader(shader);
		glAttachShader(id, shader);
	}

	glLinkProgram(id);

	int32_t result;
	glGetProgramiv(id, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		LOG_CORE_ERROR("Failed to link Shader program");
	}

	for (const auto& shader : shaders) glDeleteShader(shader);
}
Shader::~Shader() {
	glDeleteProgram(id);
}

void Shader::compileShader(const uint32_t shader) const {
	glCompileShader(shader);

	int32_t result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int32_t length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		std::string info(static_cast<std::string::size_type>(length - 1), '\0');
		glGetShaderInfoLog(shader, length, NULL, info.data());

		LOG_CORE_ERROR(info);
	}
}
int32_t Shader::getUniformLocation(std::string_view name) {
	const auto& iterator = uniformLocationCache.find(name);
	if (iterator != uniformLocationCache.end()) {
		return iterator->second;
	}

	int32_t location = glGetUniformLocation(id, name.data());
	uniformLocationCache.emplace(name, location);
	return location;
}

void Shader::bind() const {
	glUseProgram(id);
}
void Shader::unbind() const {
	glUseProgram(0);
}

void Shader::setUniform(std::string_view name, const int value) {
	glUniform1i(getUniformLocation(name), value);
}
void Shader::setUniform(std::string_view name, const float value) {
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform(std::string_view name, const glm::vec2& value) {
	glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, const glm::vec3& value) {
	glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, const glm::vec4& value) {
	glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setUniform(std::string_view name, const glm::mat2& value) {
	glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, const glm::mat3& value) {
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, const glm::mat4& value) {
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
