#pragma once
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class game_archive;
	class tex_file;

	class btx2_file {
		public:
			struct path_unk0_b {
				union {
					struct {
						r2::u8 unk0;
						r2::u8 unk1; // appears to be 0 - pathTexIds.size() or 128 to (128 + pathTexIds.size())
						r2::u8 unk2[4];
					} defined;
					r2::u8 unk0[6];
				};
			};

			struct path_unk1_b {
				r2::u16 unk0;
				r2::u8 unk1;
				r2::u8 pathTexIdIdx; // can be: 0 - pathTexIds.size(), 252-255
			};

			struct path_pixel {
				r2::u32 unk1_idx;
				r2::u32 unk0_idx;
			};

			btx2_file();
			~btx2_file();

			bool read(game_archive* archive, r2::data_container* file);

			inline r2::f32* height_map() const { return m_heightMap; }
			inline r2::u8* tex_id_map() const { return m_texIdMap; }
			inline r2::u8* tex_orientation_map() const { return m_texOrientationMap; }
			inline r2::u8* path_tex_id_map() const { return m_pathTexIdMap; }
			inline r2::u32* path_info_map() const { return m_pathInfoMap; }
			inline r2::u32 width() const { return m_width; }
			inline r2::u32 length() const { return m_length; }
			inline r2::f32 min_height() const { return m_minHeight; }
			inline r2::f32 max_height() const { return m_maxHeight; }
			inline size_t texture_count() const { return m_textures.size(); }
			inline r2::mstring texture(size_t idx) { return m_textures[idx]; }

			r2::mvector<r2::u32> pathTexIds;
			r2::mvector<path_unk0_b> pathMapUnkData0;
			r2::mvector<path_unk1_b> pathMapUnkData1;
			r2::mvector<path_pixel> pathMapPixels;

		protected:
			r2::f32* m_heightMap;
			r2::f32 m_minHeight;
			r2::f32 m_maxHeight;
			r2::u8* m_texIdMap;
			r2::u8* m_texOrientationMap;
			r2::u8* m_pathTexIdMap;
			r2::u32* m_pathInfoMap;
			r2::u32 m_width;
			r2::u32 m_length;
			r2::mvector<r2::mstring> m_textures;
	};
};

