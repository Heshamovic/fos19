#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

struct process{
	uint32 virtualAdress;
	int size;
}all_processes[1024];
int id;
unsigned int phyto[3000005]={0};

void* kmalloc(unsigned int size)
{
	int cnt = 0, bestPlace, bestSize = -1, lastfree;
		for(uint32 i = KERNEL_HEAP_START ; i < KERNEL_HEAP_MAX ; i += PAGE_SIZE)
		{
			uint32* ptr_table = NULL;
			get_page_table(ptr_page_directory, (void*) i, &ptr_table);

			if((ptr_table[PTX(i)] & PERM_PRESENT) > 0)
			{
				if(cnt > 0)
				{
					if((bestSize != -1 && cnt * PAGE_SIZE >= size && cnt < bestSize)|| (cnt * PAGE_SIZE >= size && bestSize == -1))
					{
						bestPlace = lastfree;
						bestSize =  cnt;
					}
					cnt = 0;
				}
			}
			else
			{
				if(cnt == 0)
					lastfree = i;
				cnt++;
			}
		}
		if(cnt > 0)
		{
			if((bestSize != -1 && cnt * PAGE_SIZE >= size && cnt < bestSize) || (cnt * PAGE_SIZE >= size && bestSize == -1))
			{
				bestPlace = lastfree;
				bestSize = cnt;
			}
		}
		if(bestSize == -1)
			return NULL;
		cnt = size / PAGE_SIZE;
		if(size % PAGE_SIZE > 0)
			cnt++;
		bool found=0;
		for(uint32 i = bestPlace, j = 0 ; j < cnt ; i += PAGE_SIZE, j++)
		{
			struct Frame_Info *f;
			int ret = allocate_frame(&f);
			if(ret != E_NO_MEM)
			{
				map_frame(ptr_page_directory, f, (void *) i , PERM_PRESENT|PERM_WRITEABLE);
				uint32 framenum = to_frame_number(f);
				phyto[framenum]=i;
			}
			else
			{
				/*for(uint32 k = bestPlace ; k < i ; k += PAGE_SIZE)
				{
					uint32* ptr_table = NULL;
					get_page_table(ptr_page_directory, (void*) k, &ptr_table);
					struct Frame_Info *f = get_frame_info(ptr_page_directory,(void *)k , &ptr_table);
					unmap_frame(ptr_page_directory,(void *)k);
					free_frame(f);
				}*/
				return NULL;
			}
		}

		all_processes[id].virtualAdress = bestPlace;
		all_processes[id++].size = cnt;
		uint32 temp=bestPlace;
		uint32 x=kheap_physical_address(temp);
		//phyto[x]=temp;

		return (void*)bestPlace;
	return NULL;
}

void kfree(void* virtual_address)
{
	uint32 address = (uint32)virtual_address;
	int tmp = 0, ii = -1;
	for(int i = 0 ; i < id ;i++)
	{
		if(all_processes[i].virtualAdress == address)
		{
			ii = i;
			tmp = all_processes[i].size;
			break;
		}
	}
	if(ii == -1)
		return;
	for(uint32 i = address, j = 0 ; j < tmp ; j++, i += PAGE_SIZE)
	{
		uint32 *ptr_table=NULL;
		struct Frame_Info *f = get_frame_info(ptr_page_directory,(void *)i,&ptr_table);
		uint32 framenum=to_frame_number(f);
		phyto[framenum]=0;
		unmap_frame(ptr_page_directory,(void *)i);

	}
	for(int i = ii ; i < id - 1 ; i++)
		all_processes[i] = all_processes[i + 1];
	id--;
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	/*for (uint32 i = KERNEL_HEAP_START ; i < KERNEL_HEAP_MAX ; i += PAGE_SIZE)
	{
		uint32 * ptr_table = NULL;
		get_page_table(ptr_page_directory,(void*)i ,&ptr_table);
		if (ptr_table != NULL)
		{
			if ((ptr_table[PTX(i)] & PERM_PRESENT) > 0)
			{
				if((ptr_table[PTX(i)] >> 12) == physical_address / PAGE_SIZE)
					return i;
			}
		}
		else
		{
			i += 1024 * 1024 * 4;
			i -= PAGE_SIZE;
		}
	}*/
	struct Frame_Info *f= to_frame_info(physical_address);
	uint32 framenum=to_frame_number(f);
	return phyto[framenum];
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32 * ptr_table ;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_table);
	if(ptr_table != NULL)
	{
		uint32 ent = ptr_table[PTX(virtual_address)];
		ent >>= 12;
		ent *= PAGE_SIZE;
		uint32 tmp = virtual_address;
		tmp &= 0x00000fff;
		ent += tmp;
		return ent;
	}
	return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
