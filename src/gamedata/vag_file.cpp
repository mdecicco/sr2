#include <gamedata/vag_file.h>
#include <gamedata/utils/VAG_Unpack.h>
#include <r2/engine.h>
#include <stdlib.h>

using namespace r2;

namespace sr2 {
	vag_file::vag_file() : m_samples(nullptr), m_sampleRate(0), m_sampleCount(0), m_duration(0.0f) {
	}

	vag_file::~vag_file() {
		if (m_samples) delete[] m_samples;
		m_samples = nullptr;
	}

	bool vag_file::read(r2::data_container* file) {
		if (m_samples) delete[] m_samples;
		m_samples = nullptr;
		m_sampleRate = 0;
		m_sampleCount = 0;
		m_duration = 0.0f;

		auto fail = [this, file](const char* msg) {
			r2Error(msg, file->name().c_str());
			if (this->m_samples) delete[] this->m_samples;
			this->m_samples = nullptr;
			this->m_sampleRate = 0;
			this->m_sampleCount = 0;
			this->m_duration = 0.0f;
			return false;
		};
		file->set_position(0x10);
		if (!file->read(m_sampleRate)) return fail("Failed to read sample rate from '%s'");
		m_sampleRate = _byteswap_ulong(m_sampleRate);

		file->set_position(0);
		u8* data = new u8[file->size()];
		if (!file->read_data(data, file->size())) {
			delete[] data;
			return fail("Failed to read data from '%s'");
		}

		i32 size;
		VAG_Unpack(data, 64, file->size(), (byte**)&m_samples, &size);

		m_sampleCount = size / 2;
		m_duration = f32(m_sampleCount) / f32(m_sampleRate);
	}

	bool vag_file::save(const char* wavFile) {
		struct WAV_HDR {
			char  magic0[4];		// "RIFF"
			int   file_size;		// ...
			char  magic1[4];		// "WAVE"
			char  magic2[4];		// "fmt "
			int   hdr_size;			// 16
			short type;				// 1 for PCM (this is PCM!)
			short channels;			// 1 (VAG is always MONO, apparently)
			int   samp_rate;		// whatever the VAG says it is
			int   bytes_per_second; // bytes per second
			short bytes_per_sample; // bytes per sample
			short bits_per_sample;	// bits per sample
			char  magic3[4];		// "data"
			int   data_size;		// size of sample data
		};

		FILE* fp = fopen(wavFile, "wb");
		if (!fp) return false;

		WAV_HDR hdr;
		size_t h_sz = sizeof(hdr);
		memset(&hdr, 0, h_sz);
		memcpy(hdr.magic0, "RIFF", 4);
		hdr.file_size = 44 + (m_sampleCount * 2);
		memcpy(hdr.magic1, "WAVE", 4);
		memcpy(hdr.magic2, "fmt ", 4); //null is already there
		hdr.hdr_size = 16;
		hdr.type = 1;
		hdr.channels = 1;
		hdr.samp_rate = m_sampleRate;
		hdr.bytes_per_second = m_sampleRate * 2;
		hdr.bytes_per_sample = 2;
		hdr.bits_per_sample = 16;
		memcpy(hdr.magic3, "data", 4);
		hdr.data_size = m_sampleCount * 2;

		fwrite(&hdr, h_sz, 1, fp);
		fwrite(m_samples, m_sampleCount * 2, 1, fp);
		fclose(fp);
	}
};
