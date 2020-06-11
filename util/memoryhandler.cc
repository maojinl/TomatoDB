/*
 * memoryhandler.cc
 *
 *  Created on: Jul 19, 2013
 *      Author: mao
 */

#include "memoryhandler.h"

int MemoryHandler::InitHeap(void* iHeap, size_t Size) {
	int ret = 0;
	union {
		Sint64             	l;
		void*              	p;
		MemoryBHeader*		bh;
	} a0, a1;

    this->memory_usage_  = 0;  		// Used Memory Size
    this->mAllocBaseSize = Size;  	// AllocBase Size

	a0.p = iHeap;
    a1.l = this->MemoryAligned(a0.l);
	Size -= a1.l - a0.l;
	Size &= -MEMORY_ALIGN;

	this->mpAlloc   = a1.bh;
	this->mBlocks   = 1;
	this->mpHead    = a1.bh;
	this->mpTail    = (MemoryBHeader*)((char*)a1.p + Size);

	a1.bh->mSize  = Size;
	a1.bh->mpNext = a1.bh;
	a1.bh->mpPrev = a1.bh;

	return  ret;
}

void* MemoryHandler::AllocateAligned(size_t size) { 
  return Allocate(size, true);
}

void* MemoryHandler::Allocate(size_t size, bool aligned) {
	MemoryBHeader*  p = this->mpHead;
	MemoryBHeader*  pMin = nullptr;

	Sint64             Min = (Sint64)(~(0UL) >> 1);

	if (aligned) {
      size = this->MemoryAligned(size + mAlignedHeaderSize);
    }

	do {
		Sint64  Remain = p->mSize - size;

		if( Remain == 0 ){
			this->mpAlloc = p->mpNext;
			p->mSize = 0 - p->mSize;
			return  this->skip_header(p);
		}
		if( Remain > 0 ){
			if( Remain < Min ){
				Min = Remain;
				pMin = p;
				/* break; */
			}
		}
		p = p->mpNext;
	} while( p != this->mpHead );

	if (pMin == nullptr) {
          return nullptr;
	}

	if( Min <= this->mAlignedHeaderSize ){
		pMin->mSize = 0 - pMin->mSize;
		return  this->skip_header(pMin);
	}

	this->mBlocks++;
	p = (MemoryBHeader*)((char*)pMin + size);
	this->mpAlloc       = p;
	p->mSize             = Min;
	pMin->mSize          = 0 - ((Sint64)size);

	pMin->mpNext->mpPrev = p;
	p->mpNext            = pMin->mpNext;
	p->mpPrev            = pMin;
	pMin->mpNext         = p;

	memory_usage_.fetch_add(size, std::memory_order_relaxed);
	return  this->skip_header(pMin);
}

int MemoryHandler::Destroy(void* iFree) {
    int  ret = 0;

	MemoryBHeader*  pTarget;
	MemoryBHeader*  pSide;
	MemoryBHeader*  pAlloc;

	if (iFree == nullptr || iFree <= mpHead ||
            iFree >= mpHead + mAllocBaseSize) {
		ret = 0xfffffffe;
        return ret;
	}

	iFree = (char*)iFree - this->mAlignedHeaderSize;
	pTarget = (MemoryBHeader*)iFree;
	pTarget->mSize = 0 - pTarget->mSize;
    memory_usage_.fetch_sub(pTarget->mSize, std::memory_order_relaxed);
	pAlloc = this->mpAlloc;

	pSide = pTarget->mpNext;

	if( (pSide->mSize > 0) && (((char*)iFree + pTarget->mSize) == (char*)pSide) )
	{
		pTarget->mSize += pSide->mSize;
		pTarget->mpNext = pSide->mpNext;
		pSide->mpNext->mpPrev = pTarget;
		if( pAlloc == pSide ){
			pAlloc = pTarget;
		}
		this->mBlocks--;
	}

	pSide = pTarget->mpPrev;
	if( (pSide->mSize > 0) && (((char*)iFree - pSide->mSize) == (char*)pSide) )
	{
		pSide->mSize += pTarget->mSize;
		pSide->mpNext = pTarget->mpNext;
		pSide->mpNext->mpPrev = pSide;
		if( pAlloc == pTarget ){
			pAlloc = pSide;
		}
		this->mBlocks--;
	}
	this->mpAlloc = pAlloc;
	return  ret;
}

size_t MemoryHandler::Available() {
	const MemoryBHeader*  p = this->mpAlloc;
	Sint64                Max = 0;

	do {
		if( p->mSize > Max ){
			Max = p->mSize;
		}
		p = p->mpNext;
	} while( p != this->mpAlloc );

	if( Max == 0 ){
		return  Max;
	}
	else {
		return  (Max - this->mAlignedHeaderSize);
	}
}
