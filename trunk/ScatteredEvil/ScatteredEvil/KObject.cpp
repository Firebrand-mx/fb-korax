#include "h2def.h"
#include "KObject.h"

KClass			*KClass::GClasses;

//==========================================================================
//
//	KClass::KClass
//
//==========================================================================

KClass::KClass(size_t ASize, unsigned int AClassFlags, 
	KClass *AParent, const char *AName, void(*ACtor)(void*))
	: ClassSize(ASize), ClassFlags(AClassFlags), ParentClass(AParent), 
	Name(AName), ClassConstructor(ACtor)
{
	NextClass = GClasses;
	GClasses = this;
}

//==========================================================================
//
//	KClass::FindClass
//
//==========================================================================

KClass *KClass::FindClass(const char *AName)
{
	for (KClass *c = GClasses; c; c = c->NextClass)
	{
		if (!strcmp(c->GetName(), AName))
		{
			return c;
		}
	}
	return NULL;
}

//==========================================================================
//
//	KClass::FindClassChecked
//
//==========================================================================

KClass *KClass::FindClassChecked(const char *AName)
{
	KClass *c = FindClass(AName);
	if (!c)
	{
		gi.Error("KClass::FindClassChecked: Class %s not found", AName);
	}
	return c;
}

//**************************************************************************

// Register a class at startup time.
KClass KObject::PrivateStaticClass
(
	sizeof(KObject),
	KObject::StaticClassFlags,
	NULL,
	"KObject",
	(void(*)(void*))KObject::InternalConstructor
);
KClass* autoclassVObject = KObject::StaticClass();

//==========================================================================
//
//	KObject::KObject
//
//==========================================================================

KObject::KObject(void)
{
}

//==========================================================================
//
//	KObject::KObject
//
//==========================================================================

KObject::KObject(ENativeConstructor, KClass* AClass)
	: Class(AClass)
{
}

//==========================================================================
//
//	KObject::~KObject
//
//==========================================================================

KObject::~KObject(void)
{
}

//==========================================================================
//
//	KObject::StaticSpawnObject
//
//==========================================================================

KObject *KObject::StaticSpawnObject(KClass *AClass)
{
	KObject *Obj = (KObject *)gi.Z_Malloc(AClass->ClassSize, PU_STATIC, 0);
	memset(Obj, 0, AClass->ClassSize);
	AClass->ClassConstructor(Obj);
	Obj->Class = AClass;
	return Obj;
}

//==========================================================================
//
//	KObject::Destroy
//
//==========================================================================

void KObject::Destroy(void)
{
	delete this;
}
