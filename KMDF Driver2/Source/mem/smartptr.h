#pragma once
//#include "memorymanager.h"
//
//template <typename T>
//struct SmartPtr
//{
//	T* data = nullptr;
//	SmartPtr(size_t len)
//	{
//		data = myAlloc<T>(sizeof(T) * len);
//	}
//	SmartPtr()
//	{
//		data = myAlloc<T>(sizeof(T));
//	}
//	~SmartPtr()
//	{
//		myFree(data);
//	}
//};