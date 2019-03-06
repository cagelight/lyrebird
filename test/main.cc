#include <csignal>

#include <atomic>
#include <stdexcept>
#include <thread>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>

#include <asterales/strop.hh>

#include "../src/lyrebird/lyrebird.hh"

#define ALERR do { ALenum err; if ((err = alGetError()) != AL_NO_ERROR) throw std::runtime_error { asterales::strf("[%s] OpenAL error occurred, code (%i)", _as_here, err) }; } while (0);

static constexpr size_t sample_rate = 44100;
static constexpr size_t buffer_msec = 50;
static constexpr size_t buffer_width = 2;
static constexpr size_t buffer_samples = (sample_rate * buffer_msec) / 1000;
static constexpr size_t buffer_size = buffer_samples * buffer_width;

static_assert(buffer_width == 1 || buffer_width == 2);

static std::atomic_bool run_sem { true };

static void handle_interrupt(int) {
	if (!run_sem) std::terminate();
	run_sem.store(false);
}

struct test_generator {
	
	lyrebird::gen_ptr l, r;
	
	void fill(ALuint buffer) {
		
		lyrebird::sample_vector_t data_l, data_r;
		data_l.resize(buffer_samples);
		data_r.resize(buffer_samples);
		
		l->fill_buffer(data_l);
		r->fill_buffer(data_r);
		
		lyrebird::sample_vector_t idata = lyrebird::interleve(data_l, data_r);
		
		if constexpr (buffer_width == 1) {
			auto cdata = lyrebird::compress_8(idata);
			alBufferData(buffer, AL_FORMAT_STEREO8, cdata.data(), buffer_size * 2, sample_rate);
			ALERR
		}
		if constexpr (buffer_width == 2) {
			auto cdata = lyrebird::compress_16(idata);
			alBufferData(buffer, AL_FORMAT_STEREO16, cdata.data(), buffer_size * 2, sample_rate);
			ALERR
		}
	}
};
static void run_gen(test_generator & gen) {

	ALCdevice * device = alcOpenDevice(nullptr);
	if (!device) {
		throw std::runtime_error { "could not create OpenAL device" };
	}
	
	ALCcontext * context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context)) {
		throw std::runtime_error { "could not create OpenAL context" };
	}
	
	alGetError();
	
	ALuint source = 0;
	std::array<ALuint, 4> buffers;
	
	alGenSources(1, &source);
	ALERR
	alGenBuffers(buffers.size(), buffers.data());
	ALERR
	
	for (ALuint buf : buffers) {
		gen.fill(buf);
	}
	
	alSourceQueueBuffers(source, buffers.size(), buffers.data());
	alSourcePlay(source); ALERR
	
	while (run_sem) {
		
		ALint num_processed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &num_processed);
		ALERR
		
		for(; num_processed; num_processed--) {
			
			ALuint buffer = 0;
			alSourceUnqueueBuffers(source, 1, &buffer);
			ALERR
			gen.fill(buffer);
			alSourceQueueBuffers(source, 1, &buffer);
			ALERR
			
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	alDeleteSources(1, &source);
	alDeleteBuffers(buffers.size(), buffers.data());
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

inline static lyrebird::gen_ptr test_gen_l(lyrebird::settings_ptr settings) {
	auto osc = lyrebird::create<lyrebird::sawtooth>(settings); 
	osc->frequency_modulator = lyrebird::create_modulator<lyrebird::sawtooth>(80, 160, settings, 2);
	return osc;
}

inline static lyrebird::gen_ptr test_gen_r(lyrebird::settings_ptr settings) {
	auto osc = lyrebird::create<lyrebird::sawtooth>(settings); 
	osc->frequency_modulator = lyrebird::create_modulator<lyrebird::sawtooth>(40, 80, settings, 2);
	return osc;
}

int main() {
	signal(SIGINT, handle_interrupt);
	
	lyrebird::settings_ptr settings = lyrebird::create_settings();
	settings->sample_rate = sample_rate;
	
	test_generator gen {
		.l = test_gen_l(settings),
		.r = test_gen_r(settings)
	};
	
	run_gen(gen);
	return 0;
}
