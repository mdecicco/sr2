#include <gamedata/gamedata.h>
#include <gamedata/tex_file.h>
#include <gamedata/pkg_file.h>
#include <gamedata/vag_file.h>
#include <gamedata/btx2_file.h>
#include <gamedata/inst_file.h>
#include <gamedata/bbnd_file.h>
#include <gamedata/bnd_file.h>

#include <r2/engine.h>
#include <r2/utilities/utils.h>
using namespace r2;

#include <zlib.h>
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <zip_file.hpp>

namespace sr2 {
	tex_file* game_archive::open_texture(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		tex_file* tf = new tex_file();
		if (!tf->read(container)) {
			delete tf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return tf;
	}

	vag_file* game_archive::open_audio(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		vag_file* vf = new vag_file();
		if (!vf->read(container)) {
			delete vf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return vf;
	}

	pkg_file* game_archive::open_mesh(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		pkg_file* pf = new pkg_file();
		if (!pf->read(this, container)) {
			delete pf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return pf;
	}

	bbnd_file* game_archive::open_mesh_bounds(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		bbnd_file* bf = new bbnd_file();
		if (!bf->read(container)) {
			delete bf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return bf;
	}

	bnd_file* game_archive::open_mesh_bounds_text(const mstring& file) {
		data_container* container = open(file, DM_TEXT);
		if (!container) return nullptr;

		bnd_file* bf = new bnd_file();
		if (!bf->read(container)) {
			delete bf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return bf;
	}

	btx2_file* game_archive::open_terrain(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		btx2_file* hf = new btx2_file();
		if (!hf->read(this, container)) {
			delete hf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return hf;
	}

	inst_file* game_archive::open_instances(const mstring& file) {
		data_container* container = open(file, DM_BINARY);
		if (!container) return nullptr;

		inst_file* inf = new inst_file();
		if (!inf->read(this, container)) {
			delete inf;
			r2engine::files()->destroy(container);
			return nullptr;
		}
		r2engine::files()->destroy(container);

		return inf;
	}



	dave_zip::dave_zip() : m_data(nullptr) {
	}

	dave_zip::~dave_zip() {
		if (m_data) r2engine::files()->destroy(m_data);
		m_data = nullptr;
	}

	bool dave_zip::load(const char* file) {
		if (m_data) r2engine::files()->destroy(m_data);
		m_data = r2engine::files()->open(file, DM_BINARY, file);
		if (!m_data) return false;

		auto fail = [this]() {
			r2engine::files()->destroy(this->m_data);
			this->m_data = nullptr;
			this->m_files.clear();
			return false;
		};

		char hdr[4] = { 0 };
		if (!m_data->read(hdr)) return fail();
		if (strncmp(hdr, "DAVE", 4)) {
			r2Error("File %s is not a 'DAVE' archive", file);
			return fail();
		}

		u32 fileCount = 0;
		if (!m_data->read(fileCount)) return fail();

		u32 dirOffset = 0;
		if (!m_data->read(dirOffset)) return fail();
		dirOffset += 2048;

		m_data->set_position(2048);

		struct file_entry {
			size_t name_offset;
			file_info info;
		};

		r2Log("Parsing archive");
		mvector<file_entry> files;
		for (size_t i = 0;i < fileCount;i++) {
			file_entry entry;
			u32 u32tmp = 0;
			if (!m_data->read(u32tmp)) return fail();
			entry.name_offset = size_t(u32tmp) + dirOffset;

			if (!m_data->read(u32tmp)) return fail();
			entry.info.data_offset = u32tmp;

			if (!m_data->read(u32tmp)) return fail();
			entry.info.decompressed_size = u32tmp;

			if (!m_data->read(u32tmp)) return fail();
			entry.info.compressed_size = u32tmp;

			files.push_back(entry);
		}

		r2Log("Storing records of files in memory (%llu files)", files.size());
		for (auto file : files) {
			m_data->set_position(file.name_offset);
			mstring filename;
			m_data->read_string(filename);
			std::transform(filename.begin(), filename.end(), filename.begin(), std::tolower);
			m_files.set(filename, file.info);
		}
	}

	data_container* dave_zip::open(const mstring& file, DATA_MODE mode) {
		r2Log("Attempting to read '%s' from archive", file.c_str());
		mstring filename = file;
		std::transform(filename.begin(), filename.end(), filename.begin(), std::tolower);
		if (!m_data) return nullptr;
		if (!m_files.has(filename)) return nullptr;
		file_info* info = m_files.get(filename);

		if (info->compressed_size == info->decompressed_size) {
			// file is not compressed
			m_data->set_position(info->data_offset);
			return m_data->sub(info->decompressed_size);
		}

		m_data->set_position(info->data_offset);
		u8* compressed = new u8[info->compressed_size];
		m_data->read_data(compressed, info->compressed_size);

		u8* decompressed = new u8[info->decompressed_size];
		memset(decompressed, 0, info->decompressed_size);

		r2Log("Decompressing '%s'", file.c_str());
		z_stream stream;
		stream.zalloc = [](voidpf opaque, uInt items, uInt size) {
			return (voidpf)r2alloc(size_t(items) * size_t(size));
		};
		stream.zfree = [](voidpf opaque, voidpf address) {
			r2free(address);
		};;
		stream.opaque = Z_NULL;
		stream.avail_in = info->compressed_size;
		stream.next_in = compressed;
		stream.avail_out = info->decompressed_size;
		stream.next_out = decompressed;
		if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
			r2Error("Failed to initialize zlib stream for file '%s'", file.c_str());
			delete[] compressed;
			delete[] decompressed;
			return nullptr;
		}
		i32 status = inflate(&stream, Z_NO_FLUSH);
		if (status != Z_OK && status != Z_STREAM_END) {
			r2Error("Failed to inflate file '%s'", file.c_str());
			delete[] compressed;
			delete[] decompressed;
			return nullptr;
		}
		
		if (inflateEnd(&stream) != Z_OK) {
			r2Warn("Call to inflateEnd failed for file '%s'", file.c_str());
		}

		delete [] compressed;

		data_container* result = r2engine::files()->create(mode, file);
		if (!result) {
			delete[] decompressed;
			return nullptr;
		}

		if (!result->write_data(decompressed, info->decompressed_size)) {
			delete[] decompressed;
			r2engine::files()->destroy(result);
			return nullptr;
		}
		result->set_position(0);

		delete[] decompressed;
		return result;
	}

	bool dave_zip::exists(const mstring& file) {
		mstring filename = file;
		std::transform(filename.begin(), filename.end(), filename.begin(), std::tolower);
		return m_files.has(filename);
	}

	mvector<mstring> dave_zip::file_list() const {
		return m_files.keys();
	}



	ramshop_zip::ramshop_zip() : m_archive(nullptr) {
	}

	ramshop_zip::~ramshop_zip() {
		if (m_archive) delete m_archive;
		m_archive = nullptr;
	}

	bool ramshop_zip::load(const char* file) {
		m_archive = new miniz_cpp::zip_file(file);
		mvector<mstring> files = m_archive->namelist();
		if (files.size() == 0) {
			delete m_archive;
			m_archive = nullptr;
			return false;
		}
	}

	data_container* ramshop_zip::open(const mstring& file, DATA_MODE mode) {
		r2Log("Attempting to read '%s' from ramshop", file.c_str());
		if (!m_archive) return nullptr;
		if (!m_archive->has_file(file)) return nullptr;

		r2Log("Decompressing '%s'", file.c_str());
		mstring contents = m_archive->read(file);
		if (contents.length() == 0) {
			r2Error("Failed to decompress '%s'", file.c_str());
			return nullptr;
		}

		data_container* result = r2engine::files()->create(mode, file);
		if (!result) {
			return nullptr;
		}

		if (!result->write_data(&contents[0], contents.length())) {
			r2engine::files()->destroy(result);
			return nullptr;
		}
		result->set_position(0);
		
		return result;
	}

	bool ramshop_zip::exists(const mstring& file) {
		if (!m_archive) return false;
		return m_archive->has_file(file);
	}

	mvector<mstring> ramshop_zip::file_list() const {
		if (!m_archive) return mvector<mstring>();
		return m_archive->namelist();
	}
};