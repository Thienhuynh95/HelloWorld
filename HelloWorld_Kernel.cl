#define MAX_ARRAY 20
// TODO: Add OpenCL kernel code here.
__kernel void helloworld(__global char* in, __global char* out)
{
	int num = get_global_id(0);
	out[num] = in[num] + 1;	
}

__kernel void Nhieu_helloworld(__global char* in, __global char* out, __global int* strlen, global size_t *numid, global size_t *groupid, global size_t *localid, global uint *dimension)
{
	int num = get_global_id(0);
	int i = strlen[0] - (strlen[0] - num);
	out[num] = in[i] + 1;
	numid[0] = get_num_groups(0);
	groupid[num] = get_group_id(0);
	localid[num] = get_local_id(0);
	dimension[0] = get_work_dim();
}
