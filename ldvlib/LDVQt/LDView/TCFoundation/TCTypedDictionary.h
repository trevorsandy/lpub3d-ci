#ifndef __TCTYPEDDICTIONARY_H__
#define __TCTYPEDDICTIONARY_H__

#include <TCFoundation/TCDictionary.h>
#include <TCFoundation/TCTypedObjectArray.h>

#ifdef WIN32
#pragma warning ( disable: 4710 )
#endif // WIN32

template <class Type> class TCTypedDictionary : public TCDictionary
{
	public:
		explicit TCTypedDictionary(int caseSensitive = 1)
			:TCDictionary(caseSensitive) {}

		void setObjectForKey(Type* object, char* key)
			{ TCDictionary::setObjectForKey(object, key); }
		Type* objectForKey(char* key)
			{ return (Type*)TCDictionary::objectForKey(key); }
		TCTypedObjectArray<Type>* allObjects(void)
		{ return (TCTypedObjectArray<Type>*)TCDictionary::allObjects(); }
	private:
		TCTypedDictionary(bool) {}
};

#endif // __TCTYPEDDICTIONARY_H__
