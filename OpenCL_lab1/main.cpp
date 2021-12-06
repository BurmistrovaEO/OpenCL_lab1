#include <CL/cl.h>
#include <iostream>

const int SIZE = sizeof(int)*3;

struct info{
	int block = NULL;
	int thread = NULL;
	int global_id = NULL;
};
const char * source = 
"void kernel get_kernel_info(global const int* b, global const int* th, global const int* id){"
"	id = get_global_id(0);"
"   th = get_local_id(0);"
"   b = floor(id/get_local_size(0))"	
"}";



int main() {
	cl_uint platformCount = 0;
	clGetPlatformIDs(0, nullptr, &platformCount);
	cl_platform_id* platform = new cl_platform_id[platformCount];
	clGetPlatformIDs(platformCount, platform, nullptr);
	for (cl_uint i = 0; i < platformCount; ++i) {
		char platformName[128];
		clGetPlatformInfo(platform[i], CL_PLATFORM_NAME,
			128, platformName, nullptr);
		std::cout << platformName << std::endl;
	}
	cl_context_properties properties[3] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
	};

	cl_context context = clCreateContextFromType(
		(NULL == platform) ? NULL : properties,
		CL_DEVICE_TYPE_GPU,
		NULL,
		NULL,
		NULL
	);

	size_t size = 0;

	clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		0,
		NULL,
		&size
	);

	cl_device_id device;
	
	
		cl_device_id* devices = (cl_device_id*)alloca(size);

		clGetContextInfo(
			context,
			CL_CONTEXT_DEVICES,
			size,
			devices,
			NULL
		);
		device = devices[0];
	

	cl_command_queue queue = clCreateCommandQueueWithProperties(
		context,
		device,
		0,
		NULL
	);


	size_t srclen[] = { strlen(source) };
	cl_program program = clCreateProgramWithSource(
		context,
		1,
		&source,
		srclen,
		NULL
	);
	clBuildProgram(program,
		1,
		&device,
		NULL,
		NULL,
		NULL
	);
	cl_kernel kernel = clCreateKernel(program,
		"show_info",
		NULL
	);

	int results[SIZE];
	results[0] = NULL;
	results[1] = NULL;
	results[2] = NULL;
	cl_mem output = clCreateBuffer(
		context,
		CL_MEM_HOST_WRITE_ONLY,
		sizeof(int) * SIZE,
		NULL,
		NULL
		);

	size_t count = SIZE;
	clSetKernelArg(
		kernel,
		1,
		sizeof(cl_mem),
		&output
	);

	clSetKernelArg(
		kernel,
		2,
		sizeof(unsigned int),
		&count
	);

	size_t group;
	clGetKernelWorkGroupInfo(
		kernel,
		device,
		CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(size_t),
		&group,
		NULL
	);
	clEnqueueNDRangeKernel(
		queue,
		kernel,
		1,
		NULL,
		&count,
		&group,
		0,
		NULL,
		NULL
	);

	clFinish(queue);

	clEnqueueReadBuffer(
		queue,
		output,
		CL_TRUE,
		0,
		sizeof(int)* count,
		results,
		0,
		NULL,
		NULL
	);
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

}