#include <gamedata/mission.h>
#include <gamedata/manager.h>
#include <r2/engine.h>
using namespace r2;

class TSV {
	public:
		struct block_info {
			u16 type;
			u32 size;
			u32 offset;
		};

		TSV() : m_container(nullptr) { }
		~TSV() { }

		bool parse(data_container* file) {
			file->set_position(0);
			char hdr[4] = { 0 };
			file->read(hdr);
			if (strnicmp(hdr, "TSV1", 4) != 0) {
				r2Error("File '%s' is not a TSV file", file->name().c_str());
				return false;
			}

			while(!file->at_end(3)) {
				block_info b;
				if (!file->read(b.type)) return false;
				if (!file->read(b.size)) return false;
				b.offset = file->position();
				file->seek(b.size);
				m_blocks.push_back(b);
			}

			m_container = file;
		}

		size_t count() const {
			return m_blocks.size();
		}

		u16 type(size_t idx) {
			return m_blocks[idx].type;
		}

		data_container* operator[](size_t idx) {
			block_info block = m_blocks[idx];
			m_container->set_position(block.offset);
			return m_container->sub(block.size);
		}

		data_container* find(u16 type) {
			for (auto& block : m_blocks) {
				if (block.type == type) {
					m_container->set_position(block.offset);
					return m_container->sub(block.size);
				}
			}

			return nullptr;
		}

	protected:
		data_container* m_container;
		mvector <block_info> m_blocks;
};

#define MISSION_START_POSITION_BLOCK		0x0101

namespace sr2 {
	mission::mission() {
	}

	mission::~mission() {
	}

	bool mission::load(gamedata_manager* mgr, const mstring& name) {
		data_container* miss = mgr->open("mission/" + name + ".miss", DM_BINARY);
		if (!miss) return false;
		
		TSV blocks;
		if (!blocks.parse(miss)) {
			r2engine::files()->destroy(miss);
			return false;
		}

		data_container* start_pos = blocks.find(MISSION_START_POSITION_BLOCK);
		if (start_pos) {
			start_pos->read(player_start_pos);
			f32 unk = 0.0f;
			start_pos->read(unk);
			r2engine::files()->destroy(start_pos);
		}

		entities = mgr->get_instances("mission/" + name + ".inst");
	}
};