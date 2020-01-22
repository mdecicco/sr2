#pragma once
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class game_archive;

	class pkg_file {
		public:
			struct mesh {
				r2::mstring name;
				struct vertex {
					r2::vec3f position;
					r2::vec3f normal;
					r2::vec2f texcoord;
				};
				r2::mstring texture;
				r2::mvector<vertex> vertices;
				r2::mvector<r2::u32> indices;
				struct {
					r2::mstring texture;
					r2::vec4f unk[4];
				} material;
			};

			pkg_file();
			~pkg_file();

			bool read(game_archive* archive, r2::data_container* file);

			r2::mvector<mesh> meshes;
	};
}