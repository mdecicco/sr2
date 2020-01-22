#pragma once
#include <algorithm>
#include <r2/engine.h>
using namespace r2;

#pragma pack(push, 1)

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define SET_BIT(var, pos, x) var ^= (-x ^ var) & (1UL << pos)
#define BTX2_BLOCK_UNK0				8192
#define BTX2_BLOCK_INFO				8193
#define BTX2_BLOCK_TEXID_VALUES		8195
#define BTX2_BLOCK_UNK_MAP1			8196
#define BTX2_BLOCK_MATERIALS		8197
#define BTX2_BLOCK_TEXTURES			8198
#define BTX2_BLOCK_UNK_VALUES		8199
#define BTX2_BLOCK_PATH_MAP			8201
#define BTX2_BLOCK_HEIGHT_VALUES	8203
#define BTX2_BLOCK_UNK_MAP2			8204
#define BTX2_BLOCK_UNK_MAP4			8205
#define BTX2_BLOCK_UNK_MAP0			8206

class btx2 {
	public:
		struct path_unk0_b {
			union {
				struct {
					u8 unk0;
					u8 unk1; // appears to be 0 - pathTexIds.size() or 128 to (128 + pathTexIds.size())
					u8 unk2[4];
				} defined;
				u8 unk0[6];
			};
		};

		struct path_unk1_b {
			u16 unk0;
			u8 unk1;
			u8 pathTexIdIdx; // can be: 0 - pathTexIds.size(), 252-255
		};

		struct path_pixel {
			u32 unk1_idx;
			u32 unk0_idx;
		};

		btx2() {
			header_unk0 = 0;
			info_unk0 = 0;
			info_unk1 = 0;
			width = 0;
			height = 0;
			texId_values = 0;
			height_values = 0;
			orientation_map = 0;
			unknown_map = 0;
			unknown_map1 = 0;
			unknown_map2_0 = 0;
			unknown_map2_1 = 0;
			pathTexId_map = 0;
			unknown_map4 = 0;
		}

		~btx2() {
			if (texId_values) delete[] texId_values;
			if (height_values) delete[] height_values;
			if (orientation_map) delete[] orientation_map;
			if (unknown_map) delete[] unknown_map;
			if (unknown_map1) delete[] unknown_map1;
			if (unknown_map2_0) delete[] unknown_map2_0;
			if (unknown_map2_1) delete[] unknown_map2_1;
			if (pathTexId_map) delete[] pathTexId_map;
			if (unknown_map4) delete[] unknown_map4;
		}

		bool read(data_container* r) {
			r2Log("Loading terrain '%s'", r->name().c_str());
			char magic[4] = { 0 };
			r->read(magic);
			if (strncmp(magic, "TSV1", 4) != 0) {
				printf("Not a valid btx2 file\n");
				return false;
			}

			while (!r->at_end(1)) {
				u16 type = 0;
				r->read(type);
				if (type == 0) {
					// end of file reached
					break;
				}

				u32 length = 0;
				r->read(length);
				data_container* data = r->sub(length);

				read_block(type, data);
			}

			//build_blend_map();
			//replace_blend_textures();

			return true;
		}

		void read_unk0_block(data_container* r) {
			r->read(header_unk0);
		}

		void read_info_block(data_container* r) {
			r->read(info_unk0);
			r->read(width);
			r->read(height);
			r->read(info_unk1);
			FILE* fp = fopen(".\\sr_maps\\info.txt", "w");
			fprintf(fp, "header: %d\n", header_unk0);
			fprintf(fp, "unk0: %d\n", info_unk0);
			fprintf(fp, "width: %d\n", width);
			fprintf(fp, "height: %d\n", height);
			fprintf(fp, "unk1: %d\n", info_unk1);
			fclose(fp);
		}

		void read_materials_block(data_container* r) {
			u32 count = 0;
			r->read(count);
			FILE* fp = fopen(".\\sr_maps\\materials.txt", "w");
			for (u32 i = 0; i < count; i++) {
				char name[32];
				r->read(name);
				material_names.push_back(name);
				fprintf(fp, "%d: %s\n", i, name);
			}
			fclose(fp);
		}

		void read_textures_block(data_container* r) {
			u32 count = 0;
			r->read(count);
			FILE* fp = fopen(".\\sr_maps\\textures.txt", "w");
			for (u32 i = 0; i < count; i++) {
				char name[34];
				r->read(name);
				texture_names.push_back(name);
				fprintf(fp, "%d: %s\n", i, name);
			}
			fclose(fp);
		}

		void read_texid_block(data_container* r) {
			texId_values = new u8[width * height];

			u16 y = 0;
			while(!r->at_end(1) && y < height) {
				r->read_data(&texId_values[y * width], width);
				r->seek(32);
				y++;
			}

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\texId_%dx%d.raw", width, height);
			FILE* fp = fopen(fn, "wb");
			fwrite(texId_values, 1, width * height, fp);
			fclose(fp);
		}

		void read_heights_block(data_container* r) {
			height_values = new f32[width * height];
			orientation_map = new u8[width * height];

			u16 y = 0;
			while(!r->at_end(1) && y < height) {
				r->read_data(&height_values[y * width], width * sizeof(f32));
				f32* ptr = &height_values[y * width];
				for (u32 x = 0;x < width;x++) {
					u32 val = ((u32*)ptr)[x];
					// These dictate cell texture orientation
					bool b10 = !CHECK_BIT(val, 10);
					bool b11 = !CHECK_BIT(val, 11);

					if ( b10 &&  b11) orientation_map[x + (y * width)] = 0; // rotate tile 0 * 90 degrees
					if ( b10 && !b11) orientation_map[x + (y * width)] = 2; // rotate tile 2 * 90 degrees
					if (!b10 &&  b11) orientation_map[x + (y * width)] = 1; // rotate tile 1 * 90 degrees
					if (!b10 && !b11) orientation_map[x + (y * width)] = 3; // rotate tile 3 * 90 degrees
				}
				r->seek(32 * sizeof(f32));
				y++;
			}

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\heightmap_%dx%d.raw", width, height);
			FILE* fp = fopen(fn, "wb");
			fwrite(height_values, 1, width * height * 4, fp);
			fclose(fp);
		}

		void read_unknown_map_block_0(data_container* r) {
			unknown_map = new u8[width * height];
			r->read_data(unknown_map, width * height);

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\unk0_%dx%d.raw", width, height);
			FILE* fp = fopen(fn, "wb");
			fwrite(unknown_map, 1, width * height, fp);
			fclose(fp);
		}

		void read_unknown_map_block_1(data_container* r) {
			u16 palette[256];
			r->read_data(palette, 256 * sizeof(u16));

			unknown_map1 = new u16[width * height];
			u8* row = new u8[width + 32];
			u16 y = 0;
			while(!r->at_end(1) && y < height) {
				r->read_data(row, width + 32);
				for(u16 x = 0;x < width;x++) {
					unknown_map1[x + (y * width)] = palette[row[x]];
				}
				y++;
			}
			delete[] row;

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\unk1_%dx%d.raw", width, height);
			FILE* fp = fopen(fn, "wb");
			fwrite(unknown_map1, 1, width * height * sizeof(u16), fp);
			fclose(fp);
		}

		void read_unknown_map_block_2(data_container* r) {
			unknown_map2_0 = new f32[(width / 8) * (height / 8)];
			unknown_map2_1 = new f32[(width / 8) * (height / 8)];

			FILE* fp = fopen(".\\sr_maps\\unk2_unprocessed.raw", "wb");
			fwrite(r->data(), 1, r->size() - r->position(), fp);
			fclose(fp);

			u32 i = 0;
			while(!r->at_end(1)) {
				r->read(unknown_map2_0[i]);
				r->read(unknown_map2_1[i]);
				i++;
			}

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\unk2_0_%dx%d.rawf", width / 8, height / 8);
			fp = fopen(fn, "wb");
			fwrite(unknown_map2_0, (width / 8) * (height / 8) * sizeof(f32), 1, fp);
			fclose(fp);
			memset(fn, 0, 128);
			snprintf(fn, 128, ".\\sr_maps\\unk2_1_%dx%d.rawf", width / 8, height / 8);
			fp = fopen(fn, "wb");
			fwrite(unknown_map2_1, (width / 8) * (height / 8) * sizeof(f32), 1, fp);
			fclose(fp);
		}

		void read_path_map(data_container* r) {
			// read unk0 blocks

			u32 unk0_count = 0;
			r->read(unk0_count);
			for(u32 i = 0;i < unk0_count;i++) {
				path_unk0_b b;
				r->read(b);
				pathMapUnkData0.push_back(b);
			}

			// read unk1 blocks
			u32 unk1_count = 0;
			r->read(unk1_count);
			for(u32 i = 0;i < unk1_count;i++) {
				path_unk1_b b;
				r->read(b);
				pathMapUnkData1.push_back(b);
			}

			// read path texture indices
			u32 unk2_count = 0;
			r->read(unk2_count);
			for(u32 i = 0;i < unk2_count;i++) {
				u32 texId = 0;
				r->read(texId);
				pathTexIds.push_back(texId);
			}


			// read unk3 blocks
			// will read (width / 8) * (height / 8) blocks
			while(!r->at_end(1)) {
				path_pixel e;
				r->read(e);
				pathMapPixels.push_back(e);
			}

			pathTexId_map = new u8[(width / 8) * (height / 8)];
			u32 pidx = 0;
			for (u32 p = 0;p < pathMapPixels.size();p++) {
				path_pixel& pixel = pathMapPixels[p];

				if (pixel.unk0_idx == 0xFFFFFFFF) {
					pathTexId_map[pidx] = 255;
				} else {
					auto& unk0 = pathMapUnkData0[pixel.unk0_idx];
					auto& unk1 = pathMapUnkData1[pixel.unk1_idx];

					u32 pathTexIdIdx = unk0.defined.unk1;
					bool some_flag = false;
					if (pathTexIdIdx >= 128) {
						pathTexIdIdx -= 128;
						some_flag = true;
					}
					u32 texId = pathTexIds[pathTexIdIdx];
					pathTexId_map[pidx] = (u8)texId;
				}
				pidx++;
			}
		}

		void read_unknown_map_block_4(data_container* r) {
			unknown_map4 = new u8[width * height];

			FILE* fp = fopen(".\\sr_maps\\unk4_unprocessed.raw", "wb");
			fwrite(r->data(), 1, r->size() - r->position(), fp);
			fclose(fp);

			u32 i = 0;
			while(!r->at_end(1)) {
				u8 v = 0;
				r->read(v);
				unknown_map4[i++] = CHECK_BIT(v, 0) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 1) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 2) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 3) ? 255 : 0;

				unknown_map4[i++] = CHECK_BIT(v, 4) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 5) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 6) ? 255 : 0;
				unknown_map4[i++] = CHECK_BIT(v, 7) ? 255 : 0;
			}

			char fn[128] = { 0 };
			snprintf(fn, 128, ".\\sr_maps\\unk4_%dx%d.raw", width, height);
			fp = fopen(fn, "wb");
			fwrite(unknown_map4, 1, width * height, fp);
			fclose(fp);
		}

		void read_unknown_values(data_container* r) {
			f32 values[25];
			r->read_data(values, 25 * sizeof(f32));

			FILE* fp = fopen(".\\sr_maps\\unk_values.txt", "w");
			for(u8 i = 0;i < 25;i++) {
				fprintf(fp, "%d: %f\n", i, values[i]);
			}
			fclose(fp);
		}

		void read_block(u16 type, data_container* r) {
			switch (type) {
				case BTX2_BLOCK_UNK0:
					r2Log("Reading unk0 block (type: %d)", type);
					read_unk0_block(r);
					break;
				case BTX2_BLOCK_INFO:
					r2Log("Reading info block (type: %d)", type);
					read_info_block(r);
					break;
				case BTX2_BLOCK_MATERIALS:
					break;
					r2Log("Reading materials (type: %d)", type);
					read_materials_block(r);
					break;
				case BTX2_BLOCK_TEXTURES:
					r2Log("Reading texture names (type: %d)", type);
					read_textures_block(r);
					break;
				case BTX2_BLOCK_TEXID_VALUES:
					r2Log("Reading texture ID map (type: %d)", type);
					read_texid_block(r);
					break;
				case BTX2_BLOCK_HEIGHT_VALUES:
					r2Log("Reading height map (type: %d)", type);
					read_heights_block(r);
					break;
				case BTX2_BLOCK_UNK_MAP0:
					break;
					r2Log("Reading unknown map 0 (type: %d)", type);
					read_unknown_map_block_0(r);
					break;
				case BTX2_BLOCK_UNK_MAP1:
					break;
					r2Log("Reading unknown map 1 (type: %d)", type);
					read_unknown_map_block_1(r);
					break;
				case BTX2_BLOCK_UNK_MAP2:
					r2Log("Reading unknown map 2 (type: %d)", type);
					read_unknown_map_block_2(r);
					break;
				case BTX2_BLOCK_PATH_MAP:
					r2Log("Reading unknown map 3 (type: %d)", type);
					read_path_map(r);
					break;
				case BTX2_BLOCK_UNK_MAP4:
					break;
					r2Log("Reading unknown map 4 (type: %d)", type);
					read_unknown_map_block_4(r);
					break;
				case BTX2_BLOCK_UNK_VALUES:
					break;
					r2Log("Reading unknown values (type: %d)", type);
					read_unknown_values(r);
					break;
				default:
					r2Log("Reading unknown block (type: %d)", type);
					char fn[128] = { 0 };
					snprintf(fn, 128, ".\\sr_maps\\block_0x%X.dat", type);
					FILE* fp = fopen(fn, "wb");
					r->set_position(0);
					fwrite(r->data(), r->size(), 1, fp);
					fclose(fp);
					break;
			}
		}

		u32 header_unk0;
		u32 info_unk0;
		u32 info_unk1;
		u8* unknown_map;
		u16* unknown_map1;
		f32* unknown_map2_0;
		f32* unknown_map2_1;
		u8* unknown_map4;

		// semi-knowns
		u8* pathTexId_map;
		mvector<u32> pathTexIds;
		mvector<path_unk0_b> pathMapUnkData0;
		mvector<path_unk1_b> pathMapUnkData1;
		mvector<path_pixel> pathMapPixels;

		// knowns
		u32 width;
		u32 height;
		mvector<mstring> material_names;
		mvector<mstring> texture_names;
		u8* texId_values;
		u8* orientation_map;
		f32* height_values;
};

class imap {
	public:
		imap(data_container* file) {
			values = 0;
			length = 0;
			length = file->size();
			values = new u8[length];
			file->read_data(values, length);
		}
		~imap() {
			if (values) delete[] values;
		}

		u32 length;
		u8* values;
};

#pragma pack(pop)