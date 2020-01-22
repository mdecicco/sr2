#pragma once
#include <r2/config.h>

namespace r2 {
	class data_container;
};

namespace sr2 {
	class vag_file {
		public:
			vag_file();
			~vag_file();

			bool read(r2::data_container* file);
			bool save(const char* wavFile);

		protected:
			r2::i32 m_sampleRate;
			r2::u16* m_samples;
			r2::u64 m_sampleCount;
			r2::f32 m_duration;
	};
};

