// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec2 vtex0;
layout (location = 2) in vec2 vtex1;

out vec2 o_tex0;
out vec2 o_tex1;
flat out uint o_texId;
flat out uint o_texOrientation;
out vec3 o_color;

layout (std140) uniform u_material { vec3 color; } material;
layout (std140) uniform u_scene { mat4 transform; mat4 projection; mat4 view_proj; } scene;
layout (std140) uniform u_model { mat4 transform; } model;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    gl_Position = scene.view_proj * model.transform * vec4(vpos, 1.0);
    o_tex0 = vtex0;
    o_tex1 = vtex1;
    o_color = hsv2rgb(vec3(vpos.y / 50.0, 1.0, 1.0));
};

// fragment
#version 330

in vec3 o_norm;
in vec2 o_tex0;
in vec2 o_tex1;
in vec3 o_color;

uniform sampler2D textures[70];
uniform sampler2D textureIds;
uniform sampler2D pathTextureIds;
uniform usampler2D pathInfo;
uniform sampler2D textureOrientations;
uniform sampler2D lightMap;
uniform ivec2 hoverCoords;
uniform vec2 terrainSize;
uniform ivec2 infoTexCoordIndices;
uniform int tl;
uniform int tr;
uniform int bl;
uniform int br;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out ivec4 frag_surfaceInfo;

vec2 texrot(vec2 tc, float angle) {
    float sf = sin(angle);
    float cf = cos(angle);
    return (vec2(tc.x - 0.5, tc.y - 0.5) * mat2(cf, sf, -sf, cf)) + vec2(0.5, 0.5);
}

void main() {
    int texId = int(texture(textureIds, o_tex0).r * 255.0);
    int pathTexId = int(texture(pathTextureIds, o_tex0).r * 255.0);
    float texOrientation = texture(textureOrientations, o_tex0).r * 255.0;
    float lightFactor = texture(lightMap, o_tex0).r;
    
    vec2 cellTexCoords = vec2(mod(o_tex1.x, 1.0), mod(o_tex1.y, 1.0));
    vec2 diffuseTexCoords = texrot(cellTexCoords, 1.57079632679 * texOrientation);
    vec4 diffuse = texture(textures[texId], diffuseTexCoords);
	//diffuse.rgb = vec3(lightFactor, lightFactor, lightFactor);
    
    ivec2 tc = ivec2(int(o_tex0.x * terrainSize.x), int(o_tex0.y * terrainSize.y));
    if (pathTexId != 255) {
        uvec4 pathInfo = texture(pathInfo, o_tex0);
        vec4 info[4];
        info[0] = vec4(
            (pathInfo.x >>  0) & uint(0xFF),
            (pathInfo.x >>  8) & uint(0xFF),
            (pathInfo.x >> 16) & uint(0xFF),
            (pathInfo.x >> 24) & uint(0xFF)
        ) * 0.00392156862;
        info[1] = vec4(
            (pathInfo.y >>  0) & uint(0xFF),
            (pathInfo.y >>  8) & uint(0xFF),
            (pathInfo.y >> 16) & uint(0xFF),
            (pathInfo.y >> 24) & uint(0xFF)
        ) * 0.00392156862;
        info[2] = vec4(
            (pathInfo.z >>  0) & uint(0xFF),
            (pathInfo.z >>  8) & uint(0xFF),
            (pathInfo.z >> 16) & uint(0xFF),
            (pathInfo.z >> 24) & uint(0xFF)
        ) * 0.00392156862;
        info[3] = vec4(
            (pathInfo.w >>  0) & uint(0xFF),
            (pathInfo.w >>  8) & uint(0xFF),
            (pathInfo.w >> 16) & uint(0xFF),
            (pathInfo.w >> 24) & uint(0xFF)
        ) * 0.00392156862;
        
        vec4 t0 = mix(info[tl], info[tr], cellTexCoords.x);
        vec4 t1 = mix(info[bl], info[br], cellTexCoords.x);
        vec4 interpolatedInfo = mix(t0, t1, cellTexCoords.y);
        
        float tx = interpolatedInfo.x;
        float ty = interpolatedInfo.x;
        if (infoTexCoordIndices.x == 1) tx = interpolatedInfo.y;
        if (infoTexCoordIndices.x == 2) tx = interpolatedInfo.z;
        if (infoTexCoordIndices.x == 3) tx = interpolatedInfo.w;
        if (infoTexCoordIndices.y == 1) ty = interpolatedInfo.y;
        if (infoTexCoordIndices.y == 2) ty = interpolatedInfo.z;
        if (infoTexCoordIndices.y == 3) ty = interpolatedInfo.w;
        
        vec4 pathDiffuse = texture(textures[pathTexId], texrot(cellTexCoords, info[tl].w * 6.28318530718));
        diffuse.rgb = mix(diffuse.rgb, pathDiffuse.rgb, pathDiffuse.a);
        
        if (tc.x == hoverCoords.x && tc.y == hoverCoords.y) diffuse.rgb *= vec3(1.25, 1.25, 1.25);
    }
    
    diffuse.rgb *= lightFactor;
    frag_color = diffuse;
	frag_surfaceInfo = ivec4(2, tc, 0);
}
