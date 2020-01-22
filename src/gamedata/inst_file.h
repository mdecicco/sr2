#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class game_archive;
	class inst_file {
		public:
			struct instance {
				r2::u8 unk0;
				r2::u16 unk1;
				r2::u8 unk2;
				r2::u8 unk3;
				r2::vec2f unk4;
				r2::vec3f position;
			};

			inst_file();
			~inst_file();

			bool read(game_archive* archive, r2::data_container* file);

			r2::munordered_map<r2::mstring, r2::mvector<instance>> instances;
	};
};

