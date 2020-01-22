#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class bbnd_file {
		public:
			struct material {
				r2::mstring name;
				r2::vec2f unk0;
				r2::mstring unk1;
				r2::mstring unk2;
			};
			struct vertex {
				r2::vec3f position;
				r2::u16 material_index;
			};

			bbnd_file();
			~bbnd_file();

			bool read(r2::data_container* file);

			r2::mvector<vertex> vertices;
			r2::mvector<material> materials;
	};
};

