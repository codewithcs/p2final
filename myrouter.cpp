#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

#define inf INT8_MAX

#define buffer_size 2048
#define max_routers 6

using namespace std;

struct RoutingEntry  // dv_info is the Routing Entry.
{

public:
// function and variable names cannot be same

 int nextRouterPort() const
 {
     return (Dead() ? -1 : port_dest  );
 }

 char nextRouterName() const
 {
     return (Dead() ? '0' : nextroutername  );
 }

 int Cost() const
 {
     return (Dead() ? -1 : cost);
 }

 bool Dead() const
 {
     return dead;
 }

 void setNextRouterPort(int n)
 {
     port_dest = n;
 }

 void setNextRouterName(char n)
 {
     nextroutername = n;
 }

 void setCost(int c)
 {
     cost = c;
 }

 void setAlive()
 {
     dead = false;
 }

 void setDead()
 {
     dead = true;
 }

private:
 bool dead ;
 int port_dest ; // port number of next hop router ; replace nextrouterport with port_dest
 char nextroutername ;
 int cost ; // link cost to destination

};

struct neighbour_info
{
 char name;
  // port number of the neighbour.
sockaddr_in addr;
timespec startTime; int portno;
};

// replace m_neighbours by neghbours.


// change DV to Router
// dv_info to RoutingEntry // each routing entry stored in an array.
//

// replace m_Details with Table and m_Details_backup with backupTable

class Router
{

public:

 Router() {}

 Router ( const char *file, const char *my_id ) ;

 ~Router() {}

 void reset(char dead);

 RoutingEntry  *getDetails()
 {
     return Table ;
 }

 int getSize() const
 {
     return sizeof( Table);
 }

 char getmyID() const
 {
     return nameOf(myIndex);
 }

 void updateTable ( const void *advertisement, char src ) ;

 RoutingEntry  routeTo ( const char dest ) const
 {
     return  Table[indexOf(dest)];
 };

 std::vector<neighbour_info> getNeighbours() const
 {
     return neighbours;
 };


 int portNo(char router);

 char nameOf(int index) const;

 int indexOf(char router) const;

 void initMyaddr(int portno);

 sockaddr_in myaddr() const
 {
     return myAddress;
 }

 void startTimer ( neighbour_info &n ) ; // store the time in the startTime field of n .

 bool timeExpired ( neighbour_info &n ) const; // calculates time elapsed since start of this router.

 int port()
 {
     return portNo(getmyID());
 }

private:
 // member variables
 int myIndex ;  // index   // change m_my_id to myIndex
 int mySize;  // replace m_size by mySize

 RoutingEntry  Table[max_routers]; // each router's distance vectors // can access private fields inside the constructor.

 RoutingEntry  backupTable[max_routers]; // initial distance vectors (for resetting)

 std::vector<neighbour_info> neighbours; // port numbers of my_id's neighbours

 sockaddr_in myAddress ; //  myAddress

 std::map<char, int> portnos; // portnos

 int min ( int initialCost, int startToPacketcost, int PacketToDestCost, bool &changed ) const;

 void print( RoutingEntry  dv[], char name, std::string msg, bool curr_time ) ; // removed const from here

};

struct header
{
 int type; // type of packet
 char source;
 char dest;
 int length; // length of data
};

enum
{
    DATA, ADVERTISEMENT, WAKEUP, RESET
};

// TYPE_DATA - 0 , TYPE_ADVERTISEMENT - 1 , TYPE_WAKEUP - 2 , TYPE_RESET - 3


void *createPacket(int type, char source, char dest, int payloadLength, void *payload);
// Packet consists of Header and Payload ( original data ).


header getHeader(void *packet);

void *getPayload(void *packet, int length);

void multicast( Router &dv, int socketfd);

void wakeSelfUp ( Router &dv, int socketfd, int type, char source = 0, char dest = 0, int payloadLength = 0, void *payload = 0 )   ;

// using default arguments.



int main(int argc, char **argv)
{
 // check for errors
 if (argc < 3)
 {
 perror("Not enough arguments.\nUsage: ./my_router <initialization file> <router name>\n");
 return 0;
 }

 Router  myrouter (argv[1], argv[2]);  // replace dv with myrouter and DV with Router , dv_info with RoutingEntry

 vector<neighbour_info> neighbours = myrouter.getNeighbours();

 int myPort = myrouter.portNo(argv[2][0]); // my port

 myrouter.initMyaddr(myPort); // initialise m_myaddr in DV class . // port number of 'H' is 11111.

 sockaddr_in myaddr = myrouter.myaddr(); // return m_myaddr s

 socklen_t addrlen = sizeof(sockaddr_in); // length of addresses

 // create a UDP socket

 int socketfd; // our socket file descriptor ; used to access network socket.

 if ( ( socketfd = socket ( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
 {
 perror("cannot create socket\n");
 return 0;
 }

 // bind the socket to localhost and myPort

 if ( bind ( socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)  ) < 0 )
 {
 perror("bind failed");
 return 0;
 }

 // send a data packet to router A

 if ( myrouter.getmyID() == 'H' )  // returns m_my_id
 {

 char data[100];
 memset(data, 0, 100);

 cin.getline(data, 100);  // input the Data.

 for ( int i = 0; i < neighbours.size(); i++ )
 {

 if ( neighbours[i].name == 'A' )  // Send a packet from H to D through A.
 {
 void *dataPacket = createPacket ( DATA, myrouter.getmyID() , 'D', strlen(data), (void*)data ) ; // strlen(data) will be the length of the payload as defined in createPacket

 sendto ( socketfd , dataPacket, sizeof(header) + myrouter.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in ) ) ;

 // print info
 header h = getHeader(dataPacket);

 printf("Sent data packet\n");
 printf("Type: data\n");
 printf("Source: %c\n",h.source);
 printf("Destination: %c\n", h.dest);
 printf("Length of packet: %ld\n",(sizeof(header) + h.length));
 printf("Length of payload: %ld\n",(long int) h.length);
 cout << "Payload: " << data << endl;

 free(dataPacket);

 }

 }

 exit(0);

 }

//fork() can be used to create a new process, known as a child process. This child is initially a copy of the the parent, but can be used to run
//a different branch of the program or even execute a completely different program. After forking, child and parent processes run in parallel.
//Any variables local to the parent process will have been copied for the child process, so updating a variable in one process will not affect
//the other
//we forked the program and maintained two separate processes for each router. While the parent process uses a receive-and-then-send loop,
//the child process periodically sends a packet to the parent process. That way, the parent process can wake up periodically and send
//advertisements to it neighbors.

 // distance vector routing
 int thread_id = fork() ;

cout<<"thread id is "<<thread_id <<endl ;

 if ( thread_id < 0 )
 {
 perror("fork failed") ;
 return 0 ;
 }

 else if ( thread_id == 0 )   // this will be true for the child process.
 {

// send to each neighbour periodically

 for ( ; ; )
 {
 // periodically wake up parent process //fork is use to create a child process to wake itself up and then send dv to each neighbour

 cout<<"Inside the infinite loop and thread id : "<<thread_id <<endl;

 wakeSelfUp ( myrouter, socketfd, WAKEUP ) ;

 sleep(1);

 }

 }


 else   // parent process will run this.
 {
      // listen for advertisements // when some other router runs it gets its advertisement.

 void *rcvbuf = malloc(buffer_size);  // 2048 is the buffer size.

 sockaddr_in remaddr;

 for ( ; ; )
 {

 memset ( rcvbuf, 0, buffer_size ) ;

// receive from Router process running on another terminal
// sendto() was called inside the Constructor.


 int recvlen = recvfrom ( socketfd, rcvbuf , buffer_size, 0, (struct sockaddr *)&remaddr, &addrlen ) ;
// returns the length of the message on successful completion.

 header h = getHeader(rcvbuf) ;

 void *payload = getPayload ( rcvbuf, h.length ) ; // payload is the routing table
 // initially the header type would be of wakeup ( 2 ) type.

cout<< h.type <<endl ;   // prints 2 initially , if no router comes this will become 3 after 10s

// if only 1 router is turned on then no message is really received by it. So h.type = 2, h.length=0  and h.source would be null.
// For Router A , myPort = 10000
// portnos are stored in the map .



if ( h.type == 0 )  /// Packet Generator
{
 cout << "Received data packet" << endl ;

 time_t rawtime;
 time(&rawtime);
 cout << "Timestamp: " << ctime(&rawtime) ;

 cout << "ID of source neighbour_info: " << h.source << endl;
 cout << "ID of destination neighbour_info: " << h.dest << endl;
 cout << "UDP port in which the packet arrived: " << myPort << endl; // port number of this router.

 if ( h.dest != myrouter.getmyID() )
 {
     // only forward if this router is not the destination

 if ( myrouter.routeTo(h.dest).nextRouterPort() == -1 )
 {
 cout << "Error: packet could not be forwarded" << endl;
 }

 else
 {

 cout << "UDP port along which the packet was forwarded: " << myrouter.routeTo(h.dest).nextRouterPort() << endl;

 cout << "Router the packet was forwarded to: " << myrouter.routeTo(h.dest).nextRouterName() << endl;

 void *forwardPacket = createPacket ( DATA, h.source, h.dest, h.length, (void*)payload ) ;

 for ( int i = 0; i < neighbours.size(); i++ )
 {

 if ( neighbours[i].name == myrouter.routeTo(h.dest).nextRouterName() )
 {
     sendto ( socketfd, forwardPacket, sizeof(header) + myrouter.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in) ) ;
 }

 }

 free(forwardPacket) ;

 }

 cout << endl;

 }

 else   // Destination Router, Extract Data.
 {

 char data[100];

 memset(data, 0, 100);

 memcpy( (void*)data, payload, h.length ) ;

 cout << "Data payload: " << data << endl << endl; // payload is the actual data.

 }

}

else if ( h.type == 1 )
{
// case TYPE_ADVERTISEMENT:  // 1

 RoutingEntry  Details[max_routers] ; // Array of Routing Table

 memcpy( (void*)Details, payload, h.length ) ; // fill this array with received routing table
// now we have 2 routing tables
// one of this router and another of the router that sent a packet.


 for ( int i = 0; i < neighbours.size(); i++ )
{


 if ( neighbours[i].name == h.source )   // h.source is the router that sent the routing table in the packet , suppose its E
 // A has E as its neighbour , so this evaluates to true.
 {
     myrouter.startTimer ( neighbours[i] ) ;  // store the current time.
 }

 }


 myrouter.updateTable(payload, h.source) ;  // update the distance vectors by passing the routing table data and source router that sent it.


 }


else if ( h.type == 2 )
{
// case TYPE_WAKEUP: // perform periodic tasks  // 2

 for ( int i = 0; i < neighbours.size(); i++ )
 {

 neighbour_info curneighbour = neighbours[i] ;

 if ( ( myrouter.getDetails()[ myrouter.indexOf(curneighbour.name)].Cost() != inf ) && myrouter.timeExpired( neighbours[i] ) )
 {
 wakeSelfUp ( myrouter, socketfd, RESET, myrouter.getmyID(), neighbours[i].name, myrouter.getSize() / sizeof(RoutingEntry)  ) ; // removed - 2
 }

// entries would be reset one by one for the neighbours of this Router.
// If A's neighbours are B and E then first the entry for B will be reset and then for router E.

// This resets the Routing Table of this Router
// deadTimer will be true if this ROuter has lived for more than 10s.


 }

 multicast( myrouter , socketfd) ;

// the child process calls wakeSelfUp which creates a packet with type: TYPE_WAKEUP and no data.
// The parent process receives it and depending on the type of data packet
// // if its just a wake up call it multicasts its routing table to all the routers.


}

 //case TYPE_RESET:  // after 10 seconds

else if ( h.type == 3 )
{
 int hopcount = (int)h.length - 1 ;  // See how h.length is set up.

 myrouter.reset(h.dest) ;

 if ( hopcount > 0 )  // See use of this.
 {

 void *forwardPacket = createPacket ( RESET, myrouter.getmyID(), h.dest, hopcount, (void*)0 ) ;

 for ( int i = 0; i < neighbours.size(); i++ )
 {

 if ( neighbours[i].name != h.source )  // see this part
 {
     sendto ( socketfd, forwardPacket, sizeof(header), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in) ) ;
 }

 }

 }

}

//break;

 }



 free(rcvbuf);

 }

}


void *createPacket ( int type, char source, char dest, int payloadLength, void *payload ) // payload has the Routing Table data of every router when other than router 'H' is working.
{

 int allocatedPayloadLength = payloadLength ;  // size of the routing table .

 if ( ( type != DATA ) && ( type != ADVERTISEMENT ) )
 {
     allocatedPayloadLength = 0;
 }

 // create empty packet
 void *packet = malloc ( sizeof(header) + allocatedPayloadLength ) ;

 // create header
 header h;

 h.type = type;

 h.source = source;   // current router.

 h.dest = dest;   // when multicasting destination is all other directly connected routers.

 h.length = payloadLength ; // length of data .

 // fill in packet
 memcpy ( packet, (void*)&h, sizeof(header) ) ;

 memcpy( (void*)( (char*)packet+sizeof(header) ), payload, allocatedPayloadLength ) ;
// move the pointer

// attach header with the actual data ( payload )

 return packet;

}

// extract the header from the packet
header getHeader ( void *packet )
{
 header h;
 memcpy( (void*)&h, packet, sizeof(header) ) ;
 return h;
}

// extract the payload from the packet
void *getPayload ( void *packet, int length )
{
 void *payload = malloc(length) ;
 memcpy ( payload, (void*)( (char*)packet + sizeof(header) ), length ) ;
 // shift pointer to the beginning of the payload.
 return payload;
}

// multicast advertisement to all neighbours
void multicast( Router &myrouter, int socketfd)
{

 vector<neighbour_info> neighbours = myrouter.getNeighbours();

 for (int i = 0; i < neighbours.size(); i++)
 {

 void *sendPacket = createPacket ( ADVERTISEMENT, myrouter.getmyID(), neighbours[i].name, myrouter.getSize(), (void*)myrouter.getDetails() ) ;

 sendto ( socketfd, sendPacket, sizeof(header) + myrouter.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in) ) ;

 free(sendPacket);

 }


 // Packet gets sent through multicasting .
 // After Router A is turned on and now if we turn on Router B then Router B sends its Routing Table to all the active routers.
 // Suppose only Router A is turned on,
 // Router A has something to receive now.
// This is received by parent process of some other Router.


}

// periodically wake yourmy_id up to multicast advertisement

void wakeSelfUp ( Router &myrouter, int socketfd, int type, char source, char dest, int payloadLength, void *payload )
{

 void *sendPacket = createPacket(type, source, dest, payloadLength, payload); // only type is valid and rest would be default values

 sockaddr_in destAddr = myrouter.myaddr();

 sendto(socketfd, sendPacket, sizeof(header), 0, (struct sockaddr *)&destAddr, sizeof(sockaddr_in));

 free(sendPacket);

}
