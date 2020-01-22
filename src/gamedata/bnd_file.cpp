#include <gamedata/bnd_file.h>

#include <r2/engine.h>
#include <r2/utilities/utils.h>
using namespace r2;

#pragma pack(push, 1)
namespace sr2 {
	bnd_file::bnd_file() {
	}

	bnd_file::~bnd_file() {
	}

	bool bnd_file::read(data_container* file) {
		file->set_position(0);

		mvector<vec3f> verts;

		while (!file->at_end(1)) {
			mstring s;
			if (!file->read_line(s)) break;
			if (s.length() == 0) continue;

			mvector<mstring> comps = split(s, " \t\n");
			if (comps.size() == 0) continue;

			if (comps[0] == "version:" && comps[1] != "1.01") {
				r2Warn("Unknown .bnd file version '%s'", comps[1].c_str());
				continue;
			}

			if (comps[0] == "v") {
				verts.push_back(vec3f(
					atof(comps[1].c_str()),
					atof(comps[2].c_str()),
					atof(comps[3].c_str())
				));
			} else if (comps[0] == "quad") {
				u16 idx0 = atoi(comps[1].c_str());
				u16 idx1 = atoi(comps[2].c_str());
				u16 idx2 = atoi(comps[3].c_str());
				u16 idx3 = atoi(comps[4].c_str());
				u16 mat_idx = atoi(comps[5].c_str());

				vertices.push_back({ verts[idx0], mat_idx });
				vertices.push_back({ verts[idx1], mat_idx });
				vertices.push_back({ verts[idx2], mat_idx });

				vertices.push_back({ verts[idx0], mat_idx });
				vertices.push_back({ verts[idx2], mat_idx });
				vertices.push_back({ verts[idx3], mat_idx });
			} else if (comps[0] == "tri") {
				u16 idx0 = atoi(comps[1].c_str());
				u16 idx1 = atoi(comps[2].c_str());
				u16 idx2 = atoi(comps[3].c_str());
				u16 mat_idx = atoi(comps[4].c_str());

				vertices.push_back({ verts[idx0], mat_idx });
				vertices.push_back({ verts[idx1], mat_idx });
				vertices.push_back({ verts[idx2], mat_idx });
			} else if (comps[0] == "mtl") {
				material m;
				m.name = comps[1];
				while (!file->at_end(1)) {
					mstring ms;
					if (!file->read_line(ms)) break;
					if (ms.length() == 0) continue;

					mvector<mstring> mcomps = split(ms, " \t\n");
					if (mcomps.size() == 0) continue;

					if (mcomps[0] == "elasticity:") m.elasticity = atof(mcomps[1].c_str());
					else if (mcomps[0] == "friction:") m.friction = atof(mcomps[1].c_str());
					else if (mcomps[0] == "effect:") m.effect = mcomps[1];
					else if (mcomps[0] == "sound:") m.sound = mcomps[1];
					else if (mcomps[0] == "}") {
						materials.push_back(m);
						break;
					}
				}
			}
		}

		return true;
	}
};

#pragma pack(pop)