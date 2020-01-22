// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in vec2 vtex;

out vec2 o_tex;

layout (std140) uniform u_scene {
	mat4 transform;
	mat4 projection;
	mat4 view_proj;
	mat4 invView;
	vec3 camera_pos;
	vec3 camera_left;
	vec3 camera_up;
	vec3 camera_forward;
	float camera_near;
	float camera_far;
	float camera_fov;
} scene;

layout (std140) uniform u_model { mat4 transform; } model;

void main() {
	gl_Position = scene.view_proj * model.transform * vec4(vpos, 1.0);
	o_tex = vtex;
};

// fragment
#version 330

in vec2 o_tex;

uniform sampler2D tex;

layout (std140) uniform u_material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emissive;
	float tex_factor;
} material;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out ivec4 frag_surfaceInfo;

void main() {
	frag_color = material.diffuse * texture(tex, o_tex) * material.tex_factor;
	frag_surfaceInfo = ivec4(0, 0, 0, 0);
}
