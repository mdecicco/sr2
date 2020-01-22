// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in vec2 vtex;
layout (location = 3) in mat4 transform;

out vec3 o_pos;
out vec3 o_norm;
out vec2 o_tex;
out vec2 o_ttex;
out float o_depth;

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

uniform vec2 terrainSize;

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	gl_Position = scene.view_proj * transform * vec4(vpos, 1.0);
	vec3 tpos = vec3(transform * vec4(vpos, 1.0));
	o_pos = tpos;
	o_norm = vec3(inverse(transpose(transform)) * vec4(vnorm, 1.0));
	o_tex = vtex;
	o_ttex = vec2((tpos.x + 4.75752497) / (terrainSize.x * 5.0f), (tpos.z + 5.14905024) / (terrainSize.y * 5.0f));
	o_depth = -(scene.transform * transform * vec4(vpos, 1.0)).z;
};

// fragment
#version 330

in vec3 o_pos;
in vec3 o_norm;
in vec2 o_tex;
in vec2 o_ttex;
in float o_depth;

uniform sampler2D tex;
uniform sampler2D lightMap;
uniform float ambientMult;
uniform vec2 terrainSize;

layout (std140) uniform u_material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emissive;
	float tex_factor;
	float highlight;
} material;

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

layout(location = 0) out vec4 frag_color;
layout(location = 1) out ivec4 frag_surfaceInfo;

void main() {
	float bakedLightFactor = texture(lightMap, o_ttex).r;
	float nl = length(o_norm);
	vec4 color = vec4(0, 0, 0, 1);
	if (nl == 0) {
		// If there is no texture, tex_factor will be 0, otherwise it will be 1. Textureless
		// objects will only use the material colors and not be affected by the result of
		// texture(tex, o_tex)
		
		vec4 tex_diffuse = material.diffuse * texture(tex, o_tex) * material.tex_factor;
		vec4 mat_diffuse = material.diffuse * (1.0 - material.tex_factor);
		color = tex_diffuse + mat_diffuse;
		color.rgb *= bakedLightFactor;
	} else {
		// If there is no texture, tex_factor will be 0, otherwise it will be 1. Textureless
		// objects will only use the material colors and not be affected by the result of
		// texture(tex, o_tex)
		vec3 normal = normalize(o_norm);
		float ndotl = clamp(dot(normal, vec3(1, 1, 0)), 0.0, 1.0);
		vec4 tex_diffuse = material.diffuse * texture(tex, o_tex) * material.tex_factor;
		vec4 mat_diffuse = material.diffuse * (1.0 - material.tex_factor);
		vec4 diffuse = tex_diffuse + mat_diffuse;
		vec3 ambient = vec3(material.ambient * (1.0 - ndotl) * ambientMult) * vec3(diffuse);
		color = vec4(ambient, 0.0) + vec4(vec3(diffuse) * ndotl, diffuse.a);
	}
	if (color.a < 0.5) discard;
	color.rgb *= (material.highlight + 1.0);
	frag_color = calcFog(color);
	frag_surfaceInfo = ivec4(1, int(o_ttex.x * terrainSize.x), int(o_ttex.y * terrainSize.y), 0);
}
