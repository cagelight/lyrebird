#include <random>
#include <algorithm>

#include <asterales/brassica.hh>
#include <asterales/noise.hh>

#include "lyrebird/lyrebird.hh"

using namespace lyrebird;

static constexpr sample_t pi = asterales::brassica::pi<sample_t>;

static std::default_random_engine rnd {};
static std::mt19937_64 rng { rnd() };

void white_noise::fill_buffer(sample_vector_t & data) {
	
	static std::uniform_real_distribution<sample_t> dist {-1.0, 1.0};
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = dist(rng);
	}
}

struct simplex::impl_t {
	sample_t x_c;
	sample_t y_c;
	sample_t z_c;
	asterales::simplex spx;
};

simplex::simplex(settings_ptr settings, sample_t frequency) : generator(settings), frequency(frequency), impl { std::make_unique<impl_t>() } {
	impl->spx.initialize(rng);
}

simplex::~simplex() {}

void simplex::fill_buffer(sample_vector_t & data) {
	
	sample_t radius = 1024;
	
	static std::uniform_real_distribution<sample_t> wander {0.5, 1.5};
	
	sample_t x_i = (frequency * wander(rng)) / radius;
	sample_t y_i = (frequency * wander(rng)) / radius;
	sample_t z_i = (frequency * wander(rng)) / radius;
	
	for(size_t i = 0; i < data.size(); i++) {
		data[i] = impl->spx.generate(std::sin(impl->x_c) * radius, std::sin(impl->y_c) * radius, std::sin(impl->z_c) * radius);
		
		impl->x_c += x_i / settings->sample_rate;
		impl->y_c += y_i / settings->sample_rate;
		impl->z_c += z_i / settings->sample_rate;
	}
	
	while (impl->x_c > pi) impl->x_c -= pi * 2;
	while (impl->y_c > pi) impl->y_c -= pi * 2;
	while (impl->z_c > pi) impl->z_c -= pi * 2;
	while (impl->x_c < -pi) impl->x_c += pi * 2;
	while (impl->y_c < -pi) impl->y_c += pi * 2;
	while (impl->z_c < -pi) impl->z_c += pi * 2;
}
