#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#define SUCCESS 0
#define FAILURE 1
#define MAX_ARRAY 20
using namespace std;

/* convert the kernel file into a string */
int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout << "Error: failed to open file\n:" << filename << endl;
	return FAILURE;
}

int main(int argc, char* argv[])
{
	

	/*Step1: Lấy thông tin platform*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	/*nếu có nhiều platform, chọn cái đầu tiên*/
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:lay thong tin device, neu gpu khong co se mac dinh la cpu.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if (numDevices == 0)	//khong co gpu
	{
		cout << "No GPU device available." << endl;
		cout << "Choose CPU as default device." << endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	}


	/*Step 3: tao context.*/
	cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

	/*Step 4: tao command queue voi context*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	/*Step 5: tao doi tuong program */
	const char *filename = "HelloWorld_Kernel.cl";
	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);

	/*Step 6: build đối tượng program. */
	status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);

	/*Step 7: Khởi tạo đối tượng đầu vào, đầu ra cho host và tạo buffer cho kernel*/
	const char* input = "GdkknVnqkc";
	const size_t strlength = strlen(input);
	cout << "input string:" << endl;
	cout << input << endl;
	int len[] = {strlength};
	char *output = (char*)malloc((strlength + 1)*MAX_ARRAY);
	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (strlength + 1) * sizeof(char), (void *)input, NULL);//Câu lệnh tích hợp vừa tạo vừa ghi dữ liệu vào buffer
	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, (strlength + 1)*MAX_ARRAY * sizeof(char), NULL, NULL);
	cl_mem strlengthBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(size_t), (void *)len, NULL);
	//tao doi tuong buffer cho cac thong tin work_item, work_group, so' work_group, so' dimension su dung
	cl_mem numidbuf, groupidbuf, localidbuf, dimenbuf;
	numidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t), NULL, NULL);
	dimenbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint), NULL, NULL);
	groupidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t)* MAX_ARRAY * (strlength + 1), NULL, NULL);
	localidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t)* MAX_ARRAY * (strlength + 1), NULL, NULL);

	/*Step 8: Create kernel object */
	//cl_kernel kernel = clCreateKernel(program, "helloworld", NULL);
	
	/*---------------kernel chay nhieu helloworld-----------*/
	cl_kernel kernel = clCreateKernel(program, "Nhieu_helloworld", NULL);
	

	/*Step 9: Sets Kernel arguments.*/
	
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);
	/*---------------kernel chay nhieu helloworld-----------*/
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&strlengthBuffer);
	status = clSetKernelArg(kernel, 3, sizeof(cl_mem), &numidbuf);
	status = clSetKernelArg(kernel, 4, sizeof(cl_mem), &groupidbuf);
	status = clSetKernelArg(kernel, 5, sizeof(cl_mem), &localidbuf);
	status = clSetKernelArg(kernel, 6, sizeof(cl_mem), &dimenbuf);

	/*Step 10: Running the kernel.*/
	size_t global_work_size = MAX_ARRAY*strlength;
	size_t local_work_size = strlength;
	status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &global_work_size, &strlength, 0, NULL, NULL);


	/*Step 11: Read the cout put back to host memory.*/
	/**/
	status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, (strlength+1)*MAX_ARRAY * sizeof(char), output, 0, NULL, NULL);

	// doc thong tin numgroup, group id, num group
	cl_uint dimen[1];
	size_t numid[1];
	size_t groupid[MAX_ARRAY* 11];
	size_t localid[MAX_ARRAY* 11];
	clEnqueueReadBuffer(commandQueue, numidbuf, CL_TRUE, 0, sizeof(size_t), numid, 0, NULL, NULL);
	clEnqueueReadBuffer(commandQueue, groupidbuf, CL_TRUE, 0, sizeof(size_t)* MAX_ARRAY * (strlength + 1), groupid, 0, NULL, NULL);
	clEnqueueReadBuffer(commandQueue, localidbuf, CL_TRUE, 0, sizeof(size_t)* MAX_ARRAY * (strlength + 1), localid, 0, NULL, NULL);
	clEnqueueReadBuffer(commandQueue, dimenbuf, CL_TRUE, 0, sizeof(cl_uint), dimen, 0, NULL, NULL);
	cout << "\noutput string:" << endl;
	cout << "Dang duoc chay voi " << numid[0] << " group tren " << dimen[0] << " dimension\n";
	for (int i = 0; i < MAX_ARRAY; i++)
	{
		for (int j = 0; j < strlength; j++){
			cout << output[i*strlength + j] << " dang duoc thuc thi tai group " << groupid[i*strlength + j] << " tai work item " << localid[i*strlength + j] << endl;
			if (j==strlength-1)
			{
				cout << endl;
			}
		}
	}
	

	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(kernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseMemObject(inputBuffer);		//Release mem object.
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.

	if (output != NULL)
	{
		free(output);
		output = NULL;
	}

	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}

	std::cout << "Passed!\n";
	return SUCCESS;
}
