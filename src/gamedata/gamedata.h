#pragma once
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	class data_container;
	enum DATA_MODE;
};

namespace miniz_cpp {
	class zip_file;
};

namespace sr2 {
	class tex_file;
	class vag_file;
	class pkg_file;
	class btx2_file;
	class inst_file;
	class bbnd_file;

	class game_archive {
		public:
			game_archive() { }
			virtual ~game_archive() { }

			virtual r2::data_container* open(const r2::mstring& file, r2::DATA_MODE mode) = 0;
			virtual bool exists(const r2::mstring& file) = 0;
			virtual r2::mvector<r2::mstring> file_list() const = 0;

			tex_file* open_texture(const r2::mstring& file);
			vag_file* open_audio(const r2::mstring& file);
			pkg_file* open_mesh(const r2::mstring& file);
			bbnd_file* open_mesh_bounds(const r2::mstring& file);
			btx2_file* open_terrain(const r2::mstring& file);
			inst_file* open_instances(const r2::mstring& file);
	};

	class dave_zip : public game_archive {
		public:
			dave_zip();
			~dave_zip();

			bool load(const char* file);

			virtual r2::data_container* open(const r2::mstring& file, r2::DATA_MODE mode);
			virtual bool exists(const r2::mstring& file);
			virtual r2::mvector<r2::mstring> file_list() const;

		protected:
			struct file_info {
				size_t data_offset;
				size_t decompressed_size;
				size_t compressed_size;
			};
			r2::data_container* m_data;
			r2::associative_pod_array<r2::mstring, file_info> m_files;
	};

	class ramshop_zip : public game_archive {
		public:
			ramshop_zip();
			~ramshop_zip();

			bool load(const char* file);

			virtual r2::data_container* open(const r2::mstring& file, r2::DATA_MODE mode);
			virtual bool exists(const r2::mstring& file);
			virtual r2::mvector<r2::mstring> file_list() const;

		protected:
			miniz_cpp::zip_file* m_archive;
	};
};