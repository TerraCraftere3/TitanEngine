#pragma once

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImReflect_helper.hpp"
#include "ImReflect_entry.hpp"

namespace ImReflect {

// Type settings for glm::vec2
template<>
struct type_settings<glm::vec2> : ImRequired<glm::vec2> {
private:
	bool _as_color = false;
	bool _as_position = false;
public:
	type_settings<glm::vec2>& as_color(const bool v = true) { 
		_as_color = v; 
		if (v) _as_position = false;
		RETURN_THIS_T(glm::vec2); 
	}
	type_settings<glm::vec2>& as_position(const bool v = true) { 
		_as_position = v; 
		if (v) _as_color = false;
		RETURN_THIS_T(glm::vec2); 
	}
	bool is_color() const { return _as_color; }
	bool is_position() const { return _as_position; }
};

// Type settings for glm::vec3
template<>
struct type_settings<glm::vec3> : ImRequired<glm::vec3> {
private:
	bool _as_color = false;
	bool _as_position = false;
public:
	type_settings<glm::vec3>& as_color(const bool v = true) { 
		_as_color = v; 
		if (v) _as_position = false;
		RETURN_THIS_T(glm::vec3); 
	}
	type_settings<glm::vec3>& as_position(const bool v = true) { 
		_as_position = v; 
		if (v) _as_color = false;
		RETURN_THIS_T(glm::vec3); 
	}
	bool is_color() const { return _as_color; }
	bool is_position() const { return _as_position; }
};

// Type settings for glm::vec4
template<>
struct type_settings<glm::vec4> : ImRequired<glm::vec4> {
private:
	bool _as_color = false;
	bool _as_position = false;
public:
	type_settings<glm::vec4>& as_color(const bool v = true) { 
		_as_color = v; 
		if (v) _as_position = false;
		RETURN_THIS_T(glm::vec4); 
	}
	type_settings<glm::vec4>& as_position(const bool v = true) { 
		_as_position = v; 
		if (v) _as_color = false;
		RETURN_THIS_T(glm::vec4); 
	}
	bool is_color() const { return _as_color; }
	bool is_position() const { return _as_position; }
};

} // namespace ImReflect

// Tag invoke implementation for glm::vec2
inline void tag_invoke(ImReflect::ImInput_t, const char* label, glm::vec2& value, ImSettings& settings, ImResponse& response) {
	auto& vec2_settings = settings.get<glm::vec2>();
	auto& vec2_response = response.get<glm::vec2>();

	bool changed = false;
	if (vec2_settings.is_color()) {
		// vec2 as color - no ColorEdit2 exists, so just use DragFloat2 with 0-1 range
		changed = ImGui::DragFloat2(label, glm::value_ptr(value), 0.01f, 0.0f, 1.0f);
	} else if (vec2_settings.is_position()) {
		changed = ImGui::DragFloat2(label, glm::value_ptr(value));
	} else {
		// Default behavior - drag
		changed = ImGui::DragFloat2(label, glm::value_ptr(value));
	}
	
	if (changed) vec2_response.changed();
	ImReflect::Detail::check_input_states(vec2_response);
}

// Tag invoke implementation for glm::vec3
inline void tag_invoke(ImReflect::ImInput_t, const char* label, glm::vec3& value, ImSettings& settings, ImResponse& response) {
	auto& vec3_settings = settings.get<glm::vec3>();
	auto& vec3_response = response.get<glm::vec3>();

	bool changed = false;
	if (vec3_settings.is_color()) {
		changed = ImGui::ColorEdit3(label, glm::value_ptr(value));
	} else if (vec3_settings.is_position()) {
		changed = ImGui::DragFloat3(label, glm::value_ptr(value));
	} else {
		// Default behavior - drag
		changed = ImGui::DragFloat3(label, glm::value_ptr(value));
	}
	
	if (changed) vec3_response.changed();
	ImReflect::Detail::check_input_states(vec3_response);
}

// Tag invoke implementation for glm::vec4
inline void tag_invoke(ImReflect::ImInput_t, const char* label, glm::vec4& value, ImSettings& settings, ImResponse& response) {
	auto& vec4_settings = settings.get<glm::vec4>();
	auto& vec4_response = response.get<glm::vec4>();

	bool changed = false;
	if (vec4_settings.is_color()) {
		changed = ImGui::ColorEdit4(label, glm::value_ptr(value));
	} else if (vec4_settings.is_position()) {
		changed = ImGui::DragFloat4(label, glm::value_ptr(value));
	} else {
		// Default behavior - drag
		changed = ImGui::DragFloat4(label, glm::value_ptr(value));
	}
	
	if (changed) vec4_response.changed();
	ImReflect::Detail::check_input_states(vec4_response);
}
