
#line 4 "queue_main.cc"
#include <stdio.h>
#include <string>     // std::string, std::to_string
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#line 1 ".././COST/cost.h"

























#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 ".././COST/priority_q.h"























#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <string.h>














template < class ITEM >
class SimpleQueue 
{
 public:
  SimpleQueue() :m_head(NULL) {};
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  ITEM* NextEvent() const { return m_head; };
  const char* GetName();
 protected:
  ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
  static const char* name = "SimpleQueue";
  return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
  if( m_head==NULL || item->time < m_head->time )
  {
    if(m_head!=NULL)m_head->prev=item;
    item->next=m_head;
    m_head=item;
    item->prev=NULL;
    return;
  }
    
  ITEM* i=m_head;
  while( i->next!=NULL && item->time > i->next->time)
    i=i->next;
  item->next=i->next;
  if(i->next!=NULL)i->next->prev=item;
  i->next=item;
  item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
  if(m_head==NULL)return NULL;
  ITEM* item = m_head;
  m_head=m_head->next;
  if(m_head!=NULL)m_head->prev=NULL;
  return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
  if(item==NULL) return;

  if(item==m_head)
  {
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
  }
  else
  {
    item->prev->next=item->next;
    if(item->next!=NULL)
      item->next->prev=item->prev;
  }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
  void Delete(ITEM*);
  void EnQueue(ITEM*);
  bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{

  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=NULL)
  {
    if(i==item)
    {
      pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
    }
    i=i->next;
  }
  SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=item&&i!=NULL)
    i=i->next;
  if(i==NULL)
    pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
  else
    SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
  char out[1000],buff[100];

  ITEM* i=SimpleQueue<ITEM>::m_head;
  bool qerror=false;

  sprintf(out,"queue error %s : ",s);
  while(i!=NULL)
  {
    sprintf(buff,"%f ",i->time);
    strcat(out,buff);
    if(i->next!=NULL)
      if(i->next->prev!=i)
      {
	qerror=true;
	sprintf(buff," {broken} ");
	strcat(out,buff);
      }
    if(i==i->next)
    {
      qerror=true;
      sprintf(buff,"{loop}");
      strcat(out,buff);
      break;
    }
    i=i->next;
  }
  if(qerror)
    printf("%s\n",out);
  return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
  ITEM* DeQueue(double);
  const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
  static const char* name = "ErrorQueue";
  return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
  

  if(drand48()>0.5)
    return SimpleQueue<ITEM>::DeQueue();

  int s=0;
  ITEM* e;
  e=SimpleQueue<ITEM>::m_head;
  while(e!=NULL&&e->time<stoptime)
  {
    s++;
    e=e->next;
  }
  e=SimpleQueue<ITEM>::m_head;
  s=(int)(s*drand48());
  while(s!=0)
  {
    e=e->next;
    s--;
  }
  Delete(e);
  return e;
}

template < class ITEM >
class HeapQueue 
{
 public:
  HeapQueue();
  ~HeapQueue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  const char* GetName();
  ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
 private:
  void SiftDown(int);
  void PercolateUp(int);
  void Validate(const char*);
        
  ITEM** elems;
  int num_of_elems;
  int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
  static const char* name = "HeapQueue";
  return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
  int i,j;
  char out[1000],buff[100];
  for(i=0;i<num_of_elems;i++)
    if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	 ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
    {
      sprintf(out,"queue error %s : ",s);
      for(j=0;j<num_of_elems;j++)
      {
	if(i!=j)
	  sprintf(buff,"%f(%d) ",elems[j]->time,j);
	else
	  sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
	strcat(out,buff);
      }
      printf("%s\n",out);
    }
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
  curr_max=16;
  elems=new ITEM*[curr_max];
  num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
  delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
  if(num_of_elems<=1) return;
  int i=node,k,c1,c2;
  ITEM* temp;
        
  do{
    k=i;
    c1=c2=2*i+1;
    c2++;
    if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
      i=c1;
    if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
      i=c2;
    if(k!=i)
    {
      temp=elems[i];
      elems[i]=elems[k];
      elems[k]=temp;
      elems[k]->pos=k;
      elems[i]->pos=i;
    }
  }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
  int i=node,k,p;
  ITEM* temp;
        
  do{
    k=i;
    if( (p=(i+1)/2) != 0)
    {
      --p;
      if(elems[i]->time < elems[p]->time)
      {
	i=p;
	temp=elems[i];
	elems[i]=elems[k];
	elems[k]=temp;
	elems[k]->pos=k;
	elems[i]->pos=i;
      }
    }
  }while(k!=i);
}

template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
  if(num_of_elems>=curr_max)
  {
    curr_max*=2;
    ITEM** buffer=new ITEM*[curr_max];
    for(int i=0;i<num_of_elems;i++)
      buffer[i]=elems[i];
    delete[] elems;
    elems=buffer;
  }
        
  elems[num_of_elems]=item;
  elems[num_of_elems]->pos=num_of_elems;
  num_of_elems++;
  PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
  if(num_of_elems<=0)return NULL;
        
  ITEM* item=elems[0];
  num_of_elems--;
  elems[0]=elems[num_of_elems];
  elems[0]->pos=0;
  SiftDown(0);
  return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
  int i=item->pos;

  num_of_elems--;
  elems[i]=elems[num_of_elems];
  elems[i]->pos=i;
  SiftDown(i);
  PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
 public:
  CalendarQueue();
  const char* GetName();
  ~CalendarQueue();
  void enqueue(ITEM*);
  ITEM* dequeue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  ITEM* NextEvent() const { return m_head;}
  void Delete(ITEM*);
 private:
  long last_bucket,number_of_buckets;
  double bucket_width;
        
  void ReSize(long);
  double NewWidth();

  ITEM ** buckets;
  long total_number;
  double bucket_top;
  long bottom_threshold;
  long top_threshold;
  double last_priority;
  bool resizable;

  ITEM* m_head;
  char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
  sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	  bucket_width,number_of_buckets);
  return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
  long i;
        
  number_of_buckets=16;
  bucket_width=1.0;
  bucket_top=bucket_width;
  total_number=0;
  last_bucket=0;
  last_priority=0.0;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  resizable=true;
        
  buckets= new ITEM*[number_of_buckets];
  for(i=0;i<number_of_buckets;i++)
    buckets[i]=NULL;
  m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
  delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
  long i;
  ITEM** old_buckets=buckets;
  long old_number=number_of_buckets;
        
  resizable=false;
  bucket_width=NewWidth();
  buckets= new ITEM*[newsize];
  number_of_buckets=newsize;
  for(i=0;i<newsize;i++)
    buckets[i]=NULL;
  last_bucket=0;
  total_number=0;

  
        
  ITEM *item;
  for(i=0;i<old_number;i++)
  {
    while(old_buckets[i]!=NULL)
    {
      item=old_buckets[i];
      old_buckets[i]=item->next;
      enqueue(item);
    }
  }
  resizable=true;
  delete[] old_buckets;
  number_of_buckets=newsize;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
  ITEM* head=m_head;
  m_head=dequeue();
  return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
  long i;
  for(i=last_bucket;;)
  {
    if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
    {
      ITEM * item=buckets[i];
      buckets[i]=buckets[i]->next;
      total_number--;
      last_bucket=i;
      last_priority=item->time;
                        
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      item->next=NULL;
      return item;
    }
    else
    {
      i++;
      if(i==number_of_buckets)i=0;
      bucket_top+=bucket_width;
      if(i==last_bucket)
	break;
    }
  }
        
  
  long smallest;
  for(smallest=0;smallest<number_of_buckets;smallest++)
    if(buckets[smallest]!=NULL)break;

  if(smallest >= number_of_buckets)
  {
    last_priority=bucket_top;
    return NULL;
  }

  for(i=smallest+1;i<number_of_buckets;i++)
  {
    if(buckets[i]==NULL)
      continue;
    else
      if(buckets[i]->time<buckets[smallest]->time)
	smallest=i;
  }
  ITEM * item=buckets[smallest];
  buckets[smallest]=buckets[smallest]->next;
  total_number--;
  last_bucket=smallest;
  last_priority=item->time;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  item->next=NULL;
  return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
  
  if(m_head==NULL)
  {
    m_head=item;
    return;
  }
  if(m_head->time>item->time)
  {
    enqueue(m_head);
    m_head=item;
  }
  else
    enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
  long i;
  if(item->time<last_priority)
  {
    i=(long)(item->time/bucket_width);
    last_priority=item->time;
    bucket_top=bucket_width*(i+1)+bucket_width*0.5;
    i=i%number_of_buckets;
    last_bucket=i;
  }
  else
  {
    i=(long)(item->time/bucket_width);
    i=i%number_of_buckets;
  }

        
  

  if(buckets[i]==NULL||item->time<buckets[i]->time)
  {
    item->next=buckets[i];
    buckets[i]=item;
  }
  else
  {

    ITEM* pos=buckets[i];
    while(pos->next!=NULL&&item->time>pos->next->time)
    {
      pos=pos->next;
    }
    item->next=pos->next;
    pos->next=item;
  }
  total_number++;
  if(resizable&&total_number>top_threshold)
    ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
  if(item==m_head)
  {
    m_head=dequeue();
    return;
  }
  long j;
  j=(long)(item->time/bucket_width);
  j=j%number_of_buckets;
        
  

  
  

  ITEM** p = &buckets[j];
  
  ITEM* i=buckets[j];
    
  while(i!=NULL)
  {
    if(i==item)
    { 
      (*p)=item->next;
      total_number--;
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      return;
    }
    p=&(i->next);
    i=i->next;
  }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
  long i, nsamples;
        
  if(total_number<2) return 1.0;
  if(total_number<=5)
    nsamples=total_number;
  else
    nsamples=5+total_number/10;
  if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
  long _last_bucket=last_bucket;
  double _bucket_top=bucket_top;
  double _last_priority=last_priority;
        
  double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
  ITEM* list,*next,*item;
        
  list=dequeue(); 
  long real_samples=0;
  while(real_samples<nsamples)
  {
    item=dequeue();
    if(item==NULL)
    {
      item=list;
      while(item!=NULL)
      {
	next=item->next;
	enqueue(item);
	item=next;      
      }

      last_bucket=_last_bucket;
      bucket_top=_bucket_top;
      last_priority=_last_priority;

                        
      return 1.0;
    }
    AVG[real_samples]=item->time-list->time;
    avg1+=AVG[real_samples];
    if(AVG[real_samples]!=0.0)
      real_samples++;
    item->next=list;
    list=item;
  }
  item=list;
  while(item!=NULL)
  {
    next=item->next;
    enqueue(item);
    item=next;      
  }
        
  last_bucket=_last_bucket;
  bucket_top=_bucket_top;
  last_priority=_last_priority;
        
  avg1=avg1/(double)(real_samples-1);
  avg1=avg1*2.0;
        
  
  long count=0;
  for(i=0;i<real_samples-1;i++)
  {
    if(AVG[i]<avg1&&AVG[i]!=0)
    {
      avg2+=AVG[i];
      count++;
    }
  }
  if(count==0||avg2==0)   return 1.0;
        
  avg2 /= (double) count;
  avg2 *= 3.0;
        
  return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 38 ".././COST/cost.h"


#line 1 ".././COST/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 39 ".././COST/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  
  };
  TimerBase* object;
  int index;
  unsigned char active;
};



class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	
};

class TypeII;



class CostSimEng
{
 public:

  class seed_t
      {
       public:
	void operator = (long seed) { srand48(seed); };
      };
  seed_t		Seed;
  CostSimEng()
      : stopTime( 0), clearStatsTime( 0), m_clock( 0.0)
      {
        if( m_instance == NULL)
	  m_instance = this;
        else
	  printf("Error: only one simulation engine can be created\n");
      }
  virtual		~CostSimEng()	{ }
  static CostSimEng	*Instance()
      {
        if(m_instance==NULL)
        {
	  printf("Error: a simulation engine has not been initialized\n");
	  m_instance = new CostSimEng;
        }
        return m_instance;
      }
  CorsaAllocator	*GetAllocator(unsigned int datasize)
      {
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
	  if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
      }
  void		AddComponent(TypeII*c)
      {
        m_components.push_back(c);
      }
  void		ScheduleEvent(CostEvent*e)
      {
	if( e->time < m_clock)
	  assert(e->time>=m_clock);
        
        m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        
        m_queue.Delete(e);
      }
  double	Random( double v=1.0)	{ return v*drand48();}
  int		Random( int v)		{ return (int)(v*drand48()); }
  double	Exponential(double mean)	{ return -mean*log(Random());}
  virtual void	Start()		{}
  virtual void	Stop()		{}
  void		Run();
  double	SimTime()	{ return m_clock; } 
  void		StopTime( double t)	{ stopTime = t; }
  double	StopTime() const	{ return stopTime; }
  void		ClearStatsTime( double t)	{ clearStatsTime = t; }
  double	ClearStatsTime() const	{ return clearStatsTime; }
  virtual void	ClearStats()	{}
 private:
  double	stopTime;
  double	clearStatsTime;	
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};




class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		
  TypeII()
      {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
      }

#ifdef COST_DEBUG
  void Print(const bool, const char*, ...);
#endif
    
  double Random(double v=1.0) { return v*drand48();}
  int Random(int v) { return (int)(v*drand48());}
  double Exponential(double mean) { return -mean*log(Random());}
  inline double SimTime() const { return m_simeng->SimTime(); }
  inline double StopTime() const { return m_simeng->StopTime(); }
 private:
  CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
  if(flag==false) return;
  va_list ap;
  va_start(ap, format);
  printf("[%.10f] ",SimTime());
  vprintf(format,ap);
  va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
  double	nextTime = (clearStatsTime != 0.0 && clearStatsTime < stopTime) ? clearStatsTime : stopTime;

  m_clock = 0.0;
  eventsProcessed = 0l;
  std::vector<TypeII*>::iterator iter;
      
  struct timeval start_time;    
  gettimeofday( &start_time, NULL);

  Start();

  for( iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Start();

  CostEvent* e=m_queue.DeQueue();
  while( e != NULL)
  {
    if( e->time >= nextTime)
    {
      if( nextTime == stopTime)
	break;
      
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    
    assert( e->time >= m_clock);
    m_clock = e->time;
    e->object->activate( e);
    eventsProcessed++;
    e = m_queue.DeQueue();
  }
  m_clock = stopTime;
  for(iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Stop();
	    
  Stop();

  struct timeval stop_time;    
  gettimeofday(&stop_time,NULL);

  runningTime = stop_time.tv_sec - start_time.tv_sec +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0;
  eventRate = eventsProcessed/runningTime;
  
  
  printf("# -------------------------------------------------------------------------\n");	
  printf("# CostSimEng with %s, stopped at %f\n", m_queue.GetName(), stopTime);	
  printf("# %ld events processed in %.3f seconds, event processing rate: %.0f\n",	
  eventsProcessed, runningTime, eventRate);
  
}







#line 15 "queue_main.cc"




#line 1 "queue.h"
 



#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>


#line 1 "FIFO.h"

#line 1 "transaction.h"
 



#ifndef _AUX_TRANSACTION_
#define _AUX_TRANSACTION_


struct Transaction
{
	int transaction_id;				
	double timestamp_generated;

};

#endif

#line 1 "FIFO.h"

#include <deque>





struct FIFO
{	
	std::deque <Transaction> m_queue;
	
	int queue_size;

	Transaction GetFirstPacket();
	Transaction GetPacketAt(int n);
	void DelFirstPacket();		
	void DeletePacketIn(int i);
	void PutPacket(Transaction &packet);
	void PutPacketFront(Transaction &packet);
	void PutPacketIn(Transaction &packet, int);
	int QueueSize();
};

Transaction FIFO :: GetFirstPacket()
{
	return(m_queue.front());	
}; 

Transaction FIFO :: GetPacketAt(int n)
{
	return(m_queue.at(n));	
}; 


void FIFO :: DelFirstPacket()
{
	m_queue.pop_front();
}; 

void FIFO :: PutPacket(Transaction &packet)
{	
	m_queue.push_back(packet);
}; 

void FIFO :: PutPacketFront(Transaction &packet)
{	
	m_queue.push_front(packet);
}; 

int FIFO :: QueueSize()
{
	return(m_queue.size());
}; 

void FIFO :: PutPacketIn(Transaction & packet,int i)
{
	m_queue.insert(m_queue.begin()+i,packet);
}; 

void FIFO :: DeletePacketIn(int i)
{
	m_queue.erase(m_queue.begin()+i);
};



#line 11 "queue.h"


#line 1 "transaction.h"
 



#ifndef _AUX_TRANSACTION_
#define _AUX_TRANSACTION_


struct Transaction
{
	int transaction_id;				
	double timestamp_generated;

};

#endif

#line 12 "queue.h"


#line 1 "performance.h"

 



#ifndef _AUX_PERFORMANCE_
#define _AUX_PERFORMANCE_

struct Performance
{

	int total_transactions;
	int transactions_dropped;
	double drop_percentage;
	int num_blocks_mined;
	int num_blocks_mined_by_timeout;

	double mean_occupancy;
	double mean_delay;

	double fork_probability;

	



	
	
	
	
	
	

};

#endif

#line 13 "queue.h"


#line 1 "generic_methods.h"






#include <stdlib.h>

#ifndef _GENERIC_METHODS_
#define _GENERIC_METHODS_

int compare (const void * a, const void * b)
{
  if (*(double*)a > *(double*)b) return 1;
  else if (*(double*)a < *(double*)b) return -1;
  else return 0;  
}

#endif

#line 14 "queue.h"




#line 145 "queue.h"
;





#line 157 "queue.h"
;





#line 268 "queue.h"
;





#line 350 "queue.h"
;





#line 399 "queue.h"
;






#line 488 "queue.h"
;





#line 18 "queue_main.cc"


#line 1 "traffic_generator.h"
 



#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>



#line 57 "traffic_generator.h"
;





#line 64 "traffic_generator.h"
;





#line 71 "traffic_generator.h"
;





#line 19 "queue_main.cc"


#line 1 "logger.h"




#ifndef _AUX_LOGGER_
#define _AUX_LOGGER_

struct Logger
{
	int save_logs;					
	FILE *file;					
	char head_string[8];			

	


	void SetVoidHeadString(){
		sprintf(head_string, "%s", " ");
	}

};

#endif

#line 20 "queue_main.cc"


#line 1 "output_processing.h"






#include <math.h>
#include <stddef.h>
#include <string>
#include <sstream>

#ifndef _OUT_METHODS_
#define _OUT_METHODS_




void PrintAndWriteSimulationStatistics() {

}






void GenerateScriptOutput(Logger &logger_script, double simulation_time, Performance performance,
	double lambda, double mu, double waiting_timeout, int queue_size, int batch_size) {

	fprintf(logger_script.file, " Output (sim_time=%f-lambda=%f-mu=%f-waiting_timeout=%f-queue_size=%d-batch_size=%d)\n", 
		simulation_time, lambda, mu, waiting_timeout, queue_size, batch_size);
	fprintf(logger_script.file, "%d;%d;%f;%f;%f;%f;%f\n", performance.total_transactions, performance.transactions_dropped, 
		performance.drop_percentage, (double)performance.num_blocks_mined_by_timeout/(double)performance.num_blocks_mined,
		performance.mean_occupancy, performance.mean_delay, performance.fork_probability);

}

#endif

#line 21 "queue_main.cc"


#line 1 "performance.h"

 



#ifndef _AUX_PERFORMANCE_
#define _AUX_PERFORMANCE_

struct Performance
{

	int total_transactions;
	int transactions_dropped;
	double drop_percentage;
	int num_blocks_mined;
	int num_blocks_mined_by_timeout;

	double mean_occupancy;
	double mean_delay;

	double fork_probability;

	



	
	
	
	
	
	

};

#endif

#line 22 "queue_main.cc"


template <typename T>
std::string ToString(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}



#line 122 "queue_main.cc"
;





#line 129 "queue_main.cc"
;





#line 144 "queue_main.cc"
;





#include "compcxx_queue_main.h"
class compcxx_Queue_5;/*template <class T> */
#line 267 ".././COST/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Queue_5* p_compcxx_parent;};

class compcxx_Queue_5;/*template <class T> */
#line 267 ".././COST/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Queue_5* p_compcxx_parent;};


#line 17 "queue.h"
class compcxx_Queue_5 : public compcxx_component, public TypeII{

	
	public:

		
		void Setup();
		void Start();
		void Stop();

		
		void InitializeVariables();
		
		
		void CheckMining();
		void SetMiningTimeout();


	
	public:

		double simulation_time;	 
		int logs_enabled;	 
		int queue_size; 	 
		int batch_size; 	 
		double departures_rate;  
		double timeout_mining;   
		int n_miners; 		 
		double capacity_p2p; 	 
		
		Performance performance;

		FIFO buffer;		
	
	
	private: 

		int mining_active; 			

		double delay_measurements;		
		int num_mined_transactions;		

		int *occupancy_array;			
		int occupancy_measurements;		
		int num_occupancy_measurements;		

		int *departure_array;			
		int num_departure_measurements;		

		int current_state;
		int previous_state;

		int current_d_state;
		int previous_d_state;		
	
		double timestamp_last_state;
		double timestamp_last_d_state;
		double **queue_status_from_departure;
		double *time_in_d_state;
		double *time_in_k_from_d_state;

		double *time_per_state;

		int *counter_queue_departure; 

		double *sum_time_spent_per_d_state;
		int *measurements_time_per_d_state;
		
		int num_transactions_generated;		
		int num_transactions_dropped;		
		int transaction_counter;		
		int num_transaction_to_be_mined; 	

		int timeout_active;			
		int times_timeout_expired;		
		int *times_timeout_expired_from_d_state;
		int *times_timeout_expired_from_state;

		int n_blocks;		
		int n_forks;
		double *delay_miners;		
		double propagation_time;	

		const int HEADER_LENGTH = 640;
		const int TRANSACTION_LENGTH = 3000;

		double timestamp_current_epoch;
		double sum_duration_epoch;
		int counter_sum_duration_epoch;

		int timer_expired;
		int arrivals_from_state;
		int *arrivals_from_state_timer;
		int *arrivals_from_state_no_timer;
		int *counter_arrivals_timer;
		int *counter_arrivals_no_timer;	

		double *expected_arrivals_timer;
		double *expected_arrivals_no_timer;		
		
		

	
	public:

		
		/*inport */void inline InportNewPacketGenerated();

		
		compcxx_Timer_2 /*<trigger_t> */trigger_toStartMining;
		compcxx_Timer_3 /*<trigger_t> */trigger_toFinishMining;

		
		/*inport */inline void StartMining(trigger_t& t1);
		/*inport */inline void MiningFinished(trigger_t& t1);

		
		compcxx_Queue_5 () {
			trigger_toStartMining.p_compcxx_parent=this /*connect trigger_toStartMining.to_component,*/;
			trigger_toFinishMining.p_compcxx_parent=this /*connect trigger_toFinishMining.to_component,*/;
		}
};




class compcxx_TrafficGenerator_6;/*template <class T> */
#line 267 ".././COST/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_TrafficGenerator_6* p_compcxx_parent;};

class compcxx_Queue_5;
#line 12 "traffic_generator.h"
class compcxx_TrafficGenerator_6 : public compcxx_component, public TypeII{

	
	public:
		
		void Setup();
		void Start();
		void Stop();

		
		void InitializeTrafficGenerator();
		void GenerateTraffic();

	
	public:

		int traffic_model;	
		double traffic_load;	

		int logs_enabled;	
		
	
	public:

		
		/*inport */inline void NewPacketGenerated(trigger_t& t1);

		
		/*outport void outportNewPacketGenerated()*/;

		
		compcxx_Timer_4 /*<trigger_t> */trigger_new_packet_generated;

		
		compcxx_TrafficGenerator_6 () {
			trigger_new_packet_generated.p_compcxx_parent=this /*connect trigger_new_packet_generated.to_component,*/;
		}

public:compcxx_Queue_5* p_compcxx_Queue_5;};





#line 33 "queue_main.cc"
class compcxx_QueueSimulator_7 : public compcxx_component, public CostSimEng {

	
	public:

		void Setup(double simulation_time_console, int logs_enabled_console, int queue_size_console, 
			int batch_size_console, double lambda_console, double mu_console, double timeout_mining_console, 
			int seed_console, int forks_enabled, int n_miners, double capacity_p2p, const char *script_output_filename);
		void Stop();
		void Start();

		void SetupEnvironment();
		void PrintSystemInfo();

	
	public:
		
		
		double simulation_time;	
		int logs_enabled;	
		int queue_size;		
		int batch_size;		
		double lambda;		
		double mu;		
		double timeout_mining;	
		
		int n_miners;		
		double capacity_p2p;	

		compcxx_Queue_5 queue;			    
		compcxx_TrafficGenerator_6 traffic_generator; 

	
	private:

		
		int seed; 

		
		int forks_enabled;	

		
		FILE *script_output_file;			
		Logger logger_script;				
		
};













#line 288 ".././COST/cost.h"

#line 288 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 ".././COST/cost.h"

#line 300 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 ".././COST/cost.h"

#line 311 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 ".././COST/cost.h"

#line 319 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->MiningFinished(m_event.data));
}




#line 288 ".././COST/cost.h"

#line 288 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 ".././COST/cost.h"

#line 300 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 ".././COST/cost.h"

#line 311 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 ".././COST/cost.h"

#line 319 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->StartMining(m_event.data));
}




#line 130 "queue.h"

#line 131 "queue.h"

#line 143 "queue.h"
void compcxx_Queue_5 :: Setup(){
	
}
#line 150 "queue.h"
void compcxx_Queue_5 :: Start(){

	if(logs_enabled) printf("%.12f; Queue Start()\n", SimTime());

	
	SetMiningTimeout();	
	
}
#line 162 "queue.h"
void compcxx_Queue_5 :: Stop(){

	if(logs_enabled) printf("%.12f; Queue Stop()\n", SimTime());

	if(logs_enabled) printf("\n---------------------------\n");
	if(logs_enabled) printf("------- STATISTICS --------\n");
	if(logs_enabled) printf("---------------------------\n");
	if(logs_enabled) printf("- Total transactions generated/dropped: %d/%d (drop ratio=%.2f%%)\n", 
		num_transactions_generated, num_transactions_dropped, 100*(double)num_transactions_dropped/(double)num_transactions_generated);
	
	if(logs_enabled) printf("- Blocks mined by timeout: %d/%d (%.2f%%)\n", times_timeout_expired, n_blocks, 100*(double)times_timeout_expired/(double)n_blocks);
	if(logs_enabled) printf("- Average queue delay: %f s\n", delay_measurements/num_mined_transactions);
	if(logs_enabled) printf("- Average queue occupancy: %f transactions\n", (double)occupancy_measurements/(double)num_occupancy_measurements);
	if(logs_enabled) printf("- Average inter-departure time: %f\n", sum_duration_epoch/counter_sum_duration_epoch);

	if(logs_enabled) printf("- Times timeout expired from d state: ");
	for (int i=0; i < queue_size+1; ++i) {
		if(logs_enabled) printf("%d (%.2f%%) ", times_timeout_expired_from_d_state[i], 100*(double)times_timeout_expired_from_d_state[i]/(double)times_timeout_expired);
	}
	if(logs_enabled) printf("\n");
	if(logs_enabled) printf("- Times timeout expired from state: ");
	for (int i=0; i < queue_size+1; ++i) {
		if(logs_enabled) printf("%d (%.2f%%) ", times_timeout_expired_from_state[i], 100*(double)times_timeout_expired_from_state[i]/(double)times_timeout_expired);
	}
	if(logs_enabled) printf("\n");

	if(logs_enabled) printf("- Forks vs Blocks: %d - %d (%.2f%%)\n", n_forks, n_blocks, 100*(double)n_forks/(double)n_blocks);

	performance.total_transactions = num_transactions_generated;
	performance.transactions_dropped = num_transactions_dropped;
	performance.drop_percentage = (double)num_transactions_dropped/(double)num_transactions_generated;
	performance.num_blocks_mined = n_blocks;
	performance.num_blocks_mined_by_timeout = times_timeout_expired;
	performance.mean_occupancy = (double)occupancy_measurements/(double)num_occupancy_measurements;
	performance.mean_delay = delay_measurements/num_mined_transactions;
	performance.fork_probability = (double)n_forks/(double)n_blocks;

	if (queue_size <= 10) { 

		if(logs_enabled) printf("\n       + + + + + + + + +\n");		
		if(logs_enabled) printf("       +  Model match  +\n");
		if(logs_enabled) printf("       + + + + + + + + +\n");	
		
		
		if(logs_enabled) printf("\n+ Departure Probabilities (pi_d):\n\t");
		double *pi_d = new double[queue_size+1];
		for(int i = 0; i < queue_size+1; ++i){
			pi_d[i] = (double)departure_array[i]/(double)num_departure_measurements;
			if(logs_enabled)printf("%f ", pi_d[i]);
		}
		if(logs_enabled) printf("\n");

		
		if(logs_enabled) printf("\n+ Steady-state Probabilities (pi_s):\n\t");
		double *pi_s = new double[queue_size+1];
		for(int i = 0; i < queue_size+1; ++i){
			pi_s[i] = (double)occupancy_array[i]/(double)num_occupancy_measurements;
			if(logs_enabled) printf("%f ", pi_s[i]);
		}
		if(logs_enabled) printf("\n");

		if(logs_enabled) printf("\n[Cross-validation] Steady-state distr. (pi_s) from time measured in each state:\n\t");
		for(int i = 0; i < queue_size+1; ++i){
			if(logs_enabled) printf("%f ", time_per_state[i]/simulation_time);
		}
		if(logs_enabled) printf("\n");

		
		if(logs_enabled) printf("\n+ Blocking probability (pb): %f\n", (double)num_transactions_dropped/(double)num_transactions_generated);


		printf("arrivals_from_state_timer:\n");
		for (int i=0; i < queue_size+1; ++i) {
			if (counter_arrivals_timer[i] == 0) {
				expected_arrivals_timer[i] = 0;
			} else {
				expected_arrivals_timer[i] = (double)arrivals_from_state_timer[i]/counter_arrivals_timer[i];
			}
			printf("%f ", expected_arrivals_timer[i]);
		}
		printf("\n");

		printf("arrivals_from_state_no_timer:\n");
		for (int i=0; i < queue_size+1; ++i) {
			if (counter_arrivals_no_timer[i] == 0) {
				expected_arrivals_no_timer[i] = 0;
			} else {
				expected_arrivals_no_timer[i] = (double)arrivals_from_state_no_timer[i]/counter_arrivals_no_timer[i];
			}
			printf("%f ", expected_arrivals_no_timer[i]);
		}
		printf("\n");

		double mean_arrivals_timer = 0;
		double mean_arrivals_no_timer = 0;
		for (int i=0; i < queue_size+1; ++i) {
			mean_arrivals_timer += pi_d[i]*expected_arrivals_timer[i];
			mean_arrivals_no_timer += pi_d[i]*expected_arrivals_no_timer[i];
		}
		
		printf("- mean_arrivals_timer = %f\n", mean_arrivals_timer);
		printf("- mean_arrivals_no_timer = %f\n", mean_arrivals_no_timer);
		
	}
	if(logs_enabled) printf("---------------------------\n");
	if(logs_enabled) printf("\n");
}
#line 273 "queue.h"
void compcxx_Queue_5 :: InportNewPacketGenerated(){

	

	++ num_transactions_generated;
	++ arrivals_from_state;

	
	previous_state = current_state;

	
	occupancy_measurements += buffer.QueueSize();
	occupancy_array[buffer.QueueSize()] += 1;
	++ num_occupancy_measurements;

	
	if (buffer.QueueSize() < queue_size) {
		
		Transaction new_transaction;
		new_transaction.timestamp_generated = SimTime();
		new_transaction.transaction_id = transaction_counter;
		buffer.PutPacket(new_transaction);
	} else { 
		
		++ num_transactions_dropped;
	}

	
	++ transaction_counter;

	current_state = buffer.QueueSize();

	if (previous_state != current_state) {		
		
		time_in_k_from_d_state[previous_state] = SimTime() - timestamp_last_state;
		
		time_per_state[previous_state] += SimTime() - timestamp_last_state;
		timestamp_last_state = SimTime();
	}

	
	CheckMining();
	
}





#line 321 "queue.h"
void compcxx_Queue_5 :: SetMiningTimeout(){
	
	if (!trigger_toStartMining.Active()) {
		timeout_active = 1;
		double time_to_trigger = SimTime()+timeout_mining;
		trigger_toStartMining.Set(time_to_trigger);
		
	}
}





#line 334 "queue.h"
void compcxx_Queue_5 :: CheckMining() {

	
	
	
	if (!mining_active && buffer.QueueSize() >= batch_size) {
		
		if (trigger_toStartMining.Active()) {
			
			trigger_toStartMining.Cancel();
			timeout_active = 0;
		}
		
		trigger_toStartMining.Set(SimTime());	
	}

}
#line 355 "queue.h"
void compcxx_Queue_5 :: StartMining(trigger_t &){

	
	mining_active = 1;	

	
	if (timeout_active) {
		timer_expired = 1;
		++times_timeout_expired;	
		++times_timeout_expired_from_d_state[current_d_state];
		++times_timeout_expired_from_state[current_state];
	}
	
	
	timeout_active = 0;	

	
	for (int i = 0; i < n_miners; ++i) {
		delay_miners[i] = Exponential(1/departures_rate);
	}
	qsort(delay_miners, n_miners, sizeof(double), compare);
	double mining_time = delay_miners[0];
	double time_to_trigger = SimTime() + mining_time;
	trigger_toFinishMining.Set(time_to_trigger);

	
	if (buffer.QueueSize() < batch_size) {
		num_transaction_to_be_mined = buffer.QueueSize();
	} else {
		num_transaction_to_be_mined = batch_size;
	}

	
	propagation_time = (HEADER_LENGTH + num_transaction_to_be_mined*TRANSACTION_LENGTH)/capacity_p2p;
	
	if (n_miners > 1) {
		if (delay_miners[1]-delay_miners[0] < propagation_time) {
			num_transaction_to_be_mined = 0;
			++n_forks;		
		}
	}
		
	

}
#line 405 "queue.h"
void compcxx_Queue_5 :: MiningFinished(trigger_t &){

	++n_blocks;
	
	
	

	
	previous_state = current_state;	

	
	time_in_k_from_d_state[current_state] = SimTime() - timestamp_last_state;
	
	double time_in_state = 0;
	for (int i = 0; i < queue_size + 1; ++i) {
	
		time_in_state += time_in_k_from_d_state[i]; 
	}
	
	double time_since_last_departure = SimTime() - timestamp_last_d_state;
	
	for (int i = 0; i < queue_size + 1; ++i) {
		
		if (time_since_last_departure >= 0) {
			queue_status_from_departure[current_d_state][i] += time_in_k_from_d_state[i]; 
		}	
		time_in_k_from_d_state[i] = 0;	
	}
	
	time_in_d_state[current_d_state] += time_since_last_departure;

	sum_time_spent_per_d_state[current_d_state] += time_since_last_departure;
	++measurements_time_per_d_state[current_d_state];
	
	
	for (int i = 0; i < num_transaction_to_be_mined; ++ i) {
		++ num_mined_transactions;
		delay_measurements += (SimTime() - buffer.GetFirstPacket().timestamp_generated);
		buffer.DelFirstPacket();
	}
	


	
	if (timer_expired) {
		arrivals_from_state_timer[current_d_state] += arrivals_from_state;
		++counter_arrivals_timer[current_d_state];
	} else {
		arrivals_from_state_no_timer[current_d_state] += arrivals_from_state;
		++counter_arrivals_no_timer[current_d_state];
	}
	arrivals_from_state = 0;
	timer_expired = 0;
	
	current_state = buffer.QueueSize();
	current_d_state = buffer.QueueSize();	
	timestamp_last_d_state = SimTime();

	if (previous_state != current_state) {		
		time_per_state[previous_state] += SimTime() - timestamp_last_state;
		timestamp_last_state = SimTime();
	}

	
	departure_array[buffer.QueueSize()] += 1;
	++num_departure_measurements;

	
	sum_duration_epoch += (SimTime()-timestamp_current_epoch);
	++counter_sum_duration_epoch;

	
	timestamp_current_epoch = SimTime();

	
	mining_active = 0;

	
	SetMiningTimeout();

	
	CheckMining();
	
}
#line 493 "queue.h"
void compcxx_Queue_5 :: InitializeVariables() {

	mining_active = 0;

	delay_measurements = 0;
	occupancy_measurements = 0;
	num_occupancy_measurements = 0;
	num_departure_measurements = 0;	

	num_transactions_generated = 0;
	num_transactions_dropped = 0;
	transaction_counter = 0;
	num_transaction_to_be_mined = 0;
	num_mined_transactions = 0;

	timeout_active = 0;
	times_timeout_expired = 0;
	times_timeout_expired_from_d_state = new int[queue_size+1];
	times_timeout_expired_from_state = new int[queue_size+1];

	departure_array = new int[queue_size+1];
	occupancy_array = new int[queue_size+1];

	time_per_state = new double[queue_size+1];

	queue_status_from_departure = new double*[queue_size+1]; 
	time_in_d_state = new double[queue_size+1]; 
	time_in_k_from_d_state = new double[queue_size+1]; 

	sum_time_spent_per_d_state = new double[queue_size+1]; 
	measurements_time_per_d_state = new int[queue_size+1];
	
	current_state = 0;
	timestamp_last_state = 0;
	current_d_state = 0;
	timestamp_last_d_state = 0;

	delay_miners = new double[n_miners];
	for (int i = 0; i < n_miners; ++i){
		delay_miners[i] = 0;
	}

	n_blocks = 0;
	n_forks = 0;

	timer_expired = 0;
	arrivals_from_state = 0; 
	arrivals_from_state_timer = new int[queue_size+1]; 
	arrivals_from_state_no_timer = new int[queue_size+1]; 
	counter_arrivals_timer = new int[queue_size+1];
	counter_arrivals_no_timer = new int[queue_size+1];

	expected_arrivals_timer = new double[queue_size+1];
	expected_arrivals_no_timer = new double[queue_size+1];

	for(int i = 0; i < queue_size+1; ++i){
		departure_array[i] = 0;
		occupancy_array[i] = 0;
		time_per_state[i] = 0;
		time_in_k_from_d_state[i] = 0;
		sum_time_spent_per_d_state[i] = 0;
		measurements_time_per_d_state[i] = 0;
		queue_status_from_departure[i] = new double[queue_size+1];
		time_in_d_state[i] = 0;
		times_timeout_expired_from_d_state[i] = 0;
		times_timeout_expired_from_state[i] = 0;
		arrivals_from_state_timer[i] = 0;
		arrivals_from_state_no_timer[i] = 0;
		counter_arrivals_timer[i] = 0;
		counter_arrivals_no_timer[i] = 0;
		expected_arrivals_timer[i] = 0;
		expected_arrivals_no_timer[i] = 0;
		for(int j = 0; j < queue_size; ++j){
			queue_status_from_departure[i][j] = 0;
		}
	}

	timestamp_current_epoch = 0;
	sum_duration_epoch = 0;
	counter_sum_duration_epoch = 0;

}

#line 288 ".././COST/cost.h"

#line 288 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 ".././COST/cost.h"

#line 300 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 ".././COST/cost.h"

#line 311 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 ".././COST/cost.h"

#line 319 ".././COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->NewPacketGenerated(m_event.data));
}




#line 55 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: Setup(){
	
}
#line 62 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: Start(){
	InitializeTrafficGenerator();
}
#line 69 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: Stop(){
	
}
#line 76 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: GenerateTraffic() {

	double time_for_next_packet (0);
	double time_to_trigger (0);

	switch(traffic_model) {

		
		case 1:{
			time_for_next_packet = Exponential(1/traffic_load);
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(time_to_trigger);
			break;
		}

		
		case 2:{
			time_for_next_packet = 1/traffic_load;
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(time_to_trigger);
			break;
		}


		default:{
			printf("Wrong traffic model!\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
}





#line 111 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: NewPacketGenerated(trigger_t &){
	(p_compcxx_Queue_5->InportNewPacketGenerated());
	GenerateTraffic();
}





#line 119 "traffic_generator.h"
void compcxx_TrafficGenerator_6 :: InitializeTrafficGenerator() {
	traffic_model = 1;
	GenerateTraffic();
}

#line 91 "queue_main.cc"
void compcxx_QueueSimulator_7 :: Setup(double simulation_time_console, int logs_enabled_console, 
	int queue_size_console, int batch_size_console, double lambda_console, 
	double mu_console, double timeout_mining_console, int seed_console,
	int forks_enabled_console, int n_miners_console, double capacity_p2p_console, 
	const char *script_output_filename){

	
	simulation_time = simulation_time_console;
	logs_enabled = logs_enabled_console;
	queue_size = queue_size_console;
	batch_size = batch_size_console;
	seed = seed_console;
	lambda = lambda_console;
	mu = mu_console;
	timeout_mining = timeout_mining_console;
	forks_enabled = forks_enabled_console;
	n_miners = n_miners_console;
	capacity_p2p = capacity_p2p_console;

	
	SetupEnvironment();

	
	PrintSystemInfo();

	
	script_output_file = fopen(script_output_filename, "at");	
	logger_script.save_logs = 1;
	logger_script.file = script_output_file;
	fprintf(logger_script.file, "QUEUE SIMULATION (seed %d)", seed);

}
#line 127 "queue_main.cc"
void compcxx_QueueSimulator_7 :: Start(){
	
}
#line 134 "queue_main.cc"
void compcxx_QueueSimulator_7 :: Stop(){

	if(logs_enabled) printf(" SIMULATION FINISHED\n");
	if(logs_enabled) printf("------------------------------------------\n");

	GenerateScriptOutput(logger_script, simulation_time, queue.performance,
		lambda, mu, timeout_mining, queue_size, batch_size);
	
	fclose(script_output_file);

}
#line 150 "queue_main.cc"
void compcxx_QueueSimulator_7 :: SetupEnvironment() {

	
	traffic_generator.p_compcxx_Queue_5=&queue /*connect traffic_generator.outportNewPacketGenerated,queue.InportNewPacketGenerated*/;

	
	traffic_generator.traffic_load = lambda; 
	traffic_generator.logs_enabled = logs_enabled;

	
	queue.simulation_time = simulation_time;
	queue.logs_enabled = logs_enabled;
	queue.queue_size = queue_size;
	queue.batch_size = batch_size;
	queue.departures_rate = mu;	
	queue.timeout_mining = timeout_mining;
	queue.n_miners = n_miners;
	queue.capacity_p2p = capacity_p2p;
	queue.InitializeVariables();

}





#line 175 "queue_main.cc"
void compcxx_QueueSimulator_7 :: PrintSystemInfo(){

	if(logs_enabled) printf("\n");
	if(logs_enabled) printf("*************************************************************************************\n");
	if(logs_enabled) printf(" BATCH-SERVICE QUEUE SIMULATOR FOR BLOCKCHAIN\n");
	if(logs_enabled) printf(" - Authors: Lorenza Giupponi & Francesc Wilhelmi\n");
	if(logs_enabled) printf(" - GitHub repository: https://bitbucket.org/francesc_wilhelmi/batch_service_queue_simulator\n");
	if(logs_enabled) printf("*************************************************************************************\n");
	if(logs_enabled) printf("\n");
	
	if(logs_enabled) printf(" - simulation_time = %f\n", simulation_time);
	if(logs_enabled) printf(" - queue_size = %d\n", queue_size);
	if(logs_enabled) printf(" - batch_size = %d\n", batch_size);
	if(logs_enabled) printf(" - lambda = %f\n", lambda);
	if(logs_enabled) printf(" - mu = %f\n", mu);
	if(logs_enabled) printf(" - timeout_mining = %f\n", timeout_mining);
	if(forks_enabled) {
		if(logs_enabled) printf(" - n_miners = %d\n", n_miners);
		if(logs_enabled) printf(" - capacity_p2p = %f\n", capacity_p2p);	
	}
	if(logs_enabled) printf(" - seed = %d\n", seed);
	if(logs_enabled) printf("\n");
}





#line 202 "queue_main.cc"
int main(int argc, char *argv[]){

	
	double simulation_time;
	int logs_enabled;
	int queue_size;
	int batch_size;
	double lambda;
	double mu;
	double timeout_mining;
	int seed;
	int forks_enabled;
	int n_miners;
	double capacity_p2p;
	std::string script_output_filename;

	
	if(argc == 9){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		seed = atoi(argv[8]);
		
		n_miners = 1;
		capacity_p2p = 0;
		forks_enabled = 0;
		script_output_filename.append("../output/script_output_default");
	} else if(argc == 10){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		seed = atoi(argv[8]);
		script_output_filename = ToString(argv[9]);
		
		n_miners = 1;
		capacity_p2p = 0;
		forks_enabled = 0;
	}else if(argc == 11){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		n_miners = atoi(argv[8]);
		capacity_p2p = atof(argv[9]);
		seed = atoi(argv[1]);
		forks_enabled = 1;
		
		script_output_filename.append("../output/script_output_default");
	} else if(argc == 12){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		n_miners = atoi(argv[8]);
		capacity_p2p = atof(argv[9]);
		seed = atoi(argv[10]);
		script_output_filename = ToString(argv[11]);
		forks_enabled = 1;
	} else {
		printf("- ERROR: The arguments provided were not properly set!\n "
			" + To execute the program, please introduce:\n"
			"    ./queue_main -simulation_time -logs_enabled -queue_size -batch_size -lambda -mu -timeout_mining -seed\n");
		return(-1);
	}

	
	compcxx_QueueSimulator_7 queue_simulation;
	queue_simulation.Seed = seed;
	srand(seed);
	queue_simulation.StopTime(simulation_time);
	queue_simulation.Setup(simulation_time, logs_enabled, queue_size, 
		batch_size, lambda, mu, timeout_mining, seed, forks_enabled,
		n_miners, capacity_p2p, script_output_filename.c_str());

	if(logs_enabled) printf("------------------------------------------\n");
	if(logs_enabled) printf("SIMULATION STARTED\n");

	queue_simulation.Run();

	return(0);
};
