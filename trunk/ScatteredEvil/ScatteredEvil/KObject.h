#pragma warning (disable:4291)

// Define private default constructor.
#define NO_DEFAULT_CONSTRUCTOR(cls) \
	protected: cls() {} public:

// Declare the base KObject class.
#define DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags) \
public: \
	/* Identification */ \
	enum {StaticClassFlags = TStaticFlags}; \
	private: static KClass PrivateStaticClass; public: \
	typedef TSuperClass Super;\
	typedef TClass ThisClass;\
	static KClass* StaticClass(void) \
		{ return &PrivateStaticClass; } \
	void* operator new(size_t, EInternal* Mem) \
		{ return (void *)Mem; }

// Declare a concrete class.
#define DECLARE_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags) \
	static void InternalConstructor(void* X) \
		{ new((EInternal *)X)TClass(); }

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags | CLASS_Abstract)

// Register a class at startup time.
#define IMPLEMENT_CLASS(TClass) \
	KClass TClass::PrivateStaticClass \
	( \
		sizeof(TClass), \
		TClass::StaticClassFlags, \
		TClass::Super::StaticClass(), \
		#TClass, \
		(void(*)(void*))TClass::InternalConstructor \
	); \
	KClass* autoclass##TClass = TClass::StaticClass();

// ENUMERATIONS ------------------------------------------------------------

//
// Internal enums.
//
enum ENativeConstructor		{EC_NativeConstructor};
enum EInternal				{EC_Internal};

//
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Abstract          = 0x00001,  // Class is abstract and can't be instantiated directly.
};

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//	KClass
//
//==========================================================================

class KClass
{
	friend class KObject;

private:
	// Internal class variables.
	size_t					ClassSize;
	unsigned int			ClassFlags;
	KClass*					ParentClass;
	KClass*					NextClass;			// Next object in class list.
	const char*				Name;				// Class name.
	void					(*ClassConstructor)(void*);

	static KClass			*GClasses;
public:
	// Accessors.
	const char *GetName(void) const
	{
		return Name;
	}

	static KClass *FindClass(const char *);
	static KClass *FindClassChecked(const char *);

	KClass(size_t ASize, unsigned int AClassFlags,
		KClass *AParent, const char *AName, void(*ACtor)(void*));

	bool IsChildOf(const KClass *SomeBaseClass) const
	{
		for (const KClass *c = this; c; c = c->GetSuperClass())
		{
			if (SomeBaseClass == c)
			{
				return true;
			}
		}
		return false;
	}

	KClass *GetSuperClass(void) const
	{
		return ParentClass;
	}
};

//==========================================================================
//
//	KObject
//
//==========================================================================

//
// The base class of all objects.
//
class KObject
{
	// Declarations.
	DECLARE_BASE_CLASS(KObject, KObject, CLASS_Abstract)

private:
	// Internal per-object variables.
	KClass*					Class;	  			// Class the object belongs to.

public:
	// Constructors.
	KObject();
	KObject(ENativeConstructor, KClass* AClass);
	static void InternalConstructor(void* X)
		{ new((EInternal*)X)KObject(); }

	// Destructors.
	virtual ~KObject();
	void operator delete(void* Object, size_t)
		{ gi.Z_Free(Object); }

	// KObject interface.
	virtual void Destroy(void);

	// Systemwide functions.
	static KObject *StaticSpawnObject(KClass *);

	// Functions.
	bool IsA(KClass *SomeBaseClass) const
	{
		for (const KClass *c = Class; c; c = c->GetSuperClass())
		{
			if (SomeBaseClass == c)
			{
				return true;
			}
		}
		return false;
	}

	// Accessors.
	KClass* GetClass(void) const
	{
		return Class;
	}
};

// Dynamically cast an object type-safely.
template<class T> T* Cast(KObject* Src)
{
	return Src && Src->IsA(T::StaticClass()) ? (T*)Src : NULL;
}
template<class T, class U> T* CastChecked(U* Src)
{
	if (!Src || !Src->IsA(T::StaticClass()))
		gi.Error("Cast to %s failed", T::StaticClass()->GetName());
	return (T*)Src;
}

template<class T> T* Spawn()
{
	return (T *)KObject::StaticSpawnObject(T::StaticClass());
}
