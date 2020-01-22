#pragma once
#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class bnd_file {
		public:
			struct material {
				r2::mstring name;
				r2::mstring effect;
				r2::mstring sound;
				r2::f32 elasticity;
				r2::f32 friction;
			};
			struct vertex {
				r2::vec3f position;
				r2::u16 material_index;
			};

			bnd_file();
			~bnd_file();

			bool read(r2::data_container* file);

			r2::mvector<vertex> vertices;
			r2::mvector<material> materials;
	};
};

