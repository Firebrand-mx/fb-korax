template<class T> T Clamp(T val, T low, T high)
{
	return val < low ? low : val > high ? high : val;
}

//
// Base dynamic array.
//
class FArray
{
public:
	int Num(void) const
	{
		return ArrayNum;
	}
	void Add(int Count)
	{
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc();
		}
	}
	void Shrink(void)
	{
		if (ArrayMax != ArrayNum)
		{
			ArrayMax = ArrayNum;
			Realloc();
		}
	}
	void Empty(int Slack = 0)
	{
		ArrayNum = 0;
		ArrayMax = Slack;
		Realloc();
	}
	FArray(void) : Data(NULL), ArrayNum(0), ArrayMax(0)
	{}
	~FArray(void)
	{
		if (Data)
			free(Data);
		Data = NULL;
		ArrayNum = ArrayMax = 0;
	}
protected:
	void Realloc(void)
	{
		Data = realloc(Data, ArrayMax);
	}
	FArray(int InNum)
		: Data(NULL), ArrayNum(InNum), ArrayMax(InNum)
	{
		Realloc();
	}
	void *Data;
	int ArrayNum;
	int ArrayMax;
};

//==========================================================================
//
//	Dynamic strings.
//
//==========================================================================

//
// A dynamically sizeable string.
//
class FString : protected FArray
{
protected:
	char& operator[](int i)
	{
		return ((char*)Data)[i];
	}
	const char& operator[](int i) const
	{
		return ((char*)Data)[i];
	}
public:
	FString() : FArray()
	{}
	FString(const FString& Other): FArray(Other.ArrayNum)
	{
		if (ArrayNum)
			memcpy(&(*this)[0], &Other[0], ArrayNum);
	}
	FString(const char* In) : FArray(*In ? (strlen(In) + 1) : 0)
	{
		if (ArrayNum)
			memcpy(&(*this)[0], In, ArrayNum);
	}
	FString& operator = (const char* Other)
	{
		if (&(*this)[0] != Other)
		{
			ArrayNum = ArrayMax = *Other ? strlen(Other) + 1 : 0;
			Realloc();
			if (ArrayNum)
				memcpy(&(*this)[0], Other, ArrayNum);
		}
		return *this;
	}
	FString& operator = (const FString& Other)
	{
		if (this != &Other)
		{
			ArrayNum = ArrayMax = Other.Num();
			Realloc();
			if (ArrayNum)
				memcpy(&(*this)[0], *Other, ArrayNum);
		}
		return *this;
	}
	~FString()
	{
		FArray::Empty();
	}
	void Empty()
	{
		FArray::Empty();
	}
	void Shrink()
	{
		FArray::Shrink();
	}
	const char* operator * () const
	{
		return Num() ? &(*this)[0] : "";
	}
	operator bool() const
	{
		return Num() != 0;
	}
	FString& operator += (const char* Str)
	{
		if (ArrayNum)
		{
			int Index = ArrayNum - 1;
			Add(strlen(Str));
			strcpy(&(*this)[Index], Str);
		}
		else if (*Str)
		{
			Add(strlen(Str) + 1);
			strcpy(&(*this)[0], Str);
		}
		return *this;
	}
	FString& operator += (const FString& Str)
	{
		return operator += (*Str);
	}
	FString operator + (const char* Str)
	{
		return FString(*this) += Str;
	}
	FString operator + (const FString& Str)
	{
		return operator + (*Str);
	}
	bool operator == (const char* Other) const
	{
		return stricmp(**this, Other) == 0;
	}
	bool operator == (const FString& Other) const
	{
		return stricmp(**this, *Other) == 0;
	}
	bool operator != (const char* Other) const
	{
		return stricmp(**this, Other) != 0;
	}
	bool operator != (const FString& Other) const
	{
		return stricmp(**this, *Other) != 0;
	}
	int Len() const
	{
		return Num() ? Num() - 1 : 0;
	}
	FString Left(int Count) const
	{
		return FString(Clamp(Count, 0, Len()), **this);
	}
	FString LeftChop(int Count) const
	{
		return FString(Clamp(Len() - Count, 0, Len()), **this);
	}
	FString Right(int Count) const
	{
		return FString(**this + Len() - Clamp(Count, 0, Len()));
	}
	FString Mid(int Start, unsigned Count = DDMAXINT) const
	{
		unsigned End = Start + Count;
		Start = Clamp((unsigned)Start, (unsigned)0,     (unsigned)Len());
		End   = Clamp((unsigned)End,   (unsigned)Start, (unsigned)Len());
		return FString(End - Start, **this + Start);
	}
	int InStr(const char* SubStr, bool Right = false) const
	{
		if (!Right)
		{
			const char* Tmp = strstr(**this, SubStr);
			return Tmp ? (Tmp - **this) : -1;
		}
		else
		{
			for (int i = Len() - 1; i >= 0; i--)
			{
				int j;
				for (j = 0; SubStr[j]; j++)
					if ((*this)[i + j] != SubStr[j])
						break;
				if (!SubStr[j])
					return i;
			}
			return -1;
		}
	}
	int InStr(const FString& SubStr, bool Right = false) const
	{
		return InStr(*SubStr, Right);
	}
	bool Split(const FString& InS, FString* LeftS, FString* RightS, bool Right = false) const
	{
		int InPos = InStr(InS, Right);
		if (InPos < 0)
			return 0;
		if (LeftS)
			*LeftS = Left(InPos);
		if (RightS)
			*RightS = Mid(InPos + InS.Len());
		return 1;
	}
	FString Caps() const
	{
		FString New(**this);
		for (int i = 0; i < ArrayNum; i++)
			New[i] = toupper(New[i]);
		return New;
	}
	FString Locs() const
	{
		FString New(**this);
		for (int i = 0; i < ArrayNum; i++)
			New[i] = tolower(New[i]);
		return New;
	}
	//static FString Printf( const TCHAR* Fmt, ... );
private:
	FString(int InCount, const char* InSrc) 
		: FArray(InCount ? InCount + 1 : 0)
	{
		if (ArrayNum)
		{
			strncpy(&(*this)[0], InSrc, InCount);
			(*this)[InCount] = 0;
		}
	}
};
