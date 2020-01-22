#include <gamedata/btx2_file.h>
#include <gamedata/tex_file.h>
#include <gamedata/gamedata.h>
#include <gamedata/utils/btx2.hpp>

#include <r2/engine.h>

using namespace r2;

namespace sr2 {
	btx2_file::btx2_file() : m_width(0), m_length(0), m_heightMap(nullptr), m_texIdMap(nullptr), m_texOrientationMap(nullptr) {
	}

	btx2_file::~btx2_file() {
		if (m_heightMap) delete[] m_heightMap;
		m_heightMap = nullptr;

		if (m_texIdMap) delete[] m_texIdMap;
		m_texIdMap = nullptr;

		if (m_texOrientationMap) delete[] m_texOrientationMap;
		m_texOrientationMap = nullptr;

		m_textures.clear();
	}

	bool btx2_file::read(game_archive* archive, data_container* file) {
		btx2 info;
		if (info.read(file)) {
			// Steal the relevant info
			m_heightMap = info.height_values;
			info.height_values = nullptr;
			m_texIdMap = info.texId_values;
			info.texId_values = nullptr;
			m_texOrientationMap = info.orientation_map;
			info.orientation_map = nullptr;
			
			pathTexIds = info.pathTexIds;
			for (auto& e : info.pathMapUnkData0) { pathMapUnkData0.push_back(*(path_unk0_b*)&e); }
			for (auto& e : info.pathMapUnkData1) { pathMapUnkData1.push_back(*(path_unk1_b*)&e); }
			for (auto& e : info.pathMapPixels) { pathMapPixels.push_back(*(path_pixel*)&e); }

			m_pathTexIdMap = new u8[(size_t)info.width * (size_t)info.height];
			m_pathInfoMap = new u32[(size_t)info.width * (size_t)info.height * 4];
			memset(m_pathTexIdMap, 0xFF, (size_t)info.width * (size_t)info.height);
			memset(m_pathInfoMap, 0xFF, sizeof(u32) * (size_t)info.width * (size_t)info.height * 4);

			m_minHeight = m_heightMap[0];
			m_maxHeight = m_minHeight;
			for (u32 y = 0;y < info.height - 1;y++) {
				for (u32 x = 0;x < info.width - 1;x++) {
					f32 height = m_heightMap[x + (y * info.width)];
					if (height < m_minHeight) m_minHeight = height;
					if (height > m_maxHeight) m_maxHeight = height;

					u32 sx = x + 1;
					u32 sy = y + 1;
					vec2i terrainCoord(x, y);
					vec2i pathMapSize(info.width / 8, info.height / 8);
					vec2i pathMapCoord = terrainCoord / 8;

					if (pathMapCoord.x >= 0 && pathMapCoord.x < pathMapSize.x && pathMapCoord.y >= 0 && pathMapCoord.y < pathMapSize.y) {
						auto& path_pixel = info.pathMapPixels[(size_t)pathMapCoord.x + ((size_t)pathMapCoord.y * (size_t)pathMapSize.x)];

						if (path_pixel.unk0_idx != 0xFFFFFFFF) {
							vec2i cellCoord = vec2i(terrainCoord.x - (pathMapCoord.x * 8), terrainCoord.y - (pathMapCoord.y * 8));
							u8 cidx = cellCoord.x + (cellCoord.y * 8);

							btx2::path_pixel* next = nullptr;
							for (u32 i = pathMapCoord.x + (pathMapCoord.y * pathMapSize.x) + 1;i < info.pathMapPixels.size();i++) {
								auto& next_pixel = info.pathMapPixels[(size_t)i];
								if (next_pixel.unk0_idx != 0xFFFFFFFF) {
									next = &next_pixel;
									break;
								}
							}

							btx2::path_pixel diff;
							diff.unk0_idx = next ? next->unk0_idx - path_pixel.unk0_idx : info.pathMapUnkData0.size() - path_pixel.unk0_idx;
							diff.unk1_idx = next ? next->unk1_idx - path_pixel.unk1_idx : info.pathMapUnkData1.size() - path_pixel.unk1_idx;

							btx2::path_unk0_b* block = nullptr;
							for (u16 i = 0;i < diff.unk0_idx;i++) {
								auto& b = info.pathMapUnkData0[(size_t)path_pixel.unk0_idx + (size_t)i];
								if (b.unk0[0] == cidx) {
									block = &b;
									break;
								}
							}

							if (block) {
								u32 pathTexIdIdx = block->defined.unk1;
								bool some_flag = false;
								if (pathTexIdIdx >= 128) {
									pathTexIdIdx -= 128;
									some_flag = true;
								}
								m_pathTexIdMap[sx + (sy * info.width)] = info.pathTexIds[pathTexIdIdx];

								btx2::path_unk1_b* u[4] = {
									&info.pathMapUnkData1[(size_t)path_pixel.unk1_idx + (size_t)block->unk0[2]],
									&info.pathMapUnkData1[(size_t)path_pixel.unk1_idx + (size_t)block->unk0[3]],
									&info.pathMapUnkData1[(size_t)path_pixel.unk1_idx + (size_t)block->unk0[4]],
									&info.pathMapUnkData1[(size_t)path_pixel.unk1_idx + (size_t)block->unk0[5]]
								};

								for (u32 i = 0;i < 4;i++) {
									auto data = u[i];
									u8 xyzw[4] = {
										((u8*)&data->unk0)[0],
										((u8*)&data->unk0)[1],
										data->unk1,
										data->pathTexIdIdx
									};

									memcpy(&m_pathInfoMap[(sx * 4) + (sy * info.width * 4) + i], xyzw, sizeof(u32));
								}
							}
						}
					}
				}
			}

			m_width = info.width;
			m_length = info.height;

			// Store textures used by terrain
			for (auto tex : info.texture_names) {
				mstring filename = "texture/" + tex + ".tex";
				m_textures.push_back(filename);
			}

			return true;
		}

		return false;
	}
};
