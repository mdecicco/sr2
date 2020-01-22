#include <gamedata/pkg_file.h>
#include <gamedata/utils/pkg.hpp>
#include <r2/engine.h>

using namespace r2;

namespace sr2 {
	pkg_file::pkg_file() {
	}

	pkg_file::~pkg_file() {
	}

	bool pkg_file::read(game_archive* archive, data_container* file) {
		pkg p;
		if (!read_pkg3file(file, p)) {
			return false;
		}

		auto c4b2v4f = [](const color4b& c) {
			return vec4f(f32(c.r) / 255.0f, f32(c.g) / 255.0f, f32(c.b) / 255.0f, f32(c.a) / 255.0f);
		};

		for (u32 m = 0;m < p.meshes.size();m++) {
			pkg_mesh& submesh = p.meshes[m];

			for (u32 i = 0; i < submesh.sections.size(); i++) {
				mesh_section& section = submesh.sections[i];

				mesh out;
				out.name = submesh.name;

				u32 idxOffset = 0;
				for (u32 s = 0; s < section.strips.size(); s++) {
					section_strip& strip = section.strips[s];

					for (u32 idx = 0; idx < strip.indices.size(); idx++) {
						out.indices.push_back(u32(strip.indices[idx]) + idxOffset);
					}

					for (u32 idx = 0; idx < strip.vertices.size(); idx++) {
						mesh::vertex v = {
							strip.vertices[idx].pos,
							strip.vertices[idx].normal,
							strip.vertices[idx].texcoord
						};
						out.vertices.push_back(v);
					}

					idxOffset += strip.vertices.size();
				}

				u32 sidx = section.shaderIdx;
				if (sidx == 256) sidx = section.unknown;
				if (p.shaders.size() > 0 && sidx < p.shaders.size()) {
					shader& s = p.shaders[sidx];
					if (s.texture.length() > 0) out.material.texture = "texture/" + s.texture + ".tex";
					out.material.unk[0] = c4b2v4f(s.unk[0]);
					out.material.unk[1] = c4b2v4f(s.unk[1]);
					out.material.unk[2] = c4b2v4f(s.unk[2]);
					out.material.unk[3] = c4b2v4f(s.unk[3]);
				}

				meshes.push_back(out);
			}
		}

		return true;
	}
};
