#pragma once
#include <r2/engine.h>

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define SET_BIT(var, pos, x) var ^= (-x ^ var) & (1UL << pos)
#define FLAG_TEX_COORDS		1
#define FLAG_NORMALS		4
#define FLAG_POLYGON_FACES	6
#define FLAG_POSITION		8

#define SEC_FLAG_SMALL_VERTEX 0

using namespace std;
using namespace r2;

struct vertex {
	vec3f pos;
	vec3f normal;
	vec2f texcoord;

	void print(u8 t = 0) {
		if (t == 0) printf("vertex %f %f %f %f %f %f %f %f\n", pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, texcoord.x, texcoord.y);
		else if (t == 1) printf("v %f %f %f\n", pos.x, pos.y, pos.z);
		else if (t == 2) printf("vn %f %f %f\n", normal.x, normal.y, normal.z);
		else if (t == 3) printf("vt %f %f\n", texcoord.x, texcoord.y);
	}
};

struct unprocessed_index {
	u16 index;
	bool close;
	bool unknown;
};

struct section_strip {
	void print(const string& mesh_name, u32 section, u32 idx) {
		//printf("o %s_section%d_strip%d\n", mesh_name.c_str(), section, idx);
		for (u32 i = 0; i < vertices.size(); i++) vertices[i].print(1);
		for (u32 i = 0; i < vertices.size(); i++) vertices[i].print(2);
		for (u32 i = 0; i < vertices.size(); i++) vertices[i].print(3);
		for (u32 i = 0; i < indices.size() - 3; i += 3) {
			u16 i0 = indices[i + 0] + 1;
			u16 i1 = indices[i + 1] + 1;
			u16 i2 = indices[i + 2] + 1;
			//printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", i0, i0, i0, i1, i1, i1, i2, i2, i2);
		}
	}
	u16 verticesPerFace;
	vector<vertex> vertices;
	vector<u16> indices;
};

struct mesh_section {
	void print(const string& mesh_name, u32 idx) {
		printf("section %d\n", idx);
		printf("shader %d\n", shaderIdx);
		printf("unknown %d\n", unknown);
		for (u32 i = 0; i < strips.size(); i++) {
			strips[i].print(mesh_name, idx, i);
		}
	}
	section_strip& new_strip() { strips.push_back(section_strip()); return current_strip(); };
	section_strip& current_strip() { return strips[strips.size() - 1]; }
	vector<section_strip> strips;
	u16 shaderIdx;
	u16 unknown;
};

struct pkg_mesh {
	string name;

	void print() {
		for (u32 i = 0; i < sections.size(); i++) {
			sections[i].print(name, i);
		}
	}

	mesh_section& new_section() { sections.push_back(mesh_section()); return current_section(); };
	mesh_section& current_section() { return sections[sections.size() - 1]; }
	vector<mesh_section> sections;
};

struct color4b {
	u8 r, g, b, a;
};

struct shader {
	string texture;
	color4b unk[4];
};

class pkg {
	public:
		pkg() { };
		~pkg() { };

		void print() {
			for (u32 i = 0; i < meshes.size() && i < 1; i++) {
				meshes[i].print();
			}
		}

		pkg_mesh& new_mesh() { meshes.push_back(pkg_mesh()); return current_mesh(); };
		pkg_mesh& current_mesh() { return meshes[meshes.size() - 1]; }
		vector<pkg_mesh> meshes;
		vector<shader> shaders;
};

float h2f(u16 value) {
	float out;
	unsigned int abs = value & 0x7FFF;
	if (abs > 0x7C00)
		out = (std::numeric_limits<float>::has_signaling_NaN && !(abs & 0x200)) ? std::numeric_limits<float>::signaling_NaN() :
		std::numeric_limits<float>::has_quiet_NaN ? std::numeric_limits<float>::quiet_NaN() : 0.0f;
	else if (abs == 0x7C00)
		out = std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max();
	else if (abs > 0x3FF)
		out = std::ldexp(static_cast<float>((abs & 0x3FF) | 0x400), (abs >> 10) - 25);
	else
		out = std::ldexp(static_cast<float>(abs), -24);
	return (value & 0x8000) ? -out : out;
}

f32 readAngle(u8 angle) {
	return ((f32)angle / 255.0f) * 2.0f * 3.141592653f;
}

bool read_uniform_vertex(data_container* r, bool* flags, bool* sectionFlags, section_strip& strip) {
	vec3f pos = vec3f(), normal = vec3f();
	vec2f tex = vec2f();

	if (flags[FLAG_POSITION]) {
		if (sectionFlags[SEC_FLAG_SMALL_VERTEX]) {
			u16 x, y, z;
			r->read(x);
			r->read(y);
			r->read(z);
			pos = vec3f(h2f(x), h2f(y), h2f(z));
		} else {
			r->read(pos);
		}
		////printf("Vertex position (%0.4f, %0.4f, %0.4f)\n", pos.x, pos.y, pos.z);
	}
	if (flags[FLAG_NORMALS]) {
		// This probably isn't the right way to do this
		u8 nx = 0; r->read(nx);
		u8 ny = 0; r->read(ny);
		u8 nz = 0; r->read(nz);
		normal.x = ((f32(nx) / 255.0f) * 2.0f) - 1.0f;
		normal.y = ((f32(ny) / 255.0f) * 2.0f) - 1.0f;
		normal.z = ((f32(nz) / 255.0f) * 2.0f) - 1.0f;
		////printf("\t\t\t\tVertex normal (%0.4f, %0.4f, %0.4f)\n", normal.x, normal.y, normal.z);
	}
	if (flags[FLAG_TEX_COORDS]) {
		u16 tx = 0; r->read(tx);
		u16 ty = 0; r->read(ty);
		tex.x = (h2f(tx) - 2.0f) / 0.25f;
		tex.y = 1.0f - ((h2f(ty) - 2.0f) / 0.25f);
		////printf("\t\t\t\tVertex texcoord (%0.4f, %0.4f)\n", tex.x, tex.y);
	}

	strip.vertices.push_back({ pos, normal, tex });

	return true;
}

bool read_uniform_strip(data_container* r, bool* flags, bool* sectionFlags, section_strip& strip) {
	r->read(strip.verticesPerFace);
	u16 vertex_count = 0;
	r->read(vertex_count);

	//printf("\t\t\tBegin Uniform Strip: (Vertices per face: %d, Vertex count: %d)\n", strip.verticesPerFace, vertex_count);

	for (u16 i = 0; i < vertex_count; i++) {
		if (!read_uniform_vertex(r, flags, sectionFlags, strip)) return false;
	}

	u16 index_count = 0;
	r->read(index_count);
	data_container* rawindices = r->sub(index_count * 2);
	vector<unprocessed_index> indices;
	for (u16 i = 0; i < index_count; i++) {
		u16 index = 0;
		rawindices->read(index);
		bool closePolygon = CHECK_BIT(index, 15);
		if (closePolygon) {
			// TODO: close polygon logic
			SET_BIT(index, 15, 0);
		}
		bool unknownFlag = CHECK_BIT(index, 14);
		if (unknownFlag) {
			// TODO: Figure this out (maybe means that the face is flipped and should not be culled?)
			SET_BIT(index, 14, 0);
		}
		////printf("Index: %d %s %s\n", index, closePolygon ? "(close polygon flag)" : "", unknownFlag ? "(unknown flag)" : "");
		indices.push_back({ index, closePolygon, unknownFlag });
		if (index >= vertex_count) {
			printf("Bad index!\n");
		}
	}
	r2engine::files()->destroy(rawindices);

	i32 i0 = -1;
	i32 i1 = -1;
	i32 i2 = -1;
	for (u16 i = 0; i < indices.size(); i++) {
		if (i0 == -1) i0 = indices[i].index;
		else if (i1 == -1) i1 = indices[i].index;
		else if (i2 == -1) {
			i2 = indices[i].index;
			strip.indices.push_back(i0);
			strip.indices.push_back(i1);
			strip.indices.push_back(i2);
		}
		else {
			i0 = i1;
			i1 = i2;
			i2 = indices[i].index;
			strip.indices.push_back(i0);
			strip.indices.push_back(i1);
			strip.indices.push_back(i2);
		}

		if (indices[i].close) {
			i0 = i1 = i2 = -1;
		}

		if (indices[i].unknown) {
		}
	}

	//printf("\t\t\tEnd Uniform Strip: (Index count: %d, %d bytes)\n", index_count, index_count * 2);

	return true;
}

bool read_uniform_section(data_container* r, bool* flags, mesh_section& section) {
	u16 strip_count = 0;
	r->read(strip_count);
	r->read(section.shaderIdx);
	r->read(section.unknown);

	bool sectionFlags[16];
	//printf("shaderidx flags?: ");
	for (u8 i = 0; i < 16; i++) {
		sectionFlags[i] = CHECK_BIT(section.shaderIdx, i);
		//printf("%d", sectionFlags[i] ? 1 : 0);
	}
	//printf("\n");
	if (sectionFlags[0]) SET_BIT(section.shaderIdx, 0, 0);


	//printf("\t\tUniform Section: (Strip count: %d, Shader offset: %d, Unknown: %d)\n", strip_count, section.shaderIdx, section.unknown);

	for (u32 i = 0; i < strip_count; i++) {
		if (!read_uniform_strip(r, flags, sectionFlags, section.new_strip())) return false;
	}

	//printf("\t\tEnd Uniform Section: (Strip count: %d, Shader offset: %d, Unknown: %d)\n", strip_count, section.shaderIdx, section.unknown);

	return true;
}

bool read_polygon_vertex(data_container* r, bool* flags, bool* sectionFlags, section_strip& strip) {
	vec3f pos = vec3f(), normal = vec3f();
	vec2f tex = vec2f();

	if (flags[FLAG_POSITION]) {
		if (sectionFlags[SEC_FLAG_SMALL_VERTEX]) {
			u16 x, y, z;
			r->read(x);
			r->read(y);
			r->read(z);
			pos = vec3f(h2f(x), h2f(y), h2f(z));
		} else {
			r->read(pos);
		}
		////printf("\t\t\t\tVertex position (%0.4f, %0.4f, %0.4f)\n", pos.x, pos.y, pos.z);
	}
	if (flags[FLAG_NORMALS]) {
		// This probably isn't the right way to do this
		u8 nx = 0; r->read(nx);
		u8 ny = 0; r->read(ny);
		u8 nz = 0; r->read(nz);
		normal.x = ((f32(nx) / 255.0f) * 2.0f) - 1.0f;
		normal.y = ((f32(ny) / 255.0f) * 2.0f) - 1.0f;
		normal.z = ((f32(nz) / 255.0f) * 2.0f) - 1.0f;
		////printf("\t\t\t\tVertex normal (%d, %d, %d)\n", x, y, z);
	}
	if (flags[FLAG_TEX_COORDS]) {
		u8 unk[4];
		r->read(unk);
		u16 tx = 0; r->read(tx);
		u16 ty = 0; r->read(ty);
		tex.x = (h2f(tx) - 2.0f) / 0.25f;
		tex.y = 1.0f - ((h2f(ty) - 2.0f) / 0.25f);
		//r->read(tex);
		////printf("\t\t\t\tVertex texcoord (%0.4f, %0.4f)\n", tex.x, tex.y);
	}

	strip.vertices.push_back({ pos, normal, tex });
	return true;
}

bool read_polygon_strip(data_container* r, bool* flags, bool* sectionFlags, section_strip& strip) {
	u16 vertices_per_face = 0;
	r->read(vertices_per_face);
	u16 vertex_count = 0;
	r->read(vertex_count);

	//printf("\t\t\tBegin Polygon Strip: (Vertices per face: %d, Vertex count: %d)\n", vertices_per_face, vertex_count);

	for (u16 i = 0; i < vertex_count; i++) {
		if (!read_polygon_vertex(r, flags, sectionFlags, strip)) return false;
	}

	u16 index_count = 0;
	r->read(index_count);
	data_container* rawindices = r->sub(index_count * 2);
	vector<unprocessed_index> indices;
	for (u16 i = 0; i < index_count; i++) {
		u16 index = 0;
		rawindices->read(index);
		bool closePolygon = CHECK_BIT(index, 15);
		if (closePolygon) {
			// TODO: close polygon logic
			SET_BIT(index, 15, 0);
		}
		bool unknownFlag = CHECK_BIT(index, 14);
		if (unknownFlag) {
			// TODO: Figure this out (maybe means that the face is flipped and should not be culled?)
			SET_BIT(index, 14, 0);
		}
		////printf("Index: %d %s %s\n", index, closePolygon ? "(close polygon flag)" : "", unknownFlag ? "(unknown flag)" : "");
		indices.push_back({ index, closePolygon, unknownFlag });
		if (index >= vertex_count) {
			printf("Bad index!\n");
		}
	}

	r2engine::files()->destroy(rawindices);

	i32 i0 = -1;
	i32 i1 = -1;
	i32 i2 = -1;
	for (u16 i = 0; i < indices.size(); i++) {
		if (i0 == -1) i0 = indices[i].index;
		else if (i1 == -1) i1 = indices[i].index;
		else if (i2 == -1) {
			i2 = indices[i].index;
			strip.indices.push_back(i0);
			strip.indices.push_back(i1);
			strip.indices.push_back(i2);
		}
		else {
			i0 = i1;
			i1 = i2;
			i2 = indices[i].index;
			strip.indices.push_back(i0);
			strip.indices.push_back(i1);
			strip.indices.push_back(i2);
		}

		if (indices[i].close) {
			i0 = i1 = i2 = -1;
		}

		if (indices[i].unknown) {
		}
	}
	return true;
}

bool read_polygon_section(data_container* r, bool* flags, mesh_section& section) {
	u16 strip_count = 0;
	r->read(strip_count);
	r->read(section.shaderIdx);
	r->read(section.unknown);

	bool sectionFlags[16];
	//printf("shaderidx flags?: ");
	for (u8 i = 0; i < 16; i++) {
		sectionFlags[i] = CHECK_BIT(section.shaderIdx, i);
		//printf("%d", sectionFlags[i] ? 1 : 0);
	}
	//printf("\n");
	if (sectionFlags[0]) SET_BIT(section.shaderIdx, 0, 0);

	//printf("\t\tPolygon Section: (Strip count: %d, unknown: %d, Shader offset: %d)\n", (u32)strip_count, (u32)section.unknown, (u32)section.shaderIdx);

	for (u16 i = 0; i < strip_count; i++) {
		if (!read_polygon_strip(r, flags, sectionFlags, section.new_strip())) return false;
	}

	//printf("\t\tEnd Polygon Section: (Strip count: %d, unknown: %d, Shader offset: %d)\n", (u32)strip_count, (u32)section.unknown, (u32)section.shaderIdx);

	return true;
}

bool read_geometry(data_container* r, pkg_mesh& mesh) {
	u32 section_count = 0;
	r->read(section_count);
	u32 vertex_count = 0;
	r->read(vertex_count);
	u32 index_count = 0;
	r->read(index_count);
	u32 section_count_2 = 0;
	r->read(section_count_2);
	u32 iflags = 0;
	r->read(iflags);
	bool flags[32];
	//printf("\tBegin Geometry: (Sections: %d, Vertices: %d, Indices: %d, SC2: %d, Flags: ", section_count, vertex_count, index_count, section_count_2);
	for (u8 i = 0; i < 32; i++) {
		flags[i] = CHECK_BIT(iflags, i);
		//printf("%d", flags[i] ? 1 : 0);
	}
	//printf("\n");

	for (u32 i = 0; i < section_count; i++) {
		if (flags[FLAG_POLYGON_FACES] && !read_polygon_section(r, flags, mesh.new_section())) return false;
		if (!flags[FLAG_POLYGON_FACES] && !read_uniform_section(r, flags, mesh.new_section())) return false;
	}

	//printf("\nEnd Geometry: (Sections: %d, Vertices: %d, Indices: %d, SC2: %d, Flags: ", section_count, vertex_count, index_count, section_count_2);
	//for (u8 i = 0; i < 32; i++) //printf("%d", flags[i] ? 1 : 0);
	//printf("\n");
}

bool read_shaders(data_container* r, pkg& pkgfile) {
	u32 type = 0;
	r->read(type);
	u32 count = 0;
	r->read(count);
	if (type == 129) {
		for (u32 i = 0; i < count; i++) {
			u8 texture_len;
			r->read(texture_len);
			shader s;
			if (texture_len > 0) {
				r->read_string(s.texture, texture_len);
				s.texture.pop_back();
			}
			r->read(s.unk);
			pkgfile.shaders.push_back(s);
		}
	}
	else if (type == 139) {
		while (!r->at_end(1)) {
			u8 texture_len;
			r->read(texture_len);
			shader s;
			if (texture_len > 0) {
				r->read_string(s.texture, texture_len);
				s.texture.pop_back();
			}
			r->read(s.unk);
			pkgfile.shaders.push_back(s);
		}
	}
	else {
		//printf("\nEncountered unknown shader block type: %d\n", type);
		return false;
	}
	return true;
}

bool read_pkg3file(data_container* r, pkg& pkgfile) {
	char fileheader[4];
	r->read(fileheader);
	if (strncmp(fileheader, "PKG3", 4) != 0) {
		r2Error("%s is not a valid PKG3 file", r->name().c_str());
		return false;
	}

	while (!r->at_end(1)) {
		char header[4];
		r->read(header);

		if (strncmp(header, "FILE", 4) != 0) {
			r2Error("Failed to read FILE block header from '%s'", r->name().c_str());
			return false;
		}

		u8 name_len = 0;
		r->read(name_len);
		string file_name;
		r->read_string(file_name, name_len);
		file_name.pop_back();
		u32 file_length;
		r->read(file_length);

		//printf("File %s: (Length: %d)\n", file_name.c_str(), file_length);
		data_container* data = r->sub(file_length);

		if (file_name == "shaders") {
			if (!read_shaders(data, pkgfile)) return false;
		}
		else if (file_name == "offset") {
			printf("offset\n");
		}
		else if (file_name == "xref") {
			printf("xref\n");
		}
		else if (file_name == "H") {
			pkg_mesh& m = pkgfile.new_mesh();
			m.name = file_name;
			if (!read_geometry(data, m)) return false;
		}
		else if (file_name.find("_H") != string::npos) {
			pkg_mesh& m = pkgfile.new_mesh();
			m.name = file_name;
			if (!read_geometry(data, m)) return false;
		}
		//printf("End File %s: (Length: %d)\n", file_name.c_str(), file_length);

		r2engine::files()->destroy(data);
	}

	return true;
}