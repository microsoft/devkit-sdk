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

#include "StackTrace.h"
#include "MQTTPacket.h"

#include <string.h>

/**
 * Encodes the message length according to the MQTT algorithm
 * @param buf the buffer into which the encoded data is written
 * @param length the length to be encoded
 * @return the number of bytes written to buffer
 */
int MQTTPacket_encode(unsigned char* buf, int length)
{
	int rc = 0;

	FUNC_ENTRY;
	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
 * Decodes the message length according to the MQTT algorithm
 * @param getcharfn pointer to function to read the next character from the data source
 * @param value the decoded length returned
 * @return the number of bytes read from the socket
 */
int MQTTPacket_decode(int (*getcharfn)(unsigned char*, int), int* value)
{
	unsigned char c;
	int multiplier = 1;
	int len = 0;
#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

	FUNC_ENTRY;
	*value = 0;
	do
	{
		int rc = MQTTPACKET_READ_ERROR;

		if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
		{
			rc = MQTTPACKET_READ_ERROR;	/* bad data */
			goto exit;
		}
		rc = (*getcharfn)(&c, 1);
		if (rc != 1)
			goto exit;
		*value += (c & 127) * multiplier;
		multiplier *= 128;
	} while ((c & 128) != 0);
exit:
	FUNC_EXIT_RC(len);
	return len;
}


int MQTTPacket_len(int rem_len)
{
	rem_len += 1; /* header byte */

	/* now remaining_length field */
	if (rem_len < 128)
		rem_len += 1;
	else if (rem_len < 16384)
		rem_len += 2;
	else if (rem_len < 2097151)
		rem_len += 3;
	else
		rem_len += 4;
	return rem_len;
}


static unsigned char* bufptr;

int bufchar(unsigned char* c, int count)
{
	int i;

	for (i = 0; i < count; ++i)
		*c = *bufptr++;
	return count;
}


int MQTTPacket_decodeBuf(unsigned char* buf, int* value)
{
	bufptr = buf;
	return MQTTPacket_decode(bufchar, value);
}


/**
 * Calculates an integer from two bytes read from the input buffer
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the integer value calculated
 */
int readInt(unsigned char** pptr)
{
	unsigned char* ptr = *pptr;
	int len = 256*(*ptr) + (*(ptr+1));
	*pptr += 2;
	return len;
}


/**
 * Reads one character from the input buffer.
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the character read
 */
char readChar(unsigned char** pptr)
{
	char c = **pptr;
	(*pptr)++;
	return c;
}


/**
 * Writes one character to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param c the character to write
 */
void writeChar(unsigned char** pptr, char c)
{
	**pptr = c;
	(*pptr)++;
}


/**
 * Writes an integer as 2 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void writeInt(unsigned char** pptr, int anInt)
{
	**pptr = (unsigned char)(anInt / 256);
	(*pptr)++;
	**pptr = (unsigned char)(anInt % 256);
	(*pptr)++;
}


/**
 * Writes a "UTF" string to an output buffer.  Converts C string to length-delimited.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param string the C string to write
 */
void writeCString(unsigned char** pptr, const char* string)
{
	int len = strlen(string);
	writeInt(pptr, len);
	memcpy(*pptr, string, len);
	*pptr += len;
}


int getLenStringLen(char* ptr)
{
	int len = 256*((unsigned char)(*ptr)) + (unsigned char)(*(ptr+1));
	return len;
}


void writeMQTTString(unsigned char** pptr, MQTTString mqttstring)
{
	if (mqttstring.lenstring.len > 0)
	{
		writeInt(pptr, mqttstring.lenstring.len);
		memcpy(*pptr, mqttstring.lenstring.data, mqttstring.lenstring.len);
		*pptr += mqttstring.lenstring.len;
	}
	else if (mqttstring.cstring)
		writeCString(pptr, mqttstring.cstring);
	else
		writeInt(pptr, 0);
}


/**
 * @param mqttstring the MQTTString structure into which the data is to be read
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param enddata pointer to the end of the data: do not read beyond
 * @return 1 if successful, 0 if not
 */
int readMQTTLenString(MQTTString* mqttstring, unsigned char** pptr, unsigned char* enddata)
{
	int rc = 0;

	FUNC_ENTRY;
	/* the first two bytes are the length of the string */
	if (enddata - (*pptr) > 1) /* enough length to read the integer? */
	{
		mqttstring->lenstring.len = readInt(pptr); /* increments pptr to point past length */
		if (&(*pptr)[mqttstring->lenstring.len] <= enddata)
		{
			mqttstring->lenstring.data = (char*)*pptr;
			*pptr += mqttstring->lenstring.len;
			rc = 1;
		}
	}
	mqttstring->cstring = NULL;
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
 * Return the length of the MQTTstring - C string if there is one, otherwise the length delimited string
 * @param mqttstring the string to return the length of
 * @return the length of the string
 */
int MQTTstrlen(MQTTString mqttstring)
{
	int rc = 0;

	if (mqttstring.cstring)
		rc = strlen(mqttstring.cstring);
	else
		rc = mqttstring.lenstring.len;
	return rc;
}


/**
 * Compares an MQTTString to a C string
 * @param a the MQTTString to compare
 * @param bptr the C string to compare
 * @return boolean - equal or not
 */
int MQTTPacket_equals(MQTTString* a, char* bptr)
{
	int alen = 0,
		blen = 0;
	char *aptr;
	
	if (a->cstring)
	{
		aptr = a->cstring;
		alen = strlen(a->cstring);
	}
	else
	{
		aptr = a->lenstring.data;
		alen = a->lenstring.len;
	}
	blen = strlen(bptr);
	
	return (alen == blen) && (strncmp(aptr, bptr, alen) == 0);
}


/**
 * Helper function to read packet data from some source into a buffer
 * @param buf the buffer into which the packet will be serialized
 * @param buflen the length in bytes of the supplied buffer
 * @param getfn pointer to a function which will read any number of bytes from the needed source
 * @return integer MQTT packet type, or -1 on error
 */
int MQTTPacket_read(unsigned char* buf, int buflen, int (*getfn)(unsigned char*, int))
{
	int rc = -1;
	MQTTHeader header = {0};
	int len = 0;
	int rem_len = 0;

	/* 1. read the header byte.  This has the packet type in it */
	if ((*getfn)(buf, 1) != 1)
		goto exit;

	len = 1;
	/* 2. read the remaining length.  This is variable in itself */
	MQTTPacket_decode(getfn, &rem_len);
	len += MQTTPacket_encode(buf + 1, rem_len); /* put the original remaining length back into the buffer */

	/* 3. read the rest of the buffer using a callback to supply the rest of the data */
	if ((*getfn)(buf + len, rem_len) != rem_len)
		goto exit;

	header.byte = buf[0];
	rc = header.bits.type;
exit:
	return rc;
}


const char* MQTTPacket_names[] =
{
	"RESERVED", "CONNECT", "CONNACK", "PUBLISH", "PUBACK", "PUBREC", "PUBREL",
	"PUBCOMP", "SUBSCRIBE", "SUBACK", "UNSUBSCRIBE", "UNSUBACK",
	"PINGREQ", "PINGRESP", "DISCONNECT"
};


char* MQTTPacket_toString(char* strbuf, int strbuflen, unsigned char* buf, int buflen)
{
	int index = 0;
	int rem_length = 0;
	MQTTHeader header = {0};
	int strindex = 0;

	header.byte = buf[index++];
	index += MQTTPacket_decodeBuf(&buf[index], &rem_length);

	switch (header.bits.type)
	{
	case CONNECT:
	{
		MQTTPacket_connectData data;
		if (MQTTDeserialize_connect(&data, buf, buflen) == 1)
		{
			strindex = snprintf(strbuf, strbuflen,
				"CONNECT MQTT version %d, client id %.*s, clean session %d, keep alive %hd",
				(int)data.MQTTVersion, data.clientID.lenstring.len, data.clientID.lenstring.data,
				(int)data.cleansession, data.keepAliveInterval);
			if (data.willFlag)
				strindex += snprintf(&strbuf[strindex], strbuflen - strindex,
				", will QoS %d, will retain %d, will topic %.*s, will message %.*s",
				data.will.qos, data.will.retained,
				data.will.topicName.lenstring.len, data.will.topicName.lenstring.data,
				data.will.message.lenstring.len, data.will.message.lenstring.data);
			if (data.username.lenstring.data && data.username.lenstring.len > 0)
			{
				printf("user name\n");
				strindex += snprintf(&strbuf[strindex], strbuflen - strindex,
				", user name %.*s", data.username.lenstring.len, data.username.lenstring.data);
			}
			if (data.password.lenstring.data && data.password.lenstring.len > 0)
				strindex += snprintf(&strbuf[strindex], strbuflen - strindex,
				", password %.*s", data.password.lenstring.len, data.password.lenstring.data);
		}
	}
	break;
	case CONNACK:
	{
		unsigned char sessionPresent, connack_rc;
		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
			"CONNACK session present %d, rc %d", sessionPresent, connack_rc);
	}
	break;
	case PUBLISH:
	{
		unsigned char dup, retained, *payload;
		unsigned short packetid;
		int qos, payloadlen;
		MQTTString topicName = MQTTString_initializer;
		if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName,
				&payload, &payloadlen, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"PUBLISH dup %d, QoS %d, retained %d, packet id %d, topic %.*s, payload length %d, payload %.*s",
				dup, qos, retained, packetid,
				(topicName.lenstring.len < 20) ? topicName.lenstring.len : 20, topicName.lenstring.data,
				payloadlen, (payloadlen < 20) ? payloadlen : 20, payload);
	}
	break;
	case PUBACK:
	case PUBREC:
	case PUBREL:
	case PUBCOMP:
	{
		unsigned char packettype, dup;
		unsigned short packetid;
		if (MQTTDeserialize_ack(&packettype, &dup, &packetid, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"%s dup %d, packet id %d",
				MQTTPacket_names[packettype], dup, packetid);
	}
	break;
	case SUBSCRIBE:
	{
		unsigned char dup;
		unsigned short packetid;
		int maxcount = 1, count = 0;
		MQTTString topicFilters[1];
		int requestedQoSs[1];
		if (MQTTDeserialize_subscribe(&dup, &packetid, maxcount, &count,
				topicFilters, requestedQoSs, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"SUBSCRIBE dup %d, packet id %d count %d topic %.*s qos %d",
				dup, packetid, count,
				topicFilters[0].lenstring.len, topicFilters[0].lenstring.data,
				requestedQoSs[0]);
	}
	break;
	case SUBACK:
	{
		unsigned short packetid;
		int maxcount = 1, count = 0;
		int grantedQoSs[1];
		if (MQTTDeserialize_suback(&packetid, maxcount, &count, grantedQoSs, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"SUBACK packet id %d count %d granted qos %d",
				packetid, count, grantedQoSs[0]);
	}
	break;
	case UNSUBSCRIBE:
	{
		unsigned char dup;
		unsigned short packetid;
		int maxcount = 1, count = 0;
		MQTTString topicFilters[1];
		if (MQTTDeserialize_unsubscribe(&dup, &packetid, maxcount, &count, topicFilters, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"UNSUBSCRIBE dup %d, packet id %d count %d topic %.*s",
				dup, packetid, count,
				topicFilters[0].lenstring.len, topicFilters[0].lenstring.data);
	}
	break;
	case UNSUBACK:
	{
		unsigned short packetid;
		if (MQTTDeserialize_unsuback(&packetid, buf, buflen) == 1)
			strindex = snprintf(strbuf, strbuflen,
				"UNSUBACK packet id %d", packetid);
	}
	break;
	case PINGREQ:
	case PINGRESP:
	case DISCONNECT:
		strindex = snprintf(strbuf, strbuflen, "%s", MQTTPacket_names[header.bits.type]);
		break;
	}
	return strbuf;
}
