#include "computation.h"

#include <iostream>
#include <iterator>
#include <cmath>
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

class GPUStuff
{
public:
	static GPUStuff& getInstance()
	{
		static GPUStuff instance;
		return instance;
	}

private:
	GPUStuff();

	std::vector<cl::Platform> all_platforms;
	cl::Platform default_platform;
	std::vector<cl::Device> all_devices;
	cl::Device default_device;
	cl::Context context;
	cl::Program::Sources sources;
	std::string kernel_code;
	cl::Program program;

	GPUStuff(GPUStuff const&) = delete;
	void operator=(GPUStuff const&) = delete;

	friend std::array<float, VARIATIONS_SIZE> compute(const modEntry& info, float* compared);
};

GPUStuff::GPUStuff() {
	static_assert(pow(3, PARAMETRES) == VARIATIONS_SIZE, "VARIATIONS_SIZE MUST BE EQUAL TO 3 ^ PARAMETRES");
	cl::Platform::get(&all_platforms);
	if(all_platforms.size()==0){
		std::cout<<" No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	default_platform = all_platforms[0];
	std::cout << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<< std::endl;

	//get default device of the default platform
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if(all_devices.size() == 0){
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	default_device = all_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << std::endl;

	context = cl::Context({default_device});

	// kernel calculates for each element C=A+B
	kernel_code =
		#include "kernel_program.cl"
			;
	sources.push_back({kernel_code.c_str(), kernel_code.length()});

	program = cl::Program(context,sources);
	if(program.build({default_device}) != CL_SUCCESS){
		std::cout<< " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		exit(1);
	}
	std::cerr << "Compiled successfully" << std::endl;
}

std::array<float, VARIATIONS_SIZE> compute(const modEntry& info, float* compared) {
	GPUStuff& c = GPUStuff::getInstance();

	// create buffers on the device
	cl::Buffer input_buffer(c.context, CL_MEM_READ_WRITE, sizeof(modEntry));
	cl::Buffer comparing(c.context, CL_MEM_READ_WRITE, sizeof(float) * STEPS);
	cl::Buffer output_buffer(c.context, CL_MEM_READ_WRITE, sizeof(float) * VARIATIONS_SIZE);

	//create queue to which we will push commands for the device.
	cl::CommandQueue queue(c.context, c.default_device);

	//write arrays A and B to the device
	queue.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, sizeof(modEntry), &info);
	queue.enqueueWriteBuffer(comparing, CL_TRUE, 0, sizeof(float) * STEPS, compared);

	//alternative way to run the kernel
	std::cerr << "Sending data " << getTimestamp() << std::endl;;
	cl::Kernel kernelProgram = cl::Kernel(c.program, "compute");
   kernelProgram.setArg(0, input_buffer);
   kernelProgram.setArg(1, comparing);
   kernelProgram.setArg(2, output_buffer);
   //std::cerr << "Data Sent " << getTimestamp() << std::endl;
   queue.enqueueNDRangeKernel(kernelProgram, cl::NullRange, cl::NDRange(VARIATIONS_SIZE), cl::NullRange);
   std::cerr << "Enqueued " << getTimestamp() << std::endl;;
   queue.finish();
   std::cerr << "Done " << getTimestamp() << std::endl;;

	std::array<float, VARIATIONS_SIZE> output;
	//read result C from the device to array C
	queue.enqueueReadBuffer(output_buffer, CL_TRUE, 0, sizeof(float) * VARIATIONS_SIZE, &(output[0]));

//	std::cerr << "Got output\n";
//	std::cerr << " result: \n";
//	for(unsigned int i = 0; i < VARIATIONS_SIZE; i++){
//		std::cerr << output[i] << " ";
//	}

	return output;
}

std::array<float, STEPS> computeCPU(entry& given) {
	float diffusing[WIDTH];
	float quantity = given.quantity;
	std::array<float, STEPS> values;
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
		for (unsigned int j = 0; j < WIDTH; j++) diffusing[j] = diffusing_next[j];
		values[temperature] = left + given.constantFactor + given.linearFactor * temperature;
	}
	return values;
}
