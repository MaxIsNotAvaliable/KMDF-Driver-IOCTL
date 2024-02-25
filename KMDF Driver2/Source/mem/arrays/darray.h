#pragma once
#include "..\memorymanager.h"

template<typename T>
class darray
{
protected:
	T* pData = nullptr;
	size_t size = 0;
	size_t bAllocated = 0;

	short memAllocated = 0;
	T* Alloc(size_t size)
	{
		this->memAllocated++;
		return myAlloc<T>(size);
	}
	void Release(void* pdata)
	{
		this->memAllocated--;
		return myFree(pdata);
	}

	bool IsOverflowBytes(size_t size)
	{
		return size >= this->bAllocated;
	}
	bool IsOverflowLen(size_t len)
	{
		return IsOverflowBytes(len * sizeof(T));
	}

	void CreateEmptySlot(size_t index)
	{
		if (index > this->size)
			return;

		size_t indexToByteSize = index * sizeof(T);

		if (indexToByteSize > this->bAllocated)
			Resize(this->size);
		memcpy(this->pData + indexToByteSize + 1, this->pData + indexToByteSize, this->bAllocated - indexToByteSize - sizeof(T));
	}
	void ShinkEmptySlots(size_t index, size_t itemCount = 1)
	{
		if (index > this->size)
			return;

		size_t indexToByteSize = index * sizeof(T);
		memcpy(this->pData + index, this->pData + index + itemCount, this->bAllocated - sizeof(T) * (index + itemCount));
	}

	void ctor(size_t len)
	{
		this->bAllocated = sizeof(T) * len;
		this->pData = Alloc(this->bAllocated);
		memset(this->pData, 0, this->bAllocated);
	}

public:
	void Resize(size_t newLen)
	{
		const size_t bNewSize = newLen * sizeof(T);

		T* orig = this->pData;
		T* temp = myAlloc<T>(bNewSize);

		memcpy(temp, orig, this->size * sizeof(T));

		this->pData = temp;
		this->bAllocated = bNewSize;

		myFree(orig);
	}

	bool Insert(size_t index, const T& data)
	{
		if (index > this->size)
			return false;
		/*index = this->size;*/

		this->size++;

		if (IsOverflowLen(this->size)) Resize(this->size);
		if (IsOverflowLen(index)) Resize(index);

		if (index + 1 < this->size)
			CreateEmptySlot(index);
		this->pData[index] = data;

		return true;
	}
	bool PushBack(const T& data)
	{
		return Insert(this->size, data);
	}
	bool PushForward(const T& data)
	{
		return Insert(0, data);
	}

	bool Remove(size_t index)
	{
		size_t maxIndex = this->bAllocated / sizeof(T);
		if (this->size < 1 || index >= maxIndex)
			return false;

		ShinkEmptySlots(index);
		this->size--;
		memset(this->pData + this->size, NULL, sizeof(T));
		return true;
	}
	bool PopBack()
	{
		return Remove(this->size);
	}
	bool PopFront()
	{
		return Remove(0);
	}

	bool Erase(size_t start, size_t count = 1)
	{
		if (!count)
			return false;

		if (IsOverflowLen(start) || IsOverflowLen(start + count))
			return false;

		ShinkEmptySlots(start, count);
		this->size -= count;
		memset(this->pData + this->size, NULL, sizeof(T) * count);
		return true;
	}

	bool Delete()
	{
		return Erase(0, this->size);
	}

	size_t Size()
	{
		return this->size;
	}

	T* Data()
	{
		return this->pData;
	}

	void Copy(const T* data, size_t len)
	{
		Release(this->pData);
		this->ctor(len);
		memcpy(this->pData, data, len);
	}

	T& operator[](size_t i)
	{
		//return IsOverflowLen(i) ? nullptr : this->pData[i];
		return this->pData[i];
	}
	darray& operator+=(const T& other)
	{
		this->PushBack(other);
		return *this;
	}
	darray& operator=(const T& other)
	{
		this->Delete();
		this->PushBack(other);
		return *this;
	}
	//darray& operator=(const T* other)
	//{
	//	Copy(other, strlen((const char*)(other)));
	//	return *this;
	//}

	T& Front()
	{
		return this->pData[this->size - 1];
	}
	T& Back()
	{
		return this->pData[0];
	}
	T& Get(size_t index)
	{
		//return IsOverflowLen(index) ? nullptr : this->pData[index];
		return this->pData[index];
	}

	darray() {}
	darray(size_t len)
	{
		this->ctor(len);
	}
	~darray()
	{
		Release(this->pData);
	}
};
