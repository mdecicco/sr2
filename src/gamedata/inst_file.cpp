#pragma once
#include <gamedata/gamedata.h>
#include <gamedata/inst_file.h>
#include <gamedata/pkg_file.h>

#include <r2/engine.h>
using namespace r2;

namespace sr2 {
	inst_file::inst_file() {
	}

	inst_file::~inst_file() {
	}
	
	bool inst_file::read(game_archive* archive, data_container* file) {
		while(!file->at_end(1)) {
			instance i;
			file->read(i.unk0);
			file->read(i.unk1);
			file->read(i.unk2);
			file->read(i.unk3);
			mstring filename;
			file->read_string(filename);
			file->read(i.unk4);
			file->read(i.position);
			filename = "geometry/" + filename + ".pkg";
			//r2Log("Instance: '%s' at <%.2f, %.2f, %.2f> with <%.2f, %.2f> and <%d, %d, %d, %d>", ("geometry/" + i.pkg + ".pkg").c_str(), i.position.x, i.position.y, i.position.z, i.unk4.x, i.unk4.y, i.unk0, i.unk1, i.unk2, i.unk3);

			if (instances.count(filename) > 0) instances[filename].push_back(i);
			else {
				instances[filename] = mvector<instance>();
				instances[filename].push_back(i);
			}
		}

		return true;
	}
};

