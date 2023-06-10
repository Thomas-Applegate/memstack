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

#ifndef MEMSTACK_H
#define MEMSTACK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//represents the location of the pointer held internally
//must be used to realloc or free prematurely
struct memstack_loc
{
	int16_t frameIndex;
	size_t   framePos;
};

//Push a new frame onto the stack
//Returns true if a new frame was able to be pushed false otherwise
bool memstack_push();

//Pops at most numFrames and frees all pointers contained within them
//Will not double free something that has been freed manually
void memstack_pop(unsigned int numFrames);

//Pops all frames and frees all pointers contained within them
//Will not double free something that has been freed manually
void memstack_popAll();

//Tries to allocate data of size. If sucessful returns a pointer to the
//allocated memory and returns by reference the memstack_loc of the internal
//data used to keep track of the memory. If you do not wish to use the
//memstack_loc simply pass in NULL.
//If allocation is unsucsessful then NULL will be returned
void* memstack_malloc(size_t size, struct memstack_loc* loc);

//Tries to reallocate data referenced by the provided memstack_loc
//If sucessful returns the pointer to the reallocated data, otherwise returns
//NULL. If newSize is 0 the data will be freed and NULL returned.
//This will not allocate memory for the first time you must use memstack_malloc
//to obtain memory before reallocing
void* memstack_realloc(struct memstack_loc loc, size_t newSize);

//Manually frees data previously obtained from memstack_malloc
//if loc does not refer to data previously allocated by memstack_malloc
//then the behavior is undefined
void  memstack_free(struct memstack_loc loc);


#endif //MEMSTACK_H
