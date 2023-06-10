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

#define NUM_FRAMES 4096
#define DEFAULT_CAPACITY 64

struct frameInfo
{
	size_t size;
	size_t capacity;
	void** arr;
};

static struct frameInfo frames[NUM_FRAMES];
static int16_t currentFrame = -1;

bool memstack_push()
{
	if(currentFrame == NUM_FRAMES - 1)
	{
		return false;
	}else{
		//try to allocate a new frame
		void* a = malloc(DEFAULT_CAPACITY * sizeof(void*));
		if(a == NULL){
			return false;
		}
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
	while(numFrames > 0 && currentFrame >= 0){
		for(size_t i = 0; i < frames[currentFrame].size; i++)
		{
			free(frames[currentFrame].arr[i]);
		}
		free(frames[currentFrame].arr);
		numFrames--;
		currentFrame--;
	}
}

void memstack_popAll()
{
	while(currentFrame >= 0)
	{
		for(size_t i = 0; i < frames[currentFrame].size; i++)
		{
			free(frames[currentFrame].arr[i]);
		}
		free(frames[currentFrame].arr);
		currentFrame--;
	}
}

void* memstack_malloc(size_t size, struct memstack_loc* loc)
{
	if(currentFrame < 0){
		return NULL;
	}
	if(frames[currentFrame].size == frames[currentFrame].capacity)
	{
		//try to reallocate the array of pointers
		size_t newSize = 1.5 * frames[currentFrame].capacity;
		void* a = realloc(frames[currentFrame].arr, newSize);
		if(a == NULL)
		{
			return NULL;
		}else{
			frames[currentFrame].arr = a;
			frames[currentFrame].capacity = newSize;
		}
	}
	void* out = malloc(size);
	if(out == NULL) { return NULL; }
	//store the pointer in the next open position
	frames[currentFrame].arr[frames[currentFrame].size] = out;
	if(loc != NULL){
		loc->frameIndex = currentFrame;
		loc->framePos = frames[currentFrame].size;
	}
	frames[currentFrame].size++;
	return out;
}

void* memstack_realloc(struct memstack_loc loc, size_t newSize)
{
	if(newSize == 0)
	{
		memstack_free(loc);
		return NULL;
	}else
	{
		void* n = realloc(frames[loc.frameIndex].arr[loc.framePos], newSize);
		if(n == NULL)
		{
			return NULL;
		}else{
			frames[loc.frameIndex].arr[loc.framePos] = n;
			return n;
		}
	}
}

void  memstack_free(struct memstack_loc loc)
{
	free(frames[loc.frameIndex].arr[loc.framePos]);
}
