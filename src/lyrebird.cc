#include <ctgmath>

#include "lyrebird/lyrebird.hh"

using namespace lyrebird;

template <typename T>
inline constexpr T clamp(T val, T min, T max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

sample_vector_t lyrebird::interleave(sample_vector_t const & a, sample_vector_t const & b) {
	if (a.size() != b.size()) throw buffer_exception {};
	
	sample_vector_t ret;
	ret.resize(a.size() * 2);
	
	for(size_t i = 0; i < a.size(); i++) {
		ret[(i * 2) + 0] = a[i];
		ret[(i * 2) + 1] = b[i];
	}
	
	return ret;
}

void lyrebird::modulator::fill_buffer(sample_vector_t & data) {
	gen->fill_buffer(data);
	for (size_t i = 0; i < data.size(); i++) {
		data[i] = data[i] / 2 + 0.5;
		data[i] *= max - min;
		data[i] += min;
		data[i] = clamp<sample_t>(data[i], min, max);
	}
}

static inline constexpr uint8_t map_8(sample_t sample) {
	sample_t s = (sample * 128.0) + 127.0;
	int_fast64_t si = static_cast<int_fast64_t>(s);
	return clamp<int_fast64_t>(si, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
}

std::vector<uint8_t> lyrebird::compress_8(lyrebird::sample_vector_t const & data) {
	std::vector<uint8_t> ret;
	
	ret.resize(data.size());
	for (size_t i = 0; i < data.size(); i++) {
		ret[i] = map_8(data[i]);
	}
	
	return ret;
}

static inline constexpr int16_t map_16(sample_t sample) {
	sample_t s = (sample * 32768.0);
	int_fast64_t si = static_cast<int_fast64_t>(s);
	return clamp<int_fast64_t>(si, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
}

std::vector<int16_t> lyrebird::compress_16(lyrebird::sample_vector_t const & data) {
	std::vector<int16_t> ret;
	
	ret.resize(data.size());
	for (size_t i = 0; i < data.size(); i++) {
		ret[i] = map_16(data[i]);
	}
	
	return ret;
}
