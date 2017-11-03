#ifndef sawtooth_helpers_h
#define sawtooth_helpers_h

#include <algorithm>
#include <iterator>
#include <vector>
#include <random>
#include <functional>

//#define USE_RANDOM_POOL

//collection of functions to help port matlab code
//todo: replace with Boost or some other giant library if necessary
namespace Sawtooth {
	namespace Rng {

		class Random {
		private:
			std::mt19937_64 rng;
			
			//create a sequence of uniformly distributed random numbers
			std::vector<double> _rand(size_t size, double inclusive_min = 0.0,
				double exclusive_max = 1.0) {

				std::uniform_real_distribution<double> d(inclusive_min, exclusive_max);
				auto gen = std::bind(d, rng);
				std::vector<double> vec(size);
				std::generate(begin(vec), end(vec), gen);
				return vec;
			}

#ifdef USE_RANDOM_POOL // experimental optimization for now
			
			std::vector<double> randPool;
			//this must be larger than max density (preferably much larger)
			const size_t poolSize = (size_t)2000000;

#endif

		public:
			Random(unsigned long long seed) {
				rng = std::mt19937_64(seed);
#ifdef USE_RANDOM_POOL
				randPool = _rand(poolSize);
#endif
			}

			std::vector<double> rand(size_t size) {
#ifdef USE_RANDOM_POOL
				auto d = rng();
				size_t offset = d % (poolSize - size);
				return std::vector<double>(randPool.begin() + offset, randPool.begin() + offset + size);
#else
				return _rand(size);
#endif
			}

			//create a sequence of normally distributed random numbers
			std::vector<double> randnorm(size_t size, double mean, double sigma) {
				std::normal_distribution<double> d(mean, sigma);
				auto gen = std::bind(d, rng);
				std::vector<double> vec(size);
				std::generate(begin(vec), end(vec), gen);
				return vec;
			}

			//wrapper for standard shuffle to hide the rng engine
			template<class RandomIt>
			void shuffle(RandomIt first, RandomIt last) {
				std::shuffle(first, last, rng);
			}

		};
	}
}


#endif // !helpers_h