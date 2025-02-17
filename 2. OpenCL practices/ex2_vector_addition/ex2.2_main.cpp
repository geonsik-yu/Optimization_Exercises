/*
 Title: OpenCL Vector Addition Example 2
 References:
	[1] Munshi, Aaftab, et al. OpenCL programming guide. Pearson Education, 2011.
	[2] B. Gaster, "Bgaster/opencl-book-samples". Github repository. N.p., 2011. Web. 30 June 2017
 Note:
	- Develop with OpenCL in OS X.
*/

#include <iostream>
#include <fstream>
#include <sstream>

#include <OpenCL/cl.h>

#define ARRAY_SIZE 2048 // A hard-coded number.

// Function 1. OpenCL Context.
// Create an OpenCL context on the first available platform.
cl_context CreateContext(){
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;
	
	// 1-1. Select an OpenCL platform to run on.  For this example, we simply choose the first available platform.
	// (*) Normally, you would query for all available platforms and select the most appropriate one.
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0){
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}
	
	// 1-2. Create an OpenCL context on the platform. Attempt to create a GPU-based context.
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)firstPlatformId, 0};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);
	// - If the attempt fails, try to create a CPU-based context.
	if (errNum != CL_SUCCESS){
		std::cout << "Could not create GPU context, trying CPU..." << std::endl;
		context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
		if (errNum != CL_SUCCESS){
			std::cerr << "Failed to create an OpenCL GPU or CPU context." << std::endl;
			return NULL;
		}
	}
	return context;
}


// Function 2. OpenCL Context.
// Create a command queue on the first device available on the context.
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device){
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;
	
	// 2-1. Get the size of the devices buffer
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	if (errNum != CL_SUCCESS){
		std::cerr << "Failed call to clGetContextInfo(...,GL_CONTEXT_DEVICES,...)";
		return NULL;
	}
	if (deviceBufferSize <= 0){
		std::cerr << "No devices available.";
		return NULL;
	}
	
	// 2-2. Allocate memory for the devices buffer.
	devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	if (errNum != CL_SUCCESS){
		delete [] devices;
		std::cerr << "Failed to get device IDs";
		return NULL;
	}
	// 2-3. Choose an availble device (first available device in this example).
	// (*) Normally, you would query for all available devices and select the most appropriate one.
	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (commandQueue == NULL){
		delete [] devices;
		std::cerr << "Failed to create commandQueue for device 0";
		return NULL;
	}
	
	*device = devices[0];
	delete [] devices;
	return commandQueue;
}


// Function 3. OpenCL Program.
// Create an OpenCL program from the kernel source file.
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName){
	cl_int errNum;
	cl_program program;
	
	std::ifstream kernelFile;
	kernelFile.open(fileName, std::ios::in);
	if (!kernelFile.is_open()){
		std::cerr << "Failed to open file for reading: " << fileName << std::endl;
		return NULL;
	}
	
	std::ostringstream oss;
	oss << kernelFile.rdbuf();
	
	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, NULL);
	if (program == NULL)
	{
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}
	
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS){
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}
	
	return program;
}

// Function 4. OpenCL Kernel Memory Objects.
bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float *b){
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a, NULL);
	memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, b, NULL);
	memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE, NULL, NULL);
	
	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL){
		std::cerr << "Error creating memory objects." << std::endl;
		return false;
	}
	return true;
}

// Function 5. Cleanup any created OpenCL resources.
void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3]){
	for (int i = 0; i < 3; i++)
		if (memObjects[i] != 0) clReleaseMemObject(memObjects[i]);
	if (commandQueue != 0) clReleaseCommandQueue(commandQueue);
	if (kernel != 0) clReleaseKernel(kernel);
	if (program != 0) clReleaseProgram(program);
	if (context != 0) clReleaseContext(context);
}


int main(int argc, char** argv){
	cl_context context = 0;
	cl_command_queue commandQueue = 0;
	cl_program program = 0;
	cl_device_id device = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[3] = { 0, 0, 0 };
	cl_int errNum;
	
	// 1. Context ----------------------------------------------------------
	context = CreateContext();
	if (context == NULL){
		std::cerr << "Failed to create OpenCL context." << std::endl;
		return 1;
	}

	// 2. Command Queue ----------------------------------------------------
	commandQueue = CreateCommandQueue(context, &device);
	if (commandQueue == NULL){
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	// 3. Program ----------------------------------------------------------
	program = CreateProgram(context, device, "ex2.2_vector_addition.cl");
	if (program == NULL){
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	// 4. Kernel -----------------------------------------------------------
	kernel = clCreateKernel(program, "my_vector_addition", NULL);
	if (kernel == NULL){
		std::cerr << "Failed to create kernel" << std::endl;
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	// 5. Kernel Memory Objects and Kernel Arguments -----------------------
	float result[ARRAY_SIZE];
	float a[ARRAY_SIZE];
	float b[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++){
		a[i] = (float)i;
		b[i] = (float)(i * 2);
	}
	
	if (!CreateMemObjects(context, memObjects, a, b)){
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
	if (errNum != CL_SUCCESS){
		std::cerr << "Error setting kernel arguments." << std::endl;
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	size_t globalWorkSize[1] = { ARRAY_SIZE };
	size_t localWorkSize[1] = { 1 };
	
	// 6. Queue Kernel -----------------------------------------------------
	errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (errNum != CL_SUCCESS){
		std::cerr << "Error queuing kernel for execution." << std::endl;
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	// 7. Read the output buffer back to the Host --------------------------
	errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0, ARRAY_SIZE * sizeof(float), result, 0, NULL, NULL);
	if (errNum != CL_SUCCESS){
		std::cerr << "Error reading result buffer." << std::endl;
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	
	// 8. Output the result buffer
	for (int i = 0; i < ARRAY_SIZE; i++)
		std::cout << result[i] << " ";
	std::cout << std::endl;
	std::cout << "Executed program succesfully." << std::endl;
	Cleanup(context, commandQueue, program, kernel, memObjects);
	
	return 0;
}
