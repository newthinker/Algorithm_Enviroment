//*******************************************************************
//	FArray.hpp�� FlexArray��
//	ģ����ƣ���̬����C++ģ���� 
//	version:1.0
//  ˵���t�?����CSArray 2.0�汾�Ļ���ǿ������ݲ�����ɾ�������棬
//	�������ṩ�Զ�̬�����֧�֣����ڴ��г�����ݷֿ��ţ�
//�������˴���ڴ�����룬ͬʱҲ����ͨ��˫��t�?ͬ���Ǳ������ṩ
//���˶��ڲ���ݵĿ����������������ݷ����ٶȡ�
//	��t�����ҪĿ���Ǵ���ݵĹ��?�ṩһ����ٵ���ݲ���ɾ��Ƚӿڣ���ʹ����ʱ
//	����һ�׺õĲ�������߳����ִ��Ч�ʣ�
//�������ƹ��ܣ�
//1 ��ʵ�����ڲ���Ϊ������ڴ�أ���ɾ��ѹ�����յ��ڴ�Ż��ڴ�ؿ����ظ����ñ����ڴ�Ƶ���ͷ����롣
//2 �ṩ���µ�ѹ������ڳ�Ŀռ�Żػ��ճأ�Ҳ���Զ�bɾ��������߻��ճ��ڴ档
//3 ��Ϊɾ�����ɱ���Ļ����ڴ�����ƶ����⣬����ڴ���ɾ��ʱЧ�ʻ�ܲ����ر��ṩ��һ����ɾ��Ĺ��ܱ������ֵ��0�ĵ�Ԫȫ��ɾ��
//��t���ִ��Ч�ʴ�����
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FARRAY_H__B1D40C22_2698_4202_921E_36D447EA4199__INCLUDED_)
#define AFX_FARRAY_H__B1D40C22_2698_4202_921E_36D447EA4199__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef ASSERT
#define ASSERT(a) void()
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

//( Flex Array )
template<class T>
class CFArray  
{
	typedef struct _FARRAYNODE{
		struct _FARRAYNODE *	pPrevNode;	//ǰһ�ڵ�
		T				   *	pData;		//�洢ʵ������������ݿ��ָ��	
		WORD					dwUsed;		//��ռ�õĿռ���,��ʼʱΪ0�����ֵΪ�ý��ĳ���
		struct _FARRAYNODE *	pNextNode;	//��һ�ڵ�
	}FARRAYNODE;

public:
	CFArray(WORD nGrowBy=5)
	{
		m_pCurNode=m_pHeadNode=m_pTailNode=NULL;
		m_nCurIndex=-1;
		m_nCount=0;
		m_nEmpty=0;
		m_nGrowBy=nGrowBy;
		m_byZipKey=10;	//Ĭ��ѹ��ֵΪ10%
		m_bNeedCompack = false;

		m_pEmptyHeadNode =NULL;	//ͷ�ڵ�
	    m_pEmptyEndNode  =NULL;
	}

	virtual ~CFArray()
	{
		RemoveAll();
	    //Free();
		//�￪���޸���2005 2 21
	}

	//******************************************
	//	name:Add
	//	function:������
	//	input:��T newElement-�����
	//  return: �ɹ�=��������,ʧ��=-1
	//	remark: 
	//******************************************
	int Add(T newElement)
	{
		if(m_nGrowBy==0)//
			return -1;
		if(m_pHeadNode==NULL)
		{//����ǿվͲ����һ��ڵ�
			if(!(m_pHeadNode=NewNode())) return -1;
			*m_pHeadNode->pData = newElement;
			m_pHeadNode->dwUsed = 1;
			m_nCurIndex = 0;
			m_nCount    = 1;
			m_nEmpty    = m_nGrowBy - 1;
			m_pCurNode  = m_pTailNode = m_pHeadNode;
			//��ɵ�һ��ڵ�
		}
		else
		{
			if(m_pTailNode->dwUsed==m_nGrowBy)
			{//������һ��ڵ��Ѿ�û�п���ʹ�����ٲ���һ��ڵ�
				FARRAYNODE *pNewNode=NewNode();
				if(!pNewNode) return -1;
				pNewNode->pPrevNode    = m_pTailNode;
				pNewNode->pNextNode    = NULL;// �￪�����2005 2 21 ԭȱ�ٴ����
				m_pTailNode->pNextNode = pNewNode;
				m_pTailNode = pNewNode;
				m_nEmpty   +=m_nGrowBy;
			}
			*(m_pTailNode->pData+m_pTailNode->dwUsed++)=newElement;
			//�￪���޸���2005 2 21 ԭ��Ϊm_pTailNode->pData[m_pTailNode->dwUsed++]=newElement;
			m_nCount++;
			m_nEmpty--;//��ע���ṹ�л��ж��ٿ��൥Ԫ����ʹ��
			m_nCurIndex = m_nCount-m_pTailNode->dwUsed;//ʹָ��ָ��ǰ�ڵ�
			m_pCurNode  = m_pTailNode;
		}
		return m_nCount-1;
	}

	//******************************************
	//	name:AddBatch
	//	function:��������
	//	input:��T *pElement-Դ����ָ��
	//			int count-�����С
	//  return: BOOL��TRUE���ɹ���FALSE��ʧ��
	//	remark: 
	//******************************************
	BOOL AddBatch(T *pElement,int count)
	{
		for(int i=0;i<count;i++)
			if(Add(pElement[i])==-1)
				return FALSE;
			return TRUE;
	}


	//******************************************
	//	name:Copy
	//	function:��ݸ���
	//	input:��CFArray & src-Դ��̬����
	//  return: 
	//	remark: ʹ��ǰ����ȷ��}���������ͬ���������
	//******************************************
	void Copy(CFArray &src )
	{
		T *pt;
		RemoveAll();
		int size=src.GetSize();
		SetSize(size);
		for(int i=0;i<m_nCount;i++)
		{
			pt=src.GetPtAt(i);
			SetAt(*pt,i);
		}
	}

	//******************************************
	//	name:GetAt
	//	function:��ȡ����ָ��λ�õ����
	//	input:��int index-ָ��λ��
	//  return: T ���
	//	remark: 
	//******************************************
	T GetAt(DWORD index)
	{
		FARRAYNODE      *pDest = NULL;
		ASSERT(index>=0&&index<m_nCount);
		pDest = GetDestSegEntry( index );
		return ((T *)pDest->pData)[index-m_nCurIndex];
	}
	
	//******************************************
	//	name:GetPtAt
	//	function:��ȡ����ָ��λ�õ���ݵ�ָ��
	//	input:��int index-ָ��λ��
	//  return: T ���
	//	remark: �ṩ���ڲ���ݵ�ֱ�ӷ��ʣ�С��ʹ��!!
	//******************************************
	T *GetPtAt(DWORD index)
	{
		FARRAYNODE      *pDest = NULL;
		ASSERT(index>=0&&index<m_nCount);
		pDest = GetDestSegEntry( index );
		return ((T *)pDest->pData)+index-m_nCurIndex;
	}
	
	T operator[](DWORD index){ return GetAt(index);}
	
	//******************************************
	//	name:GetSize
	//	function:��ȡ����������
	//	input:��
	//  return: int �����
	//	remark: 
	//******************************************
	int GetSize(){return m_nCount;}
	
	void SetCompackKey(BYTE byKey){ m_byZipKey=byKey;}
	//******************************************
	//	name:SetAt
	//	function:�޸�����ָ��λ�õ����
	//	input:��T newElement-�����
	//			int index-ָ�������
	//  return: BOOL TURE-�ɹ���FALSE��ʧ��
	//	remark: 
	//******************************************
	BOOL SetAt(DWORD index,T &newElement)
	{
		FARRAYNODE      *pDest = NULL;
		if(index<0||index>m_nCount-1)
			return FALSE;
		pDest=GetDestSegEntry(index);
		*(pDest->pData+index-m_nCurIndex) = newElement;
		return TRUE;
	}

	//******************************************
	//	name:InsertAt
	//	function:������ָ��λ�ò���һ�������
	//	input:��int index-ָ�������
	//			T newElement-���������
	//  return: BOOL TURE-�ɹ���FALSE��ʧ��
	//	remark: 
	//******************************************
	int InsertAt(DWORD index,T newElement)
	{
		if(index > m_nCount)
			return -1;
		if(index == m_nCount) 
			return Add(newElement);

		FARRAYNODE *pDest=GetDestSegEntry(index);
		//�����ҵ�Ŀ�굥Ԫ���ڵĽڵ�
	
		//���Ŀ��ڵ��Ѿ�������ôֻ���ں������һ���µĽڵ㣬
		//Ȼ���indexλ�ü������ĵ�Ԫֵ������²���ڵ㣬��ֵ��ֵ��index����Ԫ
		
		//���Ŀ��ڵ�û��������ôֻ���index�������ĵ�Ԫ������ֵ��ֵ��index����Ԫ
		if(pDest->dwUsed == m_nGrowBy)
		{			
			FARRAYNODE *pAddNode =NewNode();
			if(!pAddNode) return -1;
			
			pAddNode->dwUsed = pDest->dwUsed-(WORD)(index-m_nCurIndex);
			memcpy(pAddNode->pData,
				pDest->pData+index-m_nCurIndex,
				sizeof(T)*pAddNode->dwUsed);

			*(pDest->pData+index-m_nCurIndex) = newElement;
			pDest->dwUsed=(WORD)(index-m_nCurIndex+1);

			FARRAYNODE *pNext=pDest->pNextNode;
			pDest->pNextNode=pAddNode;
			pAddNode->pPrevNode=pDest;
			if(pNext)
			{
				pNext->pPrevNode=pAddNode;
				pAddNode->pNextNode=pNext;
			}
			else
			{
				m_pTailNode = pAddNode;			
				pAddNode->pNextNode = NULL;
			}
			m_nEmpty+=m_nGrowBy-1;
		}
		else
		{
			memmove(pDest->pData+index-m_nCurIndex+1,
				    pDest->pData+index-m_nCurIndex,
			 	    (pDest->dwUsed-(index-m_nCurIndex))*sizeof(T));

			*(pDest->pData+index-m_nCurIndex)=newElement;

			pDest->dwUsed++;
			m_nEmpty--;
		}
		m_nCount++;
		if(IsNeedCompack( )) Compack();
		return index;
	}

	//******************************************
	//	name:RemoveAt
	//	function:ɾ��������ָ��������а�����
	//	input:��int index-ָ�������
	//  return: BOOL TURE-�ɹ���FALSE��ʧ��
	//	remark: 
	//******************************************
	BOOL RemoveAt( DWORD index )
	{
		if(index<0||index>=m_nCount)
			return FALSE;
		FARRAYNODE *pDest=GetDestSegEntry(index);
		if(pDest==NULL) return true;//add by sunkm 2005/2/20

		m_nCount--;

		if(pDest->dwUsed==1)
		{
			FARRAYNODE *pPrev=pDest->pPrevNode;
			FARRAYNODE *pNext=pDest->pNextNode;
			if( !pPrev ) 
			{
				m_pHeadNode=pNext;
				if(m_pHeadNode) 
					m_pHeadNode->pPrevNode=NULL;
			}
			else
				pPrev->pNextNode=pNext;
			if( !pNext )
			{
				m_pTailNode = pPrev;
				if(m_pTailNode)
					m_pTailNode->pNextNode=NULL;
			}
			else
				pNext->pPrevNode=pPrev;
			
			if(pDest==m_pCurNode)
			{
				if(pPrev)
				{
					m_pCurNode  = pPrev;
					m_nCurIndex = m_nCurIndex-pPrev->dwUsed;
				}
				else
				{
					m_pCurNode  = m_pHeadNode;
					m_nCurIndex = m_pHeadNode?0:-1;
				}
				//�￪�����2005 2 21 ԭ4д�������£�ʹ�õ�ǰָ��ָ��ͷ��Ч�ʲ���
				//��Ϊָ��ɾ����ǰһ���
				//m_pCurNode=m_pHeadNode;
				//m_nCurIndex=m_pHeadNode?0:-1;
			}
			CallBack(pDest);
			m_nEmpty-=(m_nGrowBy-1);
		}
		else
		{
			memmove(pDest->pData+index-m_nCurIndex,
				    pDest->pData+index-m_nCurIndex+1,
				    sizeof(T)*(pDest->dwUsed-(index-m_nCurIndex+1)));
			pDest->dwUsed--;
			m_nEmpty++;
			if( IsNeedCompack() ) Compack();
		}
		return TRUE;
	}
	////
	//��ѹ�����߸ı��С���нڵ��ڵ�ԪdwUsed��0ʱ�����Щ�ڵ���յ�һ��ר�ŵط�
	//�����ظ����ã���Լϵͳ�ڴ������ͷŵĿ���
	// by sunkm
	////
	int CallBack(FARRAYNODE *begin,FARRAYNODE *end)//Ϊ��Ч�ʣ�����ֵ����������Ľڵ���
	{
		if(begin==NULL|| end ==NULL)
			return 0;
		if(m_pEmptyHeadNode==NULL)
		{
			m_pEmptyHeadNode = begin;
			m_pEmptyHeadNode->pPrevNode=NULL;
			m_pEmptyEndNode  = end;
			m_pEmptyEndNode->pNextNode=NULL;
			return 1;
		}
		begin->pPrevNode           = m_pEmptyEndNode;
		m_pEmptyEndNode->pNextNode = begin;		
		end->pNextNode             = NULL;
		return 1;
	}
	int CallBack(FARRAYNODE *node)
	{
		if(node==NULL)
			return 0;		
		if(m_pEmptyHeadNode==NULL)
		{
			m_pEmptyHeadNode = node;
			m_pEmptyHeadNode->pPrevNode=NULL;
			m_pEmptyEndNode  = node;
			m_pEmptyEndNode->pNextNode =NULL;
			return 1;
		}
		node->pPrevNode            = m_pEmptyEndNode;
		m_pEmptyEndNode->pNextNode = node;		
		node->pNextNode            = NULL;
		m_pEmptyEndNode            = node;
		return 1;
	}	
	//******************************************
	//	name:SetSize()
	//	function:������ݵ���
	//	input:��int size -��ݵ���
	//  return: BOOL TURE-�ɹ���FALSE��ʧ��
	//	remark:ֻ����)����
	//******************************************
	BOOL SetSize(DWORD size)
	{
		FARRAYNODE *pNewNode=NULL;
		if(m_nCount>=size)//��С����
		{
			//�����ȼ�Сʱ���ҵ�size��1��Ӧ�Ľڵ㣬Ȼ��Ѻ���Ľڵ㴮begin��end֮��Ķ����ա�
			//���Ұ�m_pTailNodeָ��pDest������������m_pCurNode��m_nCurIndexָ��pDest�ڵ�
			FARRAYNODE *pDest = GetDestSegEntry( size - 1 );
			FARRAYNODE *begin = pDest->pNextNode;
			FARRAYNODE *end   = m_pTailNode;

			pDest->dwUsed = size - m_nCurIndex;
			m_pTailNode   = pDest;		
			pDest ->pNextNode = NULL;	
			CallBack(begin,end);
			return FALSE;
		}
		else//��ӳ���
		{
			//��Ϊԭ4���һ��ڵ���Ҳ�?��ʣ�൥Ԫ�������ã����			
			//������Ҫ��Ӽ���ڵ�
			if(m_pTailNode)
			{
				if(m_nGrowBy - m_pTailNode->dwUsed >= size - m_nCount)
				{//������һ��ڵ���ʣ�൥Ԫ�����������Ԫ����ֻ���޸Ĳ�����������ӽڵ�					
					m_pTailNode->dwUsed+=(WORD)(size-m_nCount);//����ʹ�õĵ�Ԫ��������4
					m_nEmpty-=(size-m_nCount);
					m_nCount=size;
					return TRUE;
				}
				else
				{//����Ѻ����ʣ�ಿ��������4				
					m_nEmpty+=m_nGrowBy-m_pTailNode->dwUsed;
					m_nCount+=m_nGrowBy-m_pTailNode->dwUsed;
					m_pTailNode->dwUsed=m_nGrowBy;
				}
			}
			//���е������ʱ��Ҫô���һ��ڵ��Ѿ�ʹ���꣬Ҫôԭ4����Ϊ��
			int sumOfNewNode = (size-m_nCount+m_nGrowBy-1)/m_nGrowBy;
			int start = 0;
			if(m_pTailNode==NULL)
			{
				FARRAYNODE *pAddNode   =NewNode();	
				if(!pAddNode) return FALSE;
				//���ڴ�������½ڵ�
				pAddNode->dwUsed       = m_nGrowBy;
				pAddNode->pPrevNode    = NULL;
				pAddNode->pNextNode    = NULL;
				m_pCurNode = m_pTailNode = m_pHeadNode = pAddNode;
				m_nCurIndex=0;			
				start=1;
			}			
			for(int i = start ; i < sumOfNewNode ; i++)
			{
				FARRAYNODE *pAddNode   =NewNode();		
				if(!pAddNode) return FALSE;
				//���ڴ�������½ڵ�
				pAddNode->dwUsed       = m_nGrowBy;//���
				//�²���Ľڵ��и�Ԫ��Ϊ���Ѿ���ʹ�õ�
				//�˴�Ҫע�⣬�²���Ľڵ����Ѿ��������ϴ�ʹ�õ�ֵ
				m_pTailNode->pNextNode = pAddNode;
				pAddNode->pPrevNode    = m_pTailNode;
				pAddNode->pNextNode = NULL;
				m_pTailNode =pAddNode;
			}
			int left=( m_nGrowBy * sumOfNewNode - (size-m_nCount));//?
			m_pTailNode->dwUsed= m_nGrowBy-left;
			//���һ��ڵ㲻һ�����������Ҫ��ʵ�ʳ���4����
			m_nEmpty=m_nEmpty+left;
			//������һ��ڵ���ʣ��ĵ�Ԫ����ҲҪ����m_nEmpty			
			m_nCount = size;			
		}//�￪���޸���2005 2 21 
		
		/*
		if(m_pTailNode)
		{
			if((WORD)(size-m_nCount)<=m_nGrowBy-m_pTailNode->dwUsed)
			{//neet not to enlarge the buffer
				m_pTailNode->dwUsed+=(WORD)(size-m_nCount);
				m_nEmpty-=size-m_nCount;
				m_nCount=size;
				return TRUE;
			}else if(m_pTailNode->dwUsed!=m_nGrowBy)
			{//fill the tail node to full
				m_nEmpty+=m_nGrowBy-m_pTailNode->dwUsed;
				m_nCount+=m_nGrowBy-m_pTailNode->dwUsed;
				m_pTailNode->dwUsed=m_nGrowBy;
			}
		}
		int newsegs=(size-m_nCount+m_nGrowBy-1)/m_nGrowBy;
		for(int i=0;i<newsegs;i++)
		{
			pNewNode=NewNode();
			if(!pNewNode) return FALSE;
			pNewNode->dwUsed=(i<newsegs-1)?m_nGrowBy:(WORD)(size-m_nCount);
			if(!m_pHeadNode)
			{
				m_pHeadNode=m_pTailNode=m_pCurNode=pNewNode;
				m_nCurIndex=0;
			}
			ASSERT(m_pTailNode);
			m_pTailNode->pNextNode=pNewNode;
			pNewNode->pPrevNode=m_pTailNode;
			m_pTailNode=pNewNode;
			m_nCount+=pNewNode->dwUsed;
		}*/
		return TRUE;
	}
	
	//******************************************
	//	name:SetGrowBy()
	//	function:�����������
	//	input:��
	//  return: 
	//	remark: �ڳ�ʼ��ʱʹ��
	//******************************************
	void SetGrowBy(WORD nGrowBy)
	{
		ASSERT(m_nCount==0);
		m_nGrowBy=nGrowBy;
	}

	//******************************************
	//	name:RemoveAll()
	//	function:��ն����е����
	//	input:��
	//  return: BOOL TURE-�ɹ���FALSE��ʧ��
	//	remark:
	//******************************************
	BOOL RemoveAll()
	{
		if(m_pHeadNode)
			Free();	
		if(m_pEmptyHeadNode)
			FreePool();
		return TRUE;
	}

	//*********************************************
	//  name:Compack
	//	function :ѹ�����
	//	remark:��t���и���εĿ���ռ�������?�ͷŶ���Ľڵ�
	//*********************************************
	void Compack()
	{
		FARRAYNODE *pTmp1,*pTmp2=NULL;
		//pTemp1Ϊ�����п��൥Ԫ�Ľڵ�
		//pTemp2Ϊ�������Ѿ���ʹ�õ�Ԫ�Ľڵ㣬pTemp2���Ѿ���ʹ�õĵ�Ԫ������4���pTemp1�еĿ�λ
		if(m_pHeadNode==m_pTailNode) return;
		pTmp1=FindNotFullNode(m_pHeadNode,m_pTailNode);
		if(pTmp1)//while(pTmp1) sunkm changed 2005 2 21
		{
			if(pTmp2==NULL)	
				pTmp2=pTmp1->pNextNode;
			ASSERT(pTmp2);
			while(pTmp1->dwUsed!=(DWORD)m_nGrowBy&&pTmp2)
			{
				if(pTmp2->dwUsed <= (m_nGrowBy-pTmp1->dwUsed))
				{
					//��pTemp2�����õ�Ԫ��������ǡ����4���pTemp1�еĿ�λʱ��pTemp2���Ի���
					//the node can free
					memcpy(pTmp1->pData+pTmp1->dwUsed,
						pTmp2->pData,
						sizeof(T)*pTmp2->dwUsed);
					pTmp1->dwUsed+=pTmp2->dwUsed;
					if(m_pTailNode==pTmp2)
					{//���pTemp2��ĩβ�ڵ㣬����պ���Խ���
						m_pTailNode=pTmp2->pPrevNode;
						m_pTailNode->pNextNode=NULL;
						CallBack(pTmp2);
						pTmp2=NULL;
						break ;//�￪�����2005 2 21 
						        //��ΪpTemp2=NULL˵��pTemp1�еĿ���ռ��Ѿ��Ҳ�����Ԫ4��䣬ѹ�����
					}
					else
					{
						//��pTemp2�е�Ԫ��ȫ����4���pTemp1�еĿ�λ��û������pTemp1ʱpTemp2��ָ����һ��ڵ�
						//һ��4˵pTemp2Ӧ��ʼ����pTemp1����һ��ڵ�
						pTmp2->pPrevNode->pNextNode = pTmp2->pNextNode;
						pTmp2->pNextNode->pPrevNode = pTmp2->pPrevNode;
						FARRAYNODE *p=pTmp2->pNextNode;
						CallBack(pTmp2);
						pTmp2=p;

						if(pTmp1->dwUsed==m_nGrowBy)//�￪�����2005 2 21 ԭ4�㷨������
						{
							pTmp1=FindNotFullNode(pTmp1,m_pTailNode);
							if(pTmp1)
								pTmp2=pTmp1->pNextNode;
							else
								break ;//����Ҳ����п�λ�Ľڵ�˵�����
							//���pTemp1�еĿ�λ��ñ�����꣬����Ҫ�Ӻ���Ľڵ��м�������Ҫ����
						}
					}
				}
				else
				{
					//��pTemp2�����õ�Ԫ����4���pTemp1�еĿ�λ������ʣ��ʱ��
					//pTemp2��Ϊ��һ��Ҫ���Ľڵ㣬���pTemp2��ΪpTemp1��pTemp2Ϊ��һ��ڵ�
					DWORD dwMoveItems=m_nGrowBy-pTmp1->dwUsed;
					memcpy(pTmp1->pData+pTmp1->dwUsed,
						pTmp2->pData,
						sizeof(T)*dwMoveItems);
					memmove(pTmp2->pData,
						pTmp2->pData+dwMoveItems,
						sizeof(T)*(pTmp2->dwUsed-dwMoveItems));
					pTmp2->dwUsed-=(WORD)dwMoveItems;
					pTmp1->dwUsed=m_nGrowBy;

					//�￪�����2005 2 21
					pTmp1 = pTmp2;
					pTmp2 = pTmp1->pNextNode;
					//pTemp2��Ϊ��һ��Ҫ���Ľڵ㣬���pTemp2��ΪpTemp1��pTemp2Ϊ��һ��ڵ�					
				}				
			}//while			
			/*
			FARRAYNODE *pFind=FindNotFullNode(pTmp1,pTmp2);
			if(!pFind)
			{
				pTmp1=FindNotFullNode(pTmp1,m_pTailNode);
			}else
			{
				pTmp1=pFind;
			}
			pTmp2=NULL;
			*///�￪��ɾ����2005 2 21
		}//while		
		m_pCurNode=m_pHeadNode;//avoid the current node been deleted
		m_nCurIndex=0;
		if(m_pTailNode)
			m_nEmpty=m_nGrowBy-m_pTailNode->dwUsed;
		else
			m_nEmpty=0;
	}	

protected:
	bool m_bNeedCompack;


	void NeedCompack(bool bNeed)
	{
		m_bNeedCompack = bNeed;
	}
	//********************************************
	//	name:IsNeedCompack
	//	function:�ж��Ƿ���Ҫ����ݽ���ѹ��
	//*********************************************
	virtual BOOL IsNeedCompack()
	{
		return m_bNeedCompack;//�￪�����2005 2 21 ѹ�������ʹ�����������ò������
		if(m_nEmpty<m_nGrowBy) return FALSE;
		return ((m_nEmpty-m_nGrowBy)*100>m_nCount*m_byZipKey);
	}
	//******************************************
	//	name:Free()
	//	function:�ͷŶ���ռ�õĿռ䣬
	//	input:
	//  return:void
	//	remark:�ڲ�ʹ��,�ⲿҪ��ն�����ʹ��RemoveAll()�ӿ�
	//******************************************
public:
	virtual void Free()
	{
		FARRAYNODE *temp1,*temp2;
		temp1=m_pHeadNode;
		while(temp1!=NULL)
		{
			temp2=temp1->pNextNode;
			FreeNode(temp1);
			temp1=temp2;
		}
		m_pCurNode=m_pHeadNode=m_pTailNode=NULL;
		m_nCurIndex=-1;
		m_nCount=0;
		m_nEmpty=0;
	}
public:
	//******************************************
	//	name:Free()
	//	function:�ͷŶ�����ճ�ռ�õĿռ䣬
	//	input:
	//  return:void
	//	remark:�ڲ�ʹ��,�ⲿҪ��ն�����ʹ��RemoveAll()�ӿ�
	//******************************************
	virtual void FreePool()
	{
		FARRAYNODE *temp1,*temp2;
		temp1=m_pEmptyHeadNode;
		int xx=0;
		while(temp1!=NULL)
		{
			temp2=temp1->pNextNode;
			FreeNode(temp1);
			temp1=temp2;
			xx++;
		}
		m_pEmptyHeadNode=m_pEmptyEndNode=NULL;		
	}
private:

	//************************************************
	//	FreeNode
	//	�ͷŸý��ռ�õ��ڴ�
	//************************************************
	void FreeNode(FARRAYNODE *pNode)
	{
		delete []pNode->pData;
		delete pNode;
	}
	//*********************************************
	//	�����п��пռ�Ľ��
	//*********************************************
	FARRAYNODE *FindNotFullNode(FARRAYNODE *pBegin,FARRAYNODE *pEnd)
	{
		FARRAYNODE *pRet=pBegin;
		if(!pEnd) pEnd=m_pTailNode;
		while(pRet&&pRet->dwUsed==m_nGrowBy&&pRet!=pEnd)
			pRet=pRet->pNextNode;
		if(pRet==m_pTailNode||pRet==pEnd)
			pRet=NULL;
		return pRet;
	}

	//*******************************************
	//	name NewNode
	//	function:Ϊһ���½�����ռ�
	//********************************************
	FARRAYNODE *NewNode()
	{
		FARRAYNODE *pRet;
		if(m_pEmptyHeadNode)
		{
			pRet = m_pEmptyHeadNode;
			m_pEmptyHeadNode = m_pEmptyHeadNode->pNextNode;			
			if(m_pEmptyHeadNode == NULL)
				m_pEmptyEndNode =NULL;			
			else
				m_pEmptyHeadNode->pPrevNode=NULL;

		}
		else
		{
			pRet=new FARRAYNODE;	
			if(!pRet) return NULL;
			pRet->pData=new T[m_nGrowBy];
			if(pRet->pData==NULL)
			{
				delete pRet;
				return NULL;
			}
		}		
		pRet->pPrevNode=pRet->pNextNode=NULL;
		pRet->dwUsed=0;		
		return pRet;
	}
	//******************************************
	//	name:GetDestSegEntry()
	//  function:��ȡ�������t��Ľڵ�ָ��
	//	input:	int index -�������
	//  return: FARRAYNODE * -�������t��Ľڵ�ָ��
	//	remark:�ڲ�ʹ�ã�
	//******************************************
	FARRAYNODE * GetDestSegEntry(DWORD index)
	{
		FARRAYNODE * ret=NULL;
		//int			i = 0;
		DWORD		offset=0;
		if(index < m_nCurIndex)// dest pData is in before cur pData segment 
		{
			if(m_nCurIndex>2*index)
			{	//find the seg from head;
				ret=m_pHeadNode;
				while(offset+ret->dwUsed<=index)
				{
					offset+=ret->dwUsed;
					ret=ret->pNextNode;
				}
			}
			else	
			{	//find the seg from cur seg;
				ret=m_pCurNode;
				offset=m_nCurIndex;
				while(offset>index)
				{
					ret=ret->pPrevNode;
					offset-=ret->dwUsed;
				}
			}
			m_nCurIndex=offset;
		}
		else if(index >= (m_nCurIndex+m_pCurNode->dwUsed))
		{
			//if(m_nCurIndex+m_nCount<2*index)
			if(m_nCurIndex+m_nCount>2*index)//�￪���޸���2005 2 21			
			{//find the seg from cur
				ret=m_pCurNode;
				offset=m_nCurIndex;
				//while(offset+ret->dwUsed<=index)
				while(ret&&offset+ret->dwUsed<=index)///changed by sunkm 2005/2/20
				{
					offset+=ret->dwUsed;
					ret=ret->pNextNode;
				}
			}else
			{//find the seg from tail
				ret=m_pTailNode;
				offset=m_nCount-ret->dwUsed;;
				while(offset>index)
				{
					ret=ret->pPrevNode;
					offset-=ret->dwUsed;
				}
			}
			m_nCurIndex=offset;
		}else
		{//in cur pData seg
			ret=m_pCurNode;
		}
		m_pCurNode=ret;
		return ret;
	}

/////////////////////////////////////////////////////////////////////
//  private data
	DWORD m_nCurIndex;	//��ǰ�ڵ�ĵ�һ��Ԫ�ص������
	DWORD m_nCount;		//��������(�������
	WORD  m_nGrowBy;		//ÿ����ĳߴ�
	DWORD m_nEmpty;		//�ճ�Ŀռ���

	BYTE m_byZipKey;	//ѹ��ֵ
	FARRAYNODE * m_pCurNode;	//t���е�ǰ�ڵ��ָ�룬����ݼ���ʱȷ�������������
	FARRAYNODE * m_pHeadNode;	//ͷ�ڵ�
	FARRAYNODE * m_pTailNode;	//β���
    
	FARRAYNODE * m_pEmptyHeadNode;	//ͷ�ڵ�
	FARRAYNODE * m_pEmptyEndNode;	//β���


};

#endif // !defined(AFX_SARRAY_H__B1D40C22_2698_4202_921E_36D447EA4199__INCLUDED_)
