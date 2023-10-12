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
#include <stddef.h>

//typedef the function pointers used by the library
typedef void (*memstack_callbackVoid)(void);
typedef void (*memstack_callbackPtr)(void*);

//represents the location of the pointer held internally
//must be used to realloc or free prematurely
struct memstack_loc
{
	int frameIndex;
	size_t   framePos;
};

//Push a new frame onto the stack
//Returns true if a new frame was able to be pushed false otherwise
bool memstack_push();

//Pops at most numFrames and frees all pointers contained within them
//Will not double free something that has been freed manually by memstack_free
void memstack_pop(unsigned int numFrames);

//Pops all frames and frees all pointers contained within them
//Will not double free something that has been freed manually with memstack_free
void memstack_popAll();

//Tries to allocate data of size. If successful returns a pointer to the
//allocated memory and returns by reference the memstack_loc of the internal
//data used to keep track of the memory. If you do not wish to use the
//memstack_loc simply pass in NULL.
//If allocation is unsuccessful then NULL will be returned
//You can pass a function that takes a void pointer to run code when the data is
//freed, or pass NULL if you don't want to
void* memstack_malloc(size_t size, struct memstack_loc* loc, memstack_callbackPtr fn);

//Tries to allocate data of size and clear it. If successful returns a pointer
//to the allocated memory and returns by reference the memstack_loc
//of the internal data used to keep track of the memory.
//If you do not wish to use the memstack_loc simply pass in NULL.
//If allocation is unsuccessful then NULL will be returned
//You can pass a function that takes a void pointer to run code when the data is
//freed, or pass NULL if you don't want to
void* memstack_calloc(size_t size, struct memstack_loc* loc, memstack_callbackPtr fn);

//Tries to reallocate data referenced by the provided memstack_loc
//If successful returns the pointer to the reallocated data, otherwise returns
//NULL. If newSize is 0 the data will be freed and NULL returned.
//If loc is NULL will allocate newSize bytes with memstack_malloc
//If loc does not refer to data previously allocated then nothing will happen
void* memstack_realloc(struct memstack_loc* loc, size_t newSize);

//Manually frees data previously obtained from memstack_malloc
//If loc does not refer to data previously allocated then nothing will happen
//If the loc refers to a pointer with a callabck then the callback
//will be called.
//If the pointer refers to memory obtained externally, then
//it will not be freed
void  memstack_free(struct memstack_loc loc);

//Lower a previously allocated block to at most num frames down.
//If there is an error then the block will not be lowered.
//returns true on success and the new location by reference
//returns false on failure and does not move the block
bool memstack_lower(struct memstack_loc* loc, unsigned int numFrames);

//register a function to be called with an externally obtained pointer when the
//stack frame is popped. Returns true if successful false otherwise.
//Also returns a memstack_loc by reference referring to the registered callback
//use memstack_lower to lower the callback in the stack or memstack_free
//to prematurely call it.
bool memstack_registerPtr(void* ptr, memstack_callbackPtr fn, struct memstack_loc* loc);

//register a function that takes no arguments to be called when the stack frame
//is popped. This callback will not be associated with any data.
//On success returns true and the loc associated with the callback by reference
//else returns false
bool memstack_registerVoid(memstack_callbackVoid fn, struct memstack_loc* loc);

//register a function to be called with a pointer obtained by memstack_malloc or
//memstack_calloc upon freeing it. You can also pass this function into memstack_malloc
//or memstack_calloc directly, but this can be used to register or change the
//function after the fact.
//This will overwrite any previously registered function for that block
void memstack_registerLoc(struct memstack_loc loc, memstack_callbackPtr fn);

//register a function that takes no arguments to be called when
//the block referenced by loc is freed
//This will overwrite any previously registered function for that block
void memstack_registerLocVoid(struct memstack_loc loc, memstack_callbackVoid fn);

//unregister any callbacks assigned to the block referenced by the provided loc
void memstack_unregister(struct memstack_loc loc);

//get the pointer referenced by the provided loc
void* memstack_getPtr(struct memstack_loc loc);

//find the loc associated with the given pointer
//return true if found and false otherwise
//stores the loc by reference
//Note: this is slow as it has to search every saved record until it finds a match
//if possible save the loc when allocating
bool memstack_getLoc(void* ptr, struct memstack_loc* loc);

#endif //MEMSTACK_H
