#pragma once
#include <gamedata/bbnd_file.h>

#include <r2/engine.h>
using namespace r2;

#pragma pack(push, 1)
namespace sr2 {
	bbnd_file::bbnd_file() {
	}

	bbnd_file::~bbnd_file() {
	}

	bool bbnd_file::read(data_container* file) {
		file->set_position(0);
		struct header {
			u8 unk; // always 1?
			u32 vertex_count;
			u32 material_count;
			u32 face_count;
		};
		struct face {
			u16 idx0;
			u16 idx1;
			u16 idx2;
			u16 idx3; // garbage, unless quads?
			u16 material_index;
		};
		struct mtrl {
			char name[32];
			vec2f unk0;
			char unk1[32];
			char unk2[32];
		};

		header hdr;
		if (!file->read(hdr)) {
			r2Error("Failed to read bbnd header");
			return false;
		}

		if (hdr.unk != 1) {
			r2Error("Unknown bbnd header");
			return false;
		}

		vec3f* verts = new vec3f[hdr.vertex_count];
		if (!file->read_data(verts, hdr.vertex_count * sizeof(vec3f))) {
			r2Error("Failed to read bbnd vertices");
			delete[] verts;
			return false;
		}

		mtrl* mtrls = new mtrl[hdr.material_count];
		if (!file->read_data(mtrls, hdr.material_count * sizeof(mtrl))) {
			r2Error("Failed to read bbnd materials");
			delete[] verts;
			delete[] mtrls;
			return false;
		}

		face* faces = new face[hdr.face_count];
		if (!file->read_data(faces, hdr.face_count * sizeof(face))) {
			r2Error("Failed to read bbnd faces");
			delete[] verts;
			delete[] mtrls;
			delete[] faces;
			return false;
		}

		for (u32 i = 0;i < hdr.face_count;i++) {
			vertices.push_back({ verts[faces[i].idx0], faces[i].material_index });
			vertices.push_back({ verts[faces[i].idx1], faces[i].material_index });
			vertices.push_back({ verts[faces[i].idx2], faces[i].material_index });
			if(faces[i].idx3 < hdr.vertex_count) {
				vertices.push_back({ verts[faces[i].idx0], faces[i].material_index });
				vertices.push_back({ verts[faces[i].idx2], faces[i].material_index });
				vertices.push_back({ verts[faces[i].idx3], faces[i].material_index });
			}
		}

		for (u32 i = 0;i < hdr.material_count;i++) {
			materials.push_back({
				mtrls[i].name,
				mtrls[i].unk0,
				mtrls[i].unk1,
				mtrls[i].unk2
			});
		}

		delete[] verts;
		delete[] mtrls;
		delete[] faces;
		return true;
	}
};

#pragma pack(pop)