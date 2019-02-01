/* NTPClient.cpp */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "NTPClient.h"

#include "UDPSocket.h"

#include "mbed.h" //time() and set_time()
#include "def.h"

#define NTP_PORT 123
#define NTP_CLIENT_PORT 0 //Random port
#define NTP_TIMESTAMP_DELTA 2208988800ull //Diff btw a UNIX timestamp (Starting Jan, 1st 1970) and a NTP timestamp (Starting Jan, 1st 1900)

NTPClient::NTPClient(NetworkInterface *networkInterface)
    : m_sock(networkInterface), m_net(networkInterface)
{
}

NTPResult NTPClient::setTime(const char* host, uint16_t port, uint32_t timeout)
{
    //Create & bind socket
    m_sock.set_blocking(false);
    m_sock.set_timeout(timeout);
    
    struct NTPPacket pkt;

    //Now ping the server and wait for response
    //Prepare NTP Packet:
    pkt.li = 0; //Leap Indicator : No warning
    pkt.vn = 4; //Version Number : 4
    pkt.mode = 3; //Client mode
    pkt.stratum = 0; //Not relevant here
    pkt.poll = 0; //Not significant as well
    pkt.precision = 0; //Neither this one is

    pkt.rootDelay = 0; //Or this one
    pkt.rootDispersion = 0; //Or that one
    pkt.refId = 0; //...

    pkt.refTm_s = 0;
    pkt.origTm_s = 0;
    pkt.rxTm_s = 0;
    pkt.txTm_s = htonl( NTP_TIMESTAMP_DELTA + time(NULL) ); //WARN: We are in LE format, network byte order is BE

    pkt.refTm_f = pkt.origTm_f = pkt.rxTm_f = pkt.txTm_f = 0;

    SocketAddress outEndpoint(host, port);

    if(m_net->gethostbyname(host, &outEndpoint)!= 0)
    {
        m_sock.close();
        return NTP_DNS;    
    }
    
    //Set timeout, non-blocking and wait using select
    int ret = m_sock.sendto(outEndpoint, (char*)&pkt, sizeof(NTPPacket) );
    if (ret < 0 )
    {
        m_sock.close();
        return NTP_CONN;
    }
    
    SocketAddress recvAddress;
    recvAddress.set_ip_address(outEndpoint.get_ip_address());
    recvAddress.set_port(port);
    ret = m_sock.recvfrom(&recvAddress, (char*)&pkt, sizeof(NTPPacket) ); //FIXME need a DNS Resolver to actually compare the incoming address with the DNS name
    if (ret > 0)
    {
        if (strcmp(outEndpoint.get_ip_address(), recvAddress.get_ip_address()) != 0)
        {
          m_sock.close();
          return NTP_CONN;
        }
    }
    else
    {
        m_sock.close();
        return NTP_CONN;
    }

    if(ret < (int)sizeof(NTPPacket)) //TODO: Accept chunks
    {
        m_sock.close();
        return NTP_PRTCL;
    }

    if( pkt.stratum == 0)  //Kiss of death message : Not good !
    {
        m_sock.close();
        return NTP_PRTCL;
    }

    //Correct Endianness
    pkt.refTm_s = ntohl( pkt.refTm_s );
    pkt.refTm_f = ntohl( pkt.refTm_f );
    pkt.origTm_s = ntohl( pkt.origTm_s );
    pkt.origTm_f = ntohl( pkt.origTm_f );
    pkt.rxTm_s = ntohl( pkt.rxTm_s );
    pkt.rxTm_f = ntohl( pkt.rxTm_f );
    pkt.txTm_s = ntohl( pkt.txTm_s );
    pkt.txTm_f = ntohl( pkt.txTm_f );

    //Compute offset, see RFC 4330 p.13
    uint32_t destTm_s = (NTP_TIMESTAMP_DELTA + time(NULL));
    int64_t offset = ( (int64_t)( pkt.rxTm_s - pkt.origTm_s ) + (int64_t) ( pkt.txTm_s - destTm_s ) ) / 2; //Avoid overflow
    //Set time accordingly
    set_time( time(NULL) + offset );
    
    m_sock.close();

    return NTP_OK;
}

