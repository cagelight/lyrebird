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
	
	sample_vector_t interleve(sample_vector_t const & a, sample_vector_t const & b);
	std::vector<uint8_t> compress_8(sample_vector_t const & data);
	std::vector<int16_t> compress_16(sample_vector_t const & data);
	
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
		
		virtual void fill_buffer(sample_vector_t & data) override {
			
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
	private:
		sample_t c = 0;
	};
	
	// ================================================================
}
