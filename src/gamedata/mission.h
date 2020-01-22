#pragma once
#include <r2/managers/memman.h>

namespace sr2 {
	class gamedata_manager;
	class pkg_entity;

	class mission {
		public:
			mission();
			~mission();

			bool load(gamedata_manager* mgr, const r2::mstring& name);

			r2::vec3f player_start_pos;
			r2::mvector<pkg_entity*> entities;
	};
}