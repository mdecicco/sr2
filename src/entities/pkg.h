#pragma once
#include <r2/config.h>
#include <r2/managers/sceneman.h>

namespace sr2 {
	class bbnd_file;

	class pkg_entity : public scene_entity {
		public:
			pkg_entity(const mstring& name, render_node* rnode, bbnd_file* bounds, const mat4f& transform);
			~pkg_entity();

			virtual void onInitialize();

			render_node* node;
			bbnd_file* bounds;
			mat4f trns;
	};
};

