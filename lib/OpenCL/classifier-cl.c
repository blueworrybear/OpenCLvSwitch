#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "classifier-cl.h"

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

char* classifier_cl(const struct miniflow *flow){
	cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    //cl_mem memobj = NULL;
    char* memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;
    
    char string[MEM_SIZE];
    FILE *fp;
    char fileName[] = "/usr/lib/CLvSwitch/classifier.cl";
    char *source_str;
    size_t source_size;
    
    //Load the source code containing the kernel.
    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        char* a= malloc(MEM_SIZE);
        strcpy(a,"sorry");
        return a;
        //exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    
    //Get platform and Device info
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
    
    //Create OpenCL context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    
    //Create OpenCL Queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    
    //Create Memory Buffer
    //memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE*sizeof(char), NULL, &ret);
    memobj = clSVMAlloc(context,CL_MEM_SVM_FINE_GRAIN_BUFFER & CL_MEM_SVM_ATOMICS,MEM_SIZE*sizeof(char),sizeof(long16));
    
    //Create kernel program from the source
    program = clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t*)&source_size, &ret);
    
    //Build Kernel program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    
    //Create OpenCL Kernel
    kernel = clCreateKernel(program, "hello", &ret);
    
    //Set OpenCL Kernel Parameters
    // ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);
    ret = clSetKernelArg(kernel, 0, sizeof(char*), (void *)&memobj);
    
    //Execute OpenCL Kernel
    ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    
    //Copy results from the memory buffer
    //ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0, MEM_SIZE*sizeof(char), string, 0, NULL, NULL);
    
    //Display result
    //puts(string);
    
    //Finalization
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(memobj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    
    free(source_str);

    return memobj;
}
