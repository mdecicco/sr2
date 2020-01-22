#pragma once
#include <r2/config.h>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class tex_file {
		public:
			struct texHeader {
				unsigned short width;     // Width of image in pixels
				unsigned short height;    // Height of image in pixels
				unsigned short type;      // Type of image, see below
				unsigned short mips;      // Indicates how many mip maps this file contains
				unsigned short unknown;   // Unknown
				unsigned char  flags[4];  // Flags
			};
			struct colorMapEntryA8 {
				unsigned char blue;
				unsigned char green;
				unsigned char red;
				unsigned char alpha;
			};
			struct Pixel8 {
				unsigned char colorMapIndex;  // Offset in the colour map list
			};
			struct Pixel888 {
				unsigned char red;
				unsigned char green;
				unsigned char blue;
			};
			struct Pixel8888 {
				unsigned char red;
				unsigned char green;
				unsigned char blue;
				unsigned char alpha;
			};

			union nibble_access {
				struct {
					unsigned int nibble1 : 4;
					unsigned int nibble2 : 4;
				};
				unsigned char value;
			};

			tex_file();
			~tex_file();

			bool read(r2::data_container* file);

			inline r2::u8* data() const { return m_data; }
			inline r2::u32 width() const { return m_width; }
			inline r2::u32 height() const { return m_height; }

		protected:
			r2::u8* m_data;
			r2::u32 m_width;
			r2::u32 m_height;
	};
};