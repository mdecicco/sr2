// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in mat4 transform;

out vec3 o_norm;
out vec3 o_color;
out float o_depth;

layout (std140) uniform u_material { vec3 color; } material;
layout (std140) uniform u_model { mat4 transform; } model;
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

void main() {
	gl_Position = scene.view_proj * transform * vec4(vpos, 1.0);
	o_norm = (inverse(transpose(transform)) * vec4(vnorm, 1.0)).xyz;
	o_color = normalize(vpos);
	o_depth = -(scene.transform * transform * vec4(vpos, 1.0)).z;
};

// fragment
#version 330

in vec3 o_color;
in vec3 o_norm;
in float o_depth;

out vec4 frag_color;

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

layout (std140) uniform u_fog {
	float start;
	float end;
	float clamp;
	vec3 color;
} fog;

vec4 calcFog(vec4 inColor) {
	float fac = clamp((o_depth - fog.start) / (fog.end - fog.start), 0.0, fog.clamp);
	return vec4((inColor.rgb * (1.0 - fac)) + (fog.color * fac), inColor.a);
}

void main() {
	vec4 color = vec4(max(dot(o_norm, normalize(vec3(1, 1, 1))), 0.1) * o_color, 1);
	frag_color = calcFog(color);
}
