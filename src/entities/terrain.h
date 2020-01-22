#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class uniform_format;
	class node_material;
	class node_material_instance;
	class render_node;
	class shader_program;
	class scene;
};

namespace sr2 {
	class gamedata_manager;
	class btx2_file;
};

class terrain_entity : public scene_entity {
	public:
		terrain_entity(const r2::mstring& btx2, const r2::mstring& lightMap, r2::scene* s, sr2::gamedata_manager* gdm);
		~terrain_entity();

		virtual void onInitialize();

		r2::shader_program* shader;
		r2::uniform_format* material_format;
		r2::node_material* material;
		r2::node_material_instance* material_instance;
		r2::render_node* node;
		sr2::btx2_file* data;
};