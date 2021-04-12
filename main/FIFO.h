#include "transaction.h"
#include <deque>

/*
FIFO Class
*/

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


