template<class Type>
class auto_ptr
{
public:
	auto_ptr():m_pCount(new int(1)),m_pT(NULL)
	{
		
	}
	auto_ptr(Type* pT):m_pCount(new int(1)),m_pT(NULL)
	{
		m_pT=pT;
	}
	auto_ptr(const auto_ptr<Type> & other_ptr):m_pCount(NULL),m_pT(NULL)
	{
		m_pT=other_ptr.m_pT;
		m_pCount=other_ptr.m_pCount;
		++(*m_pCount);
	}
	
	~auto_ptr()
	{
		if((--(*m_pCount))==0)
		{
			delete m_pT;
			delete m_pCount;
			m_pT=NULL;
			m_pCount=NULL;
		}
	}
	void  setPtr(Type* pT)
	{
		if((--(*m_pCount))==0)
		{
			delete m_pT;
			delete m_pCount;
			m_pT=NULL;
			m_pCount=NULL;
		}
		m_pCount=new int(1);
		m_pT=pT;
	}

	auto_ptr<Type>& operator=(auto_ptr<Type> & other_ptr)
	{
		if(other_ptr.m_pT==m_pT)return other_ptr;

		if((--(*m_pCount))==0)
		{
			delete m_pT;
			delete m_pCount;
			m_pT=NULL;
			m_pCount=NULL;
		}

		m_pT=other_ptr.m_pT;
		m_pCount=other_ptr.m_pCount;
		++(*m_pCount);
		return other_ptr;
	}
	Type& operator!()
	{
		return !m_pT;
	}
	Type& operator==(const auto_ptr<Type>& other_autopt)
	{
		return m_pT==other_autopt.m_pT;
	}

	Type* operator->()
	{
		return m_pT;
	}
	
	Type& operator*()
	{
		return *m_pT;
	}
	
private:
	Type* m_pT;
	int* m_pCount;

};