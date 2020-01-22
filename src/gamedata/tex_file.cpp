#include <gamedata/tex_file.h>
#include <r2/engine.h>

namespace sr2 {
	tex_file::tex_file() : m_data(nullptr), m_width(0), m_height(0) {
	}

	tex_file::~tex_file() {
		if (m_data) delete[] m_data;
		m_data = nullptr;
	}

	bool tex_file::read(data_container* file) {
		if (m_data) delete[] m_data;
		m_data = nullptr;

		texHeader hdr;
		auto fail = [this, file](const char* msg) {
			r2Error(msg, file->name().c_str());
			if (this->m_data) delete[] this->m_data;
			this->m_data = nullptr;
			this->m_width = this->m_height = 0;
			return false;
		};

		if (!file->read(hdr)) return fail("Failed to read tex file header in '%s'");

		m_width = hdr.width;
		m_height = hdr.height;

		m_data = new u8[hdr.width * hdr.height * 4];
		auto pixel = [this, hdr](u32 x, u32 y, u8 value, u8 channel) {
			this->m_data[((x * 4) + (y * hdr.width * 4) + channel)] = value;
		};
		switch (hdr.type) {
			//P8, PA8
			case 1:
			case 14: {
				struct colorMapEntryA8 palette[256];
				if (!file->read(palette)) return fail("Failed to read P8/PA8 color palette for '%s'");

				struct Pixel8* pixels = new Pixel8[hdr.width * hdr.height];
				if (!file->read_data(pixels, hdr.width * hdr.height * sizeof(Pixel8))) return fail("Failed to read P8/PA8 pixels for '%s'");

				int x = 0;
				int y = 0;

				for(int i = 0;i < hdr.width * hdr.height;i++) {
					unsigned char color_index = pixels[i].colorMapIndex;

					colorMapEntryA8 px = palette[color_index];

					pixel(x, (hdr.height - y) - 1, px.red, 0);
					pixel(x, (hdr.height - y) - 1, px.green, 1);
					pixel(x, (hdr.height - y) - 1, px.blue, 2);
					pixel(x, (hdr.height - y) - 1, px.alpha, 3);

					x++;
					if(x >= hdr.width) { x = 0; y++; }
				}
				break;
			}
			//P4_MC, PA4_MC
			case 15:
			case 16: {
				struct colorMapEntryA8 palette[16];
				if (!file->read(palette)) return fail("Failed to read P4_MC/PA4_MC color palette for '%s'");

				//each Pixel8 here is actually two palette entries, 4 bits each
				struct Pixel8* pixels = new Pixel8[(hdr.width * hdr.height) / 2];
				if (!file->read_data(pixels, ((hdr.width * hdr.height) / 2) * sizeof(Pixel8))) return fail("Failed to read P4_MC/PA4_MC pixels for '%s'");

				int x = 0;
				int y = 0;

				for(int i = 0;i < (hdr.width * hdr.height) / 2;i++) {
					nibble_access color_index_2;
					color_index_2.value = pixels[i].colorMapIndex;

					colorMapEntryA8 px1 = palette[color_index_2.nibble1];
					colorMapEntryA8 px2 = palette[color_index_2.nibble2];

					pixel(x, (hdr.height - y) - 1, px1.red, 0);
					pixel(x, (hdr.height - y) - 1, px1.green, 1);
					pixel(x, (hdr.height - y) - 1, px1.blue, 2);
					pixel(x, (hdr.height - y) - 1, px1.alpha, 3);
					x++;
					if(x >= hdr.width) { x = 0; y++; }

					pixel(x, (hdr.height - y) - 1, px2.red, 0);
					pixel(x, (hdr.height - y) - 1, px2.green, 1);
					pixel(x, (hdr.height - y) - 1, px2.blue, 2);
					pixel(x, (hdr.height - y) - 1, px2.alpha, 3);
					x++;
					if(x >= hdr.width) { x = 0; y++; }
				}

				delete [] pixels;
				break;
			}
			//RGB888
			case 17: {
				Pixel888* pixels = new Pixel888[hdr.width * hdr.height];
				if (!file->read_data(pixels, hdr.width * hdr.height * sizeof(Pixel888))) return fail("Failed to read RGB888 pixels for '%s'");

				for(int x = 0;x < hdr.width;x++) {
					for(int y = 0;y < hdr.height;y++) {
						Pixel888 px = pixels[x + (y * hdr.height)];
						pixel(x, y, px.red, 0);
						pixel(x, y, px.green, 1);
						pixel(x, y, px.blue, 2);
						pixel(x, y, 255, 3);
					}
				}
				break;
			}
			//RGBA8888
			case 18: {
				Pixel8888* pixels = new Pixel8888[hdr.width * hdr.height];
				if (!file->read_data(pixels, hdr.width * hdr.height * sizeof(Pixel8888))) return fail("Failed to read RGBA8888 pixels for '%s'");

				for(int x = 0;x < hdr.width;x++) {
					for(int y = 0;y < hdr.height;y++) {
						Pixel8888 px = pixels[x + (y * hdr.height)];
						pixel(x, y, px.red, 0);
						pixel(x, y, px.green, 1);
						pixel(x, y, px.blue, 2);
						pixel(x, y, px.alpha, 3);
					}
				}
				break;
			}
		};

		return true;
	}
};