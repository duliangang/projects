//×î´ó¶ÑÐòÁÐ
#include <vector>
template<class T>
class MaxHeap
{
public:
	MaxHeap(){}
	MaxHeap(T* valList,unsigned int nCount);

	void InsertVal(const T& val);
	void InsertVal(const T* pVal);

	//if replace return true
	//else return false 
	bool Replace(unsigned int pos,const T& val,bool append=false);

	//if  not have element return false;
	bool Pop(T& res);


	void clear();

	unsigned int Count()const;
protected:
	std::vector<T> m_ValList;

	unsigned int right(unsigned int pos)
	{
		return ((2*pos)<(m_ValList.size()))? (2*pos):0;
	}
	unsigned int left(unsigned int pos)
	{
		return ((2*pos+1)<m_ValList.size())? (2*pos+1):0;
	}

private:
	bool MaxHeapily(unsigned int pos);
};

template<class T>
bool MaxHeap<T>::MaxHeapily(unsigned int pos)
{
	if(pos>=m_ValList.size())
		return false;
	unsigned int pTemp=0;
	if(right(pos)!=0&&m_ValList[pos]<m_ValList[right(pos)])
		pTemp=right(pos);
	else
		pTemp=pos;
	if(left(pos)!=0&&m_ValList[pTemp]<m_ValList[left(pos)])
		pTemp=left(pos);
	if(pos!=pTemp)
	{
		std::swap(m_ValList[pos],m_ValList[pTemp]);
		MaxHeapily(pTemp);
	}
	return true;
}
template<class T>
MaxHeap<T>::MaxHeap(T* valList,unsigned int nCount)
{
	for (int i=0;i!=nCount;i++)
	{
		m_ValList.push_back(valList[i]);
	}
	for (int i=nCount/2-1;i>=0;i--)
	{
		MaxHeapily(i);
	}
}
template<class T>
void MaxHeap<T>::InsertVal(const T* pVal)
{
	Replace(m_ValList.size(),*val,true);
}
template<class T>
void MaxHeap<T>::InsertVal(const T& Val)
{
	Replace(m_ValList.size(),Val,true);
}
template<class T>
bool MaxHeap<T>::Replace(unsigned int pos,const T& val,bool append)
{
	if(!append&&pos>=m_ValList.size())
	{
		return false;
	}
	if (pos>=m_ValList.size())
	{
		pos=m_ValList.size();
		m_ValList.push_back(val);
	}
	else
	{
		m_ValList[pos]=val;
	}
	while(pos>0&&m_ValList[pos]>m_ValList[pos/2])
	{
		std::swap(m_ValList[pos],m_ValList[pos/2]);
		pos=pos/2;
	}
	return true;
}
template<class T>
bool MaxHeap<T>::Pop(T& val)
{
	if(m_ValList.empty())
		return false;
	val=m_ValList[0];
	m_ValList[0]=m_ValList[m_ValList.size()-1];
	m_ValList.erase(m_ValList.begin()+m_ValList.size()-1);
	MaxHeapily(0);
	return true;
}
template<class T>
void MaxHeap<T>::clear()
{
	return m_ValList.clear();
}
template<class T>
unsigned int MaxHeap<T>::Count()const
{
	return m_ValList.size();
}