R"thedelimeter(

#define STEPS 1000
#define WIDTH 20

#define PARAMETRES 9
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

void kernel compute(global const struct modEntry* variations, global const float* compared, global float* matches) {
	
	struct entry given;
	int index = get_global_id(0);
	for (int i = PARAMETRES - 1; i >= 0; i--) {
		given.parametres[i] = variations->parametres[i][index % 3];
		index /= 3;
	}
	for (int i = PARAMETRES - 1; i < PARAMETRES + CONST_PARAMETRES; i++) {
		given.parametres[i] = variations->parametres[i][0];
	}
		
	int validCompares = 1; // Do not divide by zero
	float diffusing[WIDTH];
	float quantity = given.quantity;
	float match = 0;
	for (unsigned int j = 0; j < WIDTH; j++) diffusing[j] = 0;
	for (int temperature = 0; temperature <= STEPS; temperature++) {
		// Arrhenius' law, the constant is Boltzmann's constant in electronvolts
		float lost = quantity * given.rateConstant * pow(2.718281828, -1 * given.activationEnergy / (temperature + 273.15) / 8.6173324e-5) * given.secsPerKelvin;
		quantity -= lost;
		if (quantity < 0) quantity = 0;
		for (unsigned int j = WIDTH * given.minDepth; j < WIDTH * given.maxDepth; j++) {
			if (j == WIDTH) break;
			diffusing[WIDTH - j] += lost / WIDTH;
		}
		float left = 0; // What has diffused out of the sample
		float diffusing_next[WIDTH];
		for (unsigned int j = 0; j < WIDTH; j++) diffusing_next[j] = 0;
		for (unsigned int j = 0; j < WIDTH; j++) {
			float diffused = 0;
			for (int k = -1; k < WIDTH; k++) {
				if ((int)j == k) continue;
				float otherSide = (k == -1) ? 0 : diffusing[k];
				if (diffusing[j] - otherSide > 0) { // Do not do it twice
					float value = (diffusing[j] - diffused - otherSide) / ((j - k) * (j - k)) * given.secsPerKelvin * given.diffusion;
					if (k == -1) {
						left += value;
					}
					else diffusing_next[k] += value;
					diffused += value;
				}
			}
			diffusing_next[j] += diffusing[j] - diffused;
		}
		left += given.constantFactor + given.linearFactor * temperature;
		for (unsigned int j = 0; j < WIDTH; j++) diffusing[j] = diffusing_next[j];
		if (compared[temperature] == compared[temperature]) {
			match += (compared[temperature] - left) * (compared[temperature] - left);
			validCompares++;
		}
//		if (temperature == 600) {
//			matches[get_global_id(0)] = left;
//			return;
//		}
	}
	matches[get_global_id(0)] = (match / validCompares);
}

)thedelimeter"
