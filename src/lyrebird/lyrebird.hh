#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace lyrebird {
	
	struct buffer_exception {};
	
	using sample_t = double;
	using sample_vector_t = std::vector<sample_t>;
	
	struct settings {
		sample_t sample_rate;
	};
	
	using settings_ptr = std::shared_ptr<settings>;
	
	inline settings_ptr create_settings() { return std::make_shared<settings>(); }
	
	// ================================================================
	// UTILITY
	// ================================================================
	
	sample_vector_t interleave(sample_vector_t const & a, sample_vector_t const & b);
	std::vector<uint8_t> compress_8(sample_vector_t const & data);
	std::vector<int16_t> compress_16(sample_vector_t const & data);
	
	inline void attenuate(sample_vector_t & data, sample_t mul) {
		for (size_t i = 0; i < data.size(); i++) {
			data[i] = data[i] * mul;
		}
	}
	
	// ================================================================
	// GENERATOR
	// ================================================================

	struct generator {
		virtual void fill_buffer(sample_vector_t & data) = 0;
	protected:
		generator(settings_ptr settings) : settings(settings) {}
		settings_ptr settings;
	};
	
	using gen_ptr = std::unique_ptr<generator>;
	
	template <typename T, typename ... Args>
	inline std::unique_ptr<T> create(Args ... args) {
		return std::make_unique<T>(args ...);
	}
	
	// ================================================================
	// MODULATOR
	// ================================================================
	
	struct modulator {
		gen_ptr gen;
		sample_t min;
		sample_t max;
		
		void fill_buffer(sample_vector_t & data);
	};
	
	using modulator_ptr = std::unique_ptr<modulator>;
	
	inline modulator_ptr create_modulator(gen_ptr && gen, sample_t min, sample_t max) {
		return modulator_ptr { new modulator {
			.gen = std::forward<gen_ptr &&>(gen),
			.min = min,
			.max = max,
		}};
	}
	
	template <typename T, typename ... Args>
	inline modulator_ptr create_modulator(sample_t min, sample_t max, Args ... args) {
		return modulator_ptr { new modulator {
			.gen = lyrebird::create<T, Args ...>(args ...),
			.min = min,
			.max = max,
		}};
	}
	
	// ================================================================
	// MIXER
	// ================================================================
	
	struct mixer : public generator {
		
		mixer(settings_ptr settings) : generator(settings) {}
		
		virtual void fill_buffer(sample_vector_t & data) override;
		
		void add_generator(gen_ptr && gen) {
			gens.push_back(std::forward<gen_ptr &&>(gen));
		}
		
		template <typename T, typename ... Args>
		T & add_generator(Args ... args) {
			auto uptr = create<T>(settings, args ...);
			T & dref = *uptr;
			add_generator(std::move(uptr));
			return dref;
		}
		
	private:
		std::vector<gen_ptr> gens;
	};
	
	// ================================================================
	// BASIC OSCILLATION
	// ================================================================
	
	static constexpr sample_t default_frequency = 440;
	
	struct oscillator : generator {
		sample_t frequency;
		modulator_ptr frequency_modulator;
		
		oscillator(settings_ptr settings, sample_t freq = default_frequency) : generator(settings), frequency(freq) {}
	};
	
	struct sawtooth : oscillator {
		sawtooth(settings_ptr settings, sample_t freq = default_frequency) : oscillator(settings, freq) {}
		
		virtual void fill_buffer(sample_vector_t & data) override;
	private:
		sample_t c = 0;
	};
	
	struct square : oscillator {
		square(settings_ptr settings, sample_t freq = default_frequency) : oscillator(settings, freq) {}
		
		virtual void fill_buffer(sample_vector_t & data) override;
	private:
		sample_t c = 0;
	};
	
	struct triangle : oscillator {
		triangle(settings_ptr settings, sample_t freq = default_frequency) : oscillator(settings, freq) {}
		
		virtual void fill_buffer(sample_vector_t & data) override;
	private:
		sample_t c = 0;
	};
	
	struct sine : oscillator {
		sine(settings_ptr settings, sample_t freq = default_frequency) : oscillator(settings, freq) {}
		
		virtual void fill_buffer(sample_vector_t & data) override;
	private:
		sample_t c = 0;
	};
	
	// ================================================================
	// NOISE
	// ================================================================
	
	struct white_noise : generator {
		white_noise(settings_ptr settings) : generator(settings) {}
		virtual void fill_buffer(sample_vector_t & data) override;
	};
	
	struct simplex : public generator {
		sample_t frequency;
		simplex(settings_ptr settings, sample_t frequency = default_frequency);
		~simplex();
		virtual void fill_buffer(sample_vector_t & data) override;
	private:
		struct impl_t;
		std::unique_ptr<impl_t> impl;
	};
	
	// ================================================================
}
