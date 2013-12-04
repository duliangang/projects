//#ifndef _DEFINE_ALLOCATOR_H
//#define _DEFINE_ALLOCATOR_H
//#include <memory>
//#include "apr/apr_root_pool.h"
//#include "../pool/classPool.h"
//
//typedef struct _PoolStruct
//{
//	void* pBlock;
//	apr_pool_t* pPool;
//}PoolStruct;
//
//
//template<class T>
//class user_defined_allocator : public std::allocator<T>
//{
//public:
//	typedef std::allocator<T> base_type;
//
//	// ����Ҫ���¶��壬���������� list, set, map ʹ��ʱ������ֻ�ܵ��� std::allocator
//	template<class Other>
//	struct rebind
//	{
//		typedef user_defined_allocator<Other> other;
//	};
//
//	// ���캯������ʵ��
//	user_defined_allocator() 
//	{
//		apr_pool_create(&m_pool,sRootPool->getRootPool());
//	}
//	~user_defined_allocator()
//	{
//		if(m_pool)
//			apr_pool_destroy(m_pool);
//		m_pool=NULL;
//	}
//	user_defined_allocator(user_defined_allocator<T> const& o) 
//	{
//		
//	}
//
//	user_defined_allocator<T>& operator=(user_defined_allocator<T> const&) { return (*this); }
//
//	// idiom: Coercion by Member Template
//	template<class Other>
//	user_defined_allocator(user_defined_allocator<Other> const&o) 
//	{
//		
//	}
//
//	// idiom: Coercion by Member Template
//	template<class Other>
//	user_defined_allocator<T>& operator=(user_defined_allocator<Other> const&) { return (*this); }
//
//	// �ڴ�ķ������ͷſ���ʵ��Ϊ�Զ�����㷨���滻�����弴��
//	
//	pointer allocate(size_type count) 
//	{
//		if(!m_pool)
//		{
//			apr_pool_create(&m_pool,sRootPool->getRootPool());
//		}
//
//		ps->pBlock =(pointer)apr_palloc(ps->pPool,count);
//		
//		printf("%p,%p,%p\n",ps,ps->pBlock,ps->pPool);
//		return (pointer)ps->pBlock;
//	}
//
//	void deallocate(pointer ptr, size_type count) 
//	{
//	  void* block=ptr;
//	}
//	apr_pool_t* m_pool;
//private:
//	
//	int count;
//	Pool_Mutex<PoolStruct> m_MemberPool;
//};
//#endif
