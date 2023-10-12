//Created Saturday, Jun 10, 2023 by Thomas Applegate © 2023

//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the “Software”),
//to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the Software
//is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
//OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "memstack.h"
#include <stdlib.h>
#include <string.h>

#define NUM_FRAMES 4096
#define DEFAULT_CAPACITY 16

struct block
{
	bool shouldFree;
	void* data;
	memstack_callbackVoid cbVoid;
	memstack_callbackPtr cbPtr;
};

struct frameInfo
{
	size_t size;
	size_t capacity;
	struct block* arr;
};

static struct frameInfo frames[NUM_FRAMES];
static int currentFrame = -1;

static const struct block NULL_BLOCK = {
	.shouldFree = false,
	.data = NULL,
	.cbVoid = NULL,
	.cbPtr = NULL
};

static bool realloc_frame(int16_t frameNum)
{
	//try to reallocate the array of pointers
	size_t newCapacity = 2 * frames[frameNum].capacity;
	void* a = realloc(frames[frameNum].arr, newCapacity * sizeof(struct block));
	if(a == NULL)
	{
		return false;
	}else{
		frames[frameNum].arr = a;
		frames[frameNum].capacity = newCapacity;
	}
}

static void free_current_frame()
{
	for(int i = frames[currentFrame].size - 1; i >= 0 ; i--)
	{
		struct memstack_loc loc = {.frameIndex = currentFrame, .framePos = i};
		memstack_free(loc);
	}
	free(frames[currentFrame].arr);
}

bool memstack_push()
{
	if(currentFrame == NUM_FRAMES - 1)
	{
		return false;
	}else{
		//try to allocate a new frame
		void* a = malloc(DEFAULT_CAPACITY * sizeof(struct block));
		if(a == NULL) return false;
		//make a new frame
		currentFrame++;
		frames[currentFrame].size = 0;
		frames[currentFrame].capacity = DEFAULT_CAPACITY;
		frames[currentFrame].arr = a;
		return true;
	}
}

void memstack_pop(unsigned int numFrames)
{
	while(numFrames > 0 && currentFrame >= 0)
	{
		free_current_frame();
		numFrames--;
		currentFrame--;
	}
}

void memstack_popAll()
{
	while(currentFrame >= 0)
	{
		free_current_frame();
		currentFrame--;
	}
}

void* memstack_malloc(size_t size, struct memstack_loc* loc, memstack_callbackPtr fn)
{
	if(currentFrame < 0) return NULL;
	
	if(frames[currentFrame].size == frames[currentFrame].capacity)
	{
		if(realloc_frame(currentFrame) == false) return NULL;
	}
	void* out = malloc(size);
	if(out == NULL) return NULL;
	//store the pointer in the next open position
	frames[currentFrame].arr[frames[currentFrame].size].data = out;
	frames[currentFrame].arr[frames[currentFrame].size].shouldFree = true;
	frames[currentFrame].arr[frames[currentFrame].size].cbPtr = fn;
	frames[currentFrame].arr[frames[currentFrame].size].cbVoid = NULL;
	if(loc != NULL)
	{
		loc->frameIndex = currentFrame;
		loc->framePos = frames[currentFrame].size;
	}
	frames[currentFrame].size++;
	return out;
}

void* memstack_calloc(size_t size, struct memstack_loc* loc, memstack_callbackPtr fn)
{
	void* out = memstack_malloc(size, loc, fn);
	if(out != NULL){ memset(out, 0, size); } //don't memset if memstack_malloc fails
	return out;
}

void* memstack_realloc(struct memstack_loc* loc, size_t newSize)
{
	if(loc == NULL) return memstack_malloc(newSize, loc, NULL);
	
	//make sure that loc does not refer to an invalid position
	if(loc->frameIndex > currentFrame) return NULL;
	if(loc->framePos >= frames[loc->frameIndex].size) return NULL;
	
	if(newSize == 0)
	{
		memstack_free(*loc);
		return NULL;
	}else
	{
		void* n = realloc(frames[loc->frameIndex].arr[loc->framePos].data, newSize);
		if(n == NULL)
		{
			return NULL;
		}else{
			frames[loc->frameIndex].arr[loc->framePos].data = n;
			return n;
		}
	}
}

void  memstack_free(struct memstack_loc loc)
{
	//make sure that loc does not refer to an invalid position
	if(loc.frameIndex > currentFrame) return;
	if(loc.framePos >= frames[loc.frameIndex].size) return;
	
	//run callbacks
	if(frames[loc.frameIndex].arr[loc.framePos].cbVoid != NULL)
	{
		frames[loc.frameIndex].arr[loc.framePos].cbVoid();
	}
	
	if(frames[loc.frameIndex].arr[loc.framePos].cbPtr != NULL)
	{
		frames[loc.frameIndex].arr[loc.framePos].cbPtr(frames[loc.frameIndex].arr[loc.framePos].data);
	}
	
	//free data if needed
	if(frames[loc.frameIndex].arr[loc.framePos].shouldFree && 
		frames[loc.frameIndex].arr[loc.framePos].data != NULL)
	{
		free(frames[loc.frameIndex].arr[loc.framePos].data);
	}

	frames[loc.frameIndex].arr[loc.framePos] = NULL_BLOCK;
}

bool memstack_lower(struct memstack_loc* loc, unsigned int numFrames)
{
	//make sure that loc does not refer to an invalid position
	if(loc->frameIndex > currentFrame) return false;
	if(loc->framePos >= frames[loc->frameIndex].size) return false;
	
	//compute new frame
	int16_t newFrame = loc->frameIndex - numFrames;
	if(newFrame < 0) newFrame = 0;
	
	//check if the new frame needs to be reallocated
	if(frames[newFrame].size == frames[newFrame].capacity)
	{
		if(realloc_frame(newFrame) == false) return false;
	}
	
	//get the block pointed to by loc
	struct block* b = &frames[loc->frameIndex].arr[loc->framePos];
	//move the block
	frames[newFrame].arr[frames[newFrame].size] = *b;
	//set the old block to NULL
	*b = NULL_BLOCK;
	//update the loc
	loc->frameIndex = newFrame;
	loc->framePos   = frames[newFrame].size;
	
	frames[newFrame].size++;
	
	return true;
}

bool memstack_registerPtr(void* ptr, memstack_callbackPtr fn, struct memstack_loc* loc)
{
	if(currentFrame < 0) return false;
	
	if(frames[currentFrame].size == frames[currentFrame].capacity)
	{
		if(realloc_frame(currentFrame) == false) { return false; }
	}
	//store the pointer in the next open position
	frames[currentFrame].arr[frames[currentFrame].size].data = ptr;
	frames[currentFrame].arr[frames[currentFrame].size].shouldFree = false;
	frames[currentFrame].arr[frames[currentFrame].size].cbPtr = fn;
	frames[currentFrame].arr[frames[currentFrame].size].cbVoid = NULL;
	if(loc != NULL){
		loc->frameIndex = currentFrame;
		loc->framePos = frames[currentFrame].size;
	}
	frames[currentFrame].size++;
	return true;
}

bool memstack_registerVoid(memstack_callbackVoid fn, struct memstack_loc* loc)
{
	if(currentFrame < 0) return false;
	
	if(frames[currentFrame].size == frames[currentFrame].capacity)
	{
		if(realloc_frame(currentFrame) == false) { return false; }
	}
	//store the pointer in the next open position
	frames[currentFrame].arr[frames[currentFrame].size].data = NULL;
	frames[currentFrame].arr[frames[currentFrame].size].shouldFree = false;
	frames[currentFrame].arr[frames[currentFrame].size].cbPtr = NULL;
	frames[currentFrame].arr[frames[currentFrame].size].cbVoid = fn;
	if(loc != NULL){
		loc->frameIndex = currentFrame;
		loc->framePos = frames[currentFrame].size;
	}
	frames[currentFrame].size++;
	return true;
}

void memstack_registerLoc(struct memstack_loc loc, memstack_callbackPtr fn)
{
	//make sure that loc does not refer to an invalid position
	if(loc.frameIndex > currentFrame) return;
	if(loc.framePos >= frames[loc.frameIndex].size) return;
	
	frames[loc.frameIndex].arr[loc.framePos].cbPtr = fn;
}

void memstack_registerLocVoid(struct memstack_loc loc, memstack_callbackVoid fn)
{
	//make sure that loc does not refer to an invalid position
	if(loc.frameIndex > currentFrame) return;
	if(loc.framePos >= frames[loc.frameIndex].size) return;
	
	frames[loc.frameIndex].arr[loc.framePos].cbVoid = fn;
}

void memstack_unregister(struct memstack_loc loc)
{
	//make sure that loc does not refer to an invalid position
	if(loc.frameIndex > currentFrame) return;
	if(loc.framePos >= frames[loc.frameIndex].size) return;
	
	frames[loc.frameIndex].arr[loc.framePos].cbPtr = NULL;
	frames[loc.frameIndex].arr[loc.framePos].cbVoid = NULL;
}
