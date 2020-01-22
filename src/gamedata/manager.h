#pragma once
#include <r2/config.h>
#include <r2/managers/sceneman.h>
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	enum DATA_MODE;
	class audio_source;
};

namespace sr2 {
	class dave_zip;
	class ramshop_zip;
	class btx2_file;
	class bbnd_file;
	class mission;
	class pkg_entity;
	class vehicle_entity;

	class gamedata_manager {
		public:
			gamedata_manager(r2::scene* s);

			~gamedata_manager();

			std::pair<btx2_file*, r2::render_node*> get_terrain(const mstring& file, const mstring& lightMapFile, r2::node_material_instance* material);

			r2::mvector<std::pair<r2::render_node*, bbnd_file*>> get_mesh(const mstring& file, u32 max_instances);

			render_node* get_sky(const mstring& file);

			r2::uniform_block* get_fog(const mstring& file);

			vehicle_entity* get_vehicle(const mstring& file, u32 max_instances);

			r2::audio_source* get_audio(const mstring& file);

			r2::texture_buffer* get_texture(const mstring& file);

			r2::mvector<r2::texture_buffer*> get_texture_sequence(const mstring& file);

			r2::mvector<pkg_entity*> get_instances(const mstring& file);

			mission* get_mission(const mstring& name);

			r2::data_container* open(const mstring& file, r2::DATA_MODE mode);

			void render_debug_ui();

			dave_zip* archive() const { return m_archive; }

		protected:
			dave_zip* m_archive;
			ramshop_zip* m_ramshop;

			r2::scene* m_scene;
			r2::vertex_format* m_terrainVertexFormat;
			r2::texture_buffer* m_terrainLightMap;
			r2::vertex_format* m_pkgVertexFormat;
			r2::instance_format* m_pkgInstanceFormat;
			r2::uniform_format* m_pkgMtrlFormat;
			r2::node_material* m_pkgMtrl;
			r2::shader_program* m_pkgShdr;
			r2::uniform_format* m_skyMtrlFormat;
			r2::node_material* m_skyMtrl;
			r2::shader_program* m_skyShdr;
			r2::uniform_format* m_fogFormat;
			r2::uniform_block* m_fogUniforms;
			r2::associative_pod_array<r2::mstring, r2::audio_source*> m_sounds;
			r2::associative_pod_array<r2::mstring, r2::render_node*> m_nodes;
			r2::associative_pod_array<r2::mstring, r2::texture_buffer*> m_textures;
			r2::associative_pod_array<r2::mstring, r2::vertex_format*> m_meshVertexFormats;
			r2::munordered_map<r2::mstring, r2::mvector<std::pair<r2::render_node*, bbnd_file*>>> m_meshes;
	};
};