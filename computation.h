#ifndef COMPUTATION_H
#define COMPUTATION_H
#include <vector>
#include <array>
#include <chrono>

#define WIDTH 20
#define STEPS 1000
#define PARAMETRES 9
#define VARIATIONS_SIZE 19683 // 3 ^ 9, change when changing PARAMETRES
#define CONST_PARAMETRES 1

struct entry {
    union {
        struct {
            float activationEnergy;
            float width;
            float diffusion;
            float rateConstant;
            float minDepth;
            float maxDepth;
            float quantity;
			float constantFactor;
			float linearFactor;
			float secsPerKelvin;
        };
		float parametres[PARAMETRES + CONST_PARAMETRES];
    };
};

struct modEntry {
	float parametres[PARAMETRES + CONST_PARAMETRES][3];
};

inline long int getTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::array<float, VARIATIONS_SIZE> compute(const modEntry& info, float* compared);

std::array<float, STEPS> computeCPU(entry& given);

#endif // COMPUTATION_H
