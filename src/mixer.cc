#include "lyrebird/lyrebird.hh"

using namespace lyrebird;

void mixer::fill_buffer(sample_vector_t & data) {
	
	std::fill(data.begin(), data.end(), 0);
	sample_vector_t temp = data;
	
	for (gen_ptr & gen : gens) {
		gen->fill_buffer(temp);
		for (size_t i = 0; i < data.size(); i++) data[i] += temp[i] / gens.size();
	}
}
