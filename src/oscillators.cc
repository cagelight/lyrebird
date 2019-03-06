#include <ctgmath>

#include <asterales/brassica.hh>

#include "lyrebird/lyrebird.hh"

using namespace lyrebird;

static constexpr sample_t pi = asterales::brassica::pi<sample_t>;

void lyrebird::sawtooth::fill_buffer(sample_vector_t & data) {
			
	sample_vector_t freqs;
	freqs.resize(data.size());
	if (frequency_modulator)
		frequency_modulator->fill_buffer(freqs);
	else
		std::fill(freqs.begin(), freqs.end(), frequency);
	
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = c;
		c += (freqs[i] / settings->sample_rate) * 2;
		while (c > 1) c -= 2;
	}
}

void lyrebird::square::fill_buffer(sample_vector_t & data) {
			
	sample_vector_t freqs;
	freqs.resize(data.size());
	if (frequency_modulator)
		frequency_modulator->fill_buffer(freqs);
	else
		std::fill(freqs.begin(), freqs.end(), frequency);
	
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = c > 0 ? 1 : -1;
		c += (freqs[i] / settings->sample_rate) * 2;
		while (c > 1) c -= 2;
	}
}

void lyrebird::triangle::fill_buffer(sample_vector_t & data) {
			
	sample_vector_t freqs;
	freqs.resize(data.size());
	if (frequency_modulator)
		frequency_modulator->fill_buffer(freqs);
	else
		std::fill(freqs.begin(), freqs.end(), frequency);
	
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = std::abs(c) - 1;
		c += (freqs[i] / settings->sample_rate) * 4;
		while (c > 2) c -= 4;
	}
}

void lyrebird::sine::fill_buffer(sample_vector_t & data) {
			
	sample_vector_t freqs;
	freqs.resize(data.size());
	if (frequency_modulator)
		frequency_modulator->fill_buffer(freqs);
	else
		std::fill(freqs.begin(), freqs.end(), frequency);
	
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = std::sin(c);
		c += (freqs[i] / settings->sample_rate) * pi * 2;
	}
	
	while (c > pi) c -= pi * 2;
}
