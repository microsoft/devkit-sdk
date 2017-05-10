/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(MQTTASYNC_H)
#define MQTTASYNC_H

#include "FP.h"
#include "MQTTPacket.h"
#include "stdio.h"

namespace MQTT
{


enum QoS { QOS0, QOS1, QOS2 };


struct Message
{
    enum QoS qos;
    bool retained;
    bool dup;
    unsigned short id;
    void *payload;
    size_t payloadlen;
};


class PacketId
{
public:
    PacketId();
    
    int getNext();
   
private:
    static const int MAX_PACKET_ID = 65535;
    int next;
};

typedef void (*messageHandler)(Message*);

typedef struct limits
{
	int MAX_MQTT_PACKET_SIZE; // 
	int MAX_MESSAGE_HANDLERS;  // each subscription requires a message handler
	int MAX_CONCURRENT_OPERATIONS;  // each command which runs concurrently can have a result handler, when we are in multi-threaded mode
	int command_timeout_ms;
		
	limits()
	{
		MAX_MQTT_PACKET_SIZE = 100;
		MAX_MESSAGE_HANDLERS = 5;
		MAX_CONCURRENT_OPERATIONS = 1; // 1 indicates single-threaded mode - set to >1 for multithreaded mode
		command_timeout_ms = 30000;
	}
} Limits;
  

/**
 * @class Async
 * @brief non-blocking, threaded MQTT client API
 * @param Network a network class which supports send, receive
 * @param Timer a timer class with the methods: 
 */ 
template<class Network, class Timer, class Thread, class Mutex> class Async
{
    
public:    

	struct Result
	{
    	/* success or failure result data */
    	Async<Network, Timer, Thread, Mutex>* client;
		int rc;
	};

	typedef void (*resultHandler)(Result*);	
   
    Async(Network* network, const Limits limits = Limits()); 
        
    typedef struct
    {
        Async* client;
        Network* network;
    } connectionLostInfo;
    
    typedef int (*connectionLostHandlers)(connectionLostInfo*);
    
    /** Set the connection lost callback - called whenever the connection is lost and we should be connected
     *  @param clh - pointer to the callback function
     */
    void setConnectionLostHandler(connectionLostHandlers clh)
    {
        connectionLostHandler.attach(clh);
    }
    
    /** Set the default message handling callback - used for any message which does not match a subscription message handler
     *  @param mh - pointer to the callback function
     */
    void setDefaultMessageHandler(messageHandler mh)
    {
        defaultMessageHandler.attach(mh);
    }
           
    int connect(resultHandler fn, MQTTPacket_connectData* options = 0);
    
     template<class T>
    int connect(void(T::*method)(Result *), MQTTPacket_connectData* options = 0, T *item = 0);  // alternative to pass in pointer to member function
        
    int publish(resultHandler rh, const char* topic, Message* message);
    
    int subscribe(resultHandler rh, const char* topicFilter, enum QoS qos, messageHandler mh);
    
    int unsubscribe(resultHandler rh, const char* topicFilter);
    
    int disconnect(resultHandler rh);
    
private:

    void run(void const *argument);
    int cycle(int timeout);
    int waitfor(int packet_type, Timer& atimer);
	int keepalive();
	int findFreeOperation();

    int decodePacket(int* value, int timeout);
    int readPacket(int timeout);
    int sendPacket(int length, int timeout);
	int deliverMessage(MQTTString* topic, Message* message);
    
    Thread* thread;
    Network* ipstack;
    
    Limits limits;
    
    char* buf;  
    char* readbuf;

    Timer ping_timer, connect_timer;
    unsigned int keepAliveInterval;
	bool ping_outstanding;
    
    PacketId packetid;
    
    typedef FP<void, Result*> resultHandlerFP;    
    resultHandlerFP connectHandler; 
    
    typedef FP<void, Message*> messageHandlerFP;
    struct MessageHandlers
    {
    	const char* topic;
    	messageHandlerFP fp;
    } *messageHandlers;      // Message handlers are indexed by subscription topic
    
    // how many concurrent operations should we allow?  Each one will require a function pointer
    struct Operations
    {
    	unsigned short id;
    	resultHandlerFP fp;
    	const char* topic;         // if this is a publish, store topic name in case republishing is required
    	Message* message;    // for publish, 
    	Timer timer;         // to check if the command has timed out
    } *operations;           // result handlers are indexed by packet ids

	static void threadfn(void* arg);
	
	messageHandlerFP defaultMessageHandler;
    
    typedef FP<int, connectionLostInfo*> connectionLostFP;
    
    connectionLostFP connectionLostHandler;
    
};

}


template<class Network, class Timer, class Thread, class Mutex> void MQTT::Async<Network, Timer, Thread, Mutex>::threadfn(void* arg)
{
    ((Async<Network, Timer, Thread, Mutex>*) arg)->run(NULL);
}


template<class Network, class Timer, class Thread, class Mutex> MQTT::Async<Network, Timer, Thread, Mutex>::Async(Network* network, Limits limits)  : limits(limits), packetid()
{
	this->thread = 0;
	this->ipstack = network;
	this->ping_timer = Timer();
	this->ping_outstanding = 0;
	   
	// How to make these memory allocations portable?  I was hoping to avoid the heap
	buf = new char[limits.MAX_MQTT_PACKET_SIZE];
	readbuf = new char[limits.MAX_MQTT_PACKET_SIZE];
	this->operations = new struct Operations[limits.MAX_CONCURRENT_OPERATIONS];
	for (int i = 0; i < limits.MAX_CONCURRENT_OPERATIONS; ++i)
		operations[i].id = 0;
	this->messageHandlers = new struct MessageHandlers[limits.MAX_MESSAGE_HANDLERS];
	for (int i = 0; i < limits.MAX_MESSAGE_HANDLERS; ++i)
		messageHandlers[i].topic = 0;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::sendPacket(int length, int timeout)
{
    int sent = 0;
    
    while (sent < length)
        sent += ipstack->write(&buf[sent], length, timeout);
	if (sent == length)
	    ping_timer.countdown(this->keepAliveInterval); // record the fact that we have successfully sent the packet    
    return sent;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::decodePacket(int* value, int timeout)
{
    char c;
    int multiplier = 1;
    int len = 0;
	const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        rc = ipstack->read(&c, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (c & 127) * multiplier;
        multiplier *= 128;
    } while ((c & 128) != 0);
exit:
    return len;
}


/**
 * If any read fails in this method, then we should disconnect from the network, as on reconnect
 * the packets can be retried. 
 * @param timeout the max time to wait for the packet read to complete, in milliseconds
 * @return the MQTT packet type, or -1 if none
 */
template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::readPacket(int timeout) 
{
    int rc = -1;
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (ipstack->read(readbuf, 1, timeout) != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(&rem_len, timeout);
    len += MQTTPacket_encode(readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (ipstack->read(readbuf + len, rem_len, timeout) != rem_len)
        goto exit;

    header.byte = readbuf[0];
    rc = header.bits.type;
exit:
    return rc;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::deliverMessage(MQTTString* topic, Message* message)
{
	int rc = -1;

	// we have to find the right message handler - indexed by topic
	for (int i = 0; i < limits.MAX_MESSAGE_HANDLERS; ++i)
	{
		if (messageHandlers[i].topic != 0 && MQTTPacket_equals(topic, (char*)messageHandlers[i].topic))
		{
			messageHandlers[i].fp(message);
			rc = 0;
			break;
		}
	}
	
	return rc;
}



template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::cycle(int timeout)
{
    /* get one piece of work off the wire and one pass through */

    // read the socket, see what work is due
    int packet_type = readPacket(timeout);
    
	int len, rc;
    switch (packet_type)
    {
        case CONNACK:
			if (this->thread)
			{
				Result res = {this, 0};
            	if (MQTTDeserialize_connack(&res.rc, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
                	;
				connectHandler(&res);
				connectHandler.detach(); // only invoke the callback once
			}
			break;
        case PUBACK:
        	if (this->thread)
        		; //call resultHandler
        case SUBACK:
            break;
        case PUBLISH:
			MQTTString topicName;
			Message msg;
			rc = MQTTDeserialize_publish((int*)&msg.dup, (int*)&msg.qos, (int*)&msg.retained, (int*)&msg.id, &topicName,
								 (char**)&msg.payload, (int*)&msg.payloadlen, readbuf, limits.MAX_MQTT_PACKET_SIZE);;
			if (msg.qos == QOS0)
				deliverMessage(&topicName, &msg);
            break;
        case PUBREC:
   	        int type, dup, mypacketid;
   	        if (MQTTDeserialize_ack(&type, &dup, &mypacketid, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
   	            ; 
   	        // must lock this access against the application thread, if we are multi-threaded
			len = MQTTSerialize_ack(buf, limits.MAX_MQTT_PACKET_SIZE, PUBREL, 0, mypacketid);
		    rc = sendPacket(len, timeout); // send the PUBREL packet
			if (rc != len) 
				goto exit; // there was a problem

            break;
        case PUBCOMP:
            break;
        case PINGRESP:
			ping_outstanding = false;
            break;
    }
	keepalive();
exit:
    return packet_type;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::keepalive()
{
	int rc = 0;

	if (keepAliveInterval == 0)
		goto exit;

	if (ping_timer.expired())
	{
		if (ping_outstanding)
			rc = -1;
		else
		{
			int len = MQTTSerialize_pingreq(buf, limits.MAX_MQTT_PACKET_SIZE);
			rc = sendPacket(len, 1000); // send the ping packet
			if (rc != len) 
				rc = -1; // indicate there's a problem
			else
				ping_outstanding = true;
		}
	}

exit:
	return rc;
}


template<class Network, class Timer, class Thread, class Mutex> void MQTT::Async<Network, Timer, Thread, Mutex>::run(void const *argument)
{
	while (true)
		cycle(ping_timer.left_ms());
}


// only used in single-threaded mode where one command at a time is in process
template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::waitfor(int packet_type, Timer& atimer)
{
	int rc = -1;
	
	do
    {
		if (atimer.expired()) 
			break; // we timed out
	}
	while ((rc = cycle(atimer.left_ms())) != packet_type);	
	
	return rc;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::connect(resultHandler resultHandler, MQTTPacket_connectData* options)
{
	connect_timer.countdown(limits.command_timeout_ms);

    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    if (options == 0)
        options = &default_options; // set default options if none were supplied
    
    this->keepAliveInterval = options->keepAliveInterval;
	ping_timer.countdown(this->keepAliveInterval);
    int len = MQTTSerialize_connect(buf, limits.MAX_MQTT_PACKET_SIZE, options);
    int rc = sendPacket(len, connect_timer.left_ms()); // send the connect packet
	if (rc != len) 
		goto exit; // there was a problem
    
    if (resultHandler == 0)     // wait until the connack is received 
    {
        // this will be a blocking call, wait for the connack
		if (waitfor(CONNACK, connect_timer) == CONNACK)
		{
        	int connack_rc = -1;
        	if (MQTTDeserialize_connack(&connack_rc, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
	        	rc = connack_rc;
	    }
    }
    else
    {
        // set connect response callback function
        connectHandler.attach(resultHandler);
        
        // start background thread            
        this->thread = new Thread((void (*)(void const *argument))&MQTT::Async<Network, Timer, Thread, Mutex>::threadfn, (void*)this);
    }
    
exit:
    return rc;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::findFreeOperation()
{
	int found = -1;
	for (int i = 0; i < limits.MAX_CONCURRENT_OPERATIONS; ++i)
	{
		if (operations[i].id == 0)
		{
			found = i;
			break;
		}
	}
	return found;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::subscribe(resultHandler resultHandler, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{
	int index = 0;
	if (this->thread)
		index = findFreeOperation();	
	Timer& atimer = operations[index].timer;
	
	atimer.countdown(limits.command_timeout_ms);
    MQTTString topic = {(char*)topicFilter, 0, 0};
    
    int len = MQTTSerialize_subscribe(buf, limits.MAX_MQTT_PACKET_SIZE, 0, packetid.getNext(), 1, &topic, (int*)&qos);
    int rc = sendPacket(len, atimer.left_ms()); // send the subscribe packet
	if (rc != len) 
		goto exit; // there was a problem
    
    /* wait for suback */
    if (resultHandler == 0)
    {
        // this will block
        if (waitfor(SUBACK, atimer) == SUBACK)
        {
            int count = 0, grantedQoS = -1, mypacketid;
            if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
                rc = grantedQoS; // 0, 1, 2 or 0x80 
            if (rc != 0x80)
            {
            	for (int i = 0; i < limits.MAX_MESSAGE_HANDLERS; ++i)
				{
					if (messageHandlers[i].topic == 0)
					{
						messageHandlers[i].topic = topicFilter;
						messageHandlers[i].fp.attach(messageHandler);
						rc = 0;
						break;
					}
				}
            }
        }
    }
    else
    {
        // set subscribe response callback function
        
    }
    
exit:
    return rc;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::unsubscribe(resultHandler resultHandler, const char* topicFilter)
{
	int index = 0;
	if (this->thread)
		index = findFreeOperation();	
	Timer& atimer = operations[index].timer;

	atimer.countdown(limits.command_timeout_ms);
    MQTTString topic = {(char*)topicFilter, 0, 0};
    
    int len = MQTTSerialize_unsubscribe(buf, limits.MAX_MQTT_PACKET_SIZE, 0, packetid.getNext(), 1, &topic);
    int rc = sendPacket(len, atimer.left_ms()); // send the subscribe packet
	if (rc != len) 
		goto exit; // there was a problem
    
    // set unsubscribe response callback function
        
    
exit:
    return rc;
}


   
template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::publish(resultHandler resultHandler, const char* topicName, Message* message)
{
	int index = 0;
	if (this->thread)
		index = findFreeOperation();	
	Timer& atimer = operations[index].timer;

	atimer.countdown(limits.command_timeout_ms);
    MQTTString topic = {(char*)topicName, 0, 0};

	if (message->qos == QOS1 || message->qos == QOS2)
		message->id = packetid.getNext();
    
	int len = MQTTSerialize_publish(buf, limits.MAX_MQTT_PACKET_SIZE, 0, message->qos, message->retained, message->id, topic, (char*)message->payload, message->payloadlen);
    int rc = sendPacket(len, atimer.left_ms()); // send the subscribe packet
	if (rc != len) 
		goto exit; // there was a problem
    
    /* wait for acks */
    if (resultHandler == 0)
    {
 		if (message->qos == QOS1)
		{
	        if (waitfor(PUBACK, atimer) == PUBACK)
    	    {
    	        int type, dup, mypacketid;
    	        if (MQTTDeserialize_ack(&type, &dup, &mypacketid, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
    	            rc = 0; 
    	    }
		}
		else if (message->qos == QOS2)
		{
	        if (waitfor(PUBCOMP, atimer) == PUBCOMP)
	   	    {
	   	    	int type, dup, mypacketid;
            	if (MQTTDeserialize_ack(&type, &dup, &mypacketid, readbuf, limits.MAX_MQTT_PACKET_SIZE) == 1)
    	           	rc = 0; 
			}

		}
    }
    else
    {
        // set publish response callback function
        
    }
    
exit:
    return rc;
}


template<class Network, class Timer, class Thread, class Mutex> int MQTT::Async<Network, Timer, Thread, Mutex>::disconnect(resultHandler resultHandler)
{  
    Timer timer = Timer(limits.command_timeout_ms);     // we might wait for incomplete incoming publishes to complete
    int len = MQTTSerialize_disconnect(buf, limits.MAX_MQTT_PACKET_SIZE);
    int rc = sendPacket(len, timer.left_ms());   // send the disconnect packet
    
    return (rc == len) ? 0 : -1;
}



#endif