//Things left-:
//write router-start script (done)
//Write router data to files (done)
//Handle router disapperance and re-appearance (write script for that)
//Pass packet and see it works properly (write inject script)
//we can improve the reset function

//TinyAODV (we didnt use this, reasons below)
//Didnt implement AODV. didnt see the need for it, especially with such a small no. of routers in our project. that would work better in a real
//life network. 1) Only affected nodes are informed. 2)AODV reduces the networkwide broadcasts to the extent possible.
//3)Whenever routes are not used -> get expired -> Discarded
//❍ Reduces stale routes
//❍ Reduces need for route maintenance
//AODV discovers routes as and when necessary
//1. Does not maintain routes from every node to every other
//2. Routes are maintained just as long as necessary
//we didnt need such high level sophisticated network, and so didnt implement aodv.


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

#define inf INT_MAX

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
 int portno;  // port number of the neighbour. 
 timespec startTime;
 sockaddr_in addr;
};


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
     return nameOf(m_my_id); 
 }

 void updateTable(const void *advertisement, char src) ;

 RoutingEntry  routeTo ( const char dest ) const 
 { 
     return  Table[indexOf(dest)]; 
 };

 std::vector<neighbour_info> neighbours() const 
 { 
     return m_neighbours; 
 };


 int portNoOf(char router);

 char nameOf(int index) const;

 int indexOf(char router) const;

 void initMyaddr(int portno);

 sockaddr_in myaddr() const 
 { 
     return m_myaddr; 
 }

 void aliveTime(neighbour_info &n);

 bool deadTimer(neighbour_info &n) const;

 int port() 
 { 
     return portNoOf(getmyID()); 
 }

private:
 // member variables
 int m_my_id;  // index   // change m_my_id to myId  
 int m_size;

 RoutingEntry  Table[max_routers]; // each router's distance vectors // can access private fields inside the constructor. 

 RoutingEntry  backupTable[max_routers]; // initial distance vectors (for resetting)

 std::vector<neighbour_info> m_neighbours; // port numbers of my_id's neighbours

 sockaddr_in m_myaddr;

 std::map<char, int> m_portnos;

 int min ( int initialCost, int startToPacketcost, int PacketToDestCost, bool &changed ) const;

 void print( RoutingEntry  dv[], char name, std::string msg, bool curr_time ) const;

};

struct header
{
 int type; // type of packet 
 char source;
 char dest;
 int length; // length of data 
};


enum type // 
{
 TYPE_DATA, TYPE_ADVERTISEMENT, TYPE_WAKEUP, TYPE_RESET
};

void *createPacket(int type, char source, char dest, int payloadLength, void *payload);

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

 vector<neighbour_info> neighbours = myrouter.neighbours();

 int myPort = myrouter.portNoOf(argv[2][0]); // my port

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
 void *dataPacket = createPacket ( TYPE_DATA, myrouter.getmyID() , 'D', strlen(data), (void*)data ) ; // strlen(data) will be the length of the payload as defined in createPacket

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
 
 wakeSelfUp ( myrouter, socketfd, TYPE_WAKEUP ) ;

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

 switch ( h.type )   // replace switch case with if-else 
 {

 case TYPE_DATA:  // 0 

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

 void *forwardPacket = createPacket ( TYPE_DATA, h.source, h.dest, h.length, (void*)payload ) ;

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

 break;

 case TYPE_ADVERTISEMENT:  // 1 

 RoutingEntry  Details[max_routers] ; // Array of Routing Table

 memcpy( (void*)Details, payload, h.length ) ; // fill this array with received routing table 
// now we have 2 routing tables 
// one of this router and another of the router that sent a packet. 


 for ( int i = 0; i < neighbours.size(); i++ )
{

  
 if ( neighbours[i].name == h.source )   // h.source is the router that sent the routing table in the packet , suppose its E
 // A has E as its neighbour , so this evaluates to true. 
 {
     myrouter.aliveTime( neighbours[i] ) ;  // store the current time. 
 }

 } 
 
 
 myrouter.updateTable(payload, h.source) ;  // update the distance vectors by passing the routing table data and source router that sent it. 
 
 
 break;



 case TYPE_WAKEUP: // perform periodic tasks  // 2 

 for ( int i = 0; i < neighbours.size(); i++ )
 {
 
 neighbour_info curneighbour = neighbours[i] ;
 
 if ( ( myrouter.getDetails()[ myrouter.indexOf(curneighbour.name)].Cost() != inf ) && myrouter.deadTimer( neighbours[i] ) )
 {
 wakeSelfUp ( myrouter, socketfd, TYPE_RESET, myrouter.getmyID(), neighbours[i].name, myrouter.getSize() / sizeof(RoutingEntry)  ) ; // removed - 2 
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


 break ;

 case TYPE_RESET:  // after 10 seconds 
 
 int hopcount = (int)h.length - 1 ;  // See how h.length is set up. 
 
 myrouter.reset(h.dest) ;

 if ( hopcount > 0 )  // See use of this.
 {
 
 void *forwardPacket = createPacket ( TYPE_RESET, myrouter.getmyID(), h.dest, hopcount, (void*)0 ) ;
 
 for ( int i = 0; i < neighbours.size(); i++ )
 {
 
 if ( neighbours[i].name != h.source )  // see this part 
 {
     sendto ( socketfd, forwardPacket, sizeof(header), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in) ) ;
 }

 }
 
 }
 
 break;
 
 }
 
 }
 
 free(rcvbuf);
 
 }

 }



Router::Router ( const char *file, const char *my_id )
{

 fstream topology(file);

 string line; // current line of file

 string field; // current token (to be put into entry's field)

 char my_idName = my_id[0]; // name of my_id

//cout<<"my_idName "<<my_idName <<endl;
 
 m_my_id = indexOf(my_id[0]); // index 

 // initialize m_Details

 for ( int dest = 0; dest <  max_routers; dest++ )  // set the default values 
 {
  Table[dest].setNextRouterName('0'); 
  Table[dest].setNextRouterPort(inf); // INT_MAX 
  Table[dest].setCost(inf);
  Table[dest].setAlive(); // router is alive.  // check whether here setAlive() will come or not
 }

 while ( getline(topology, line) ) 
 { // parse file line by line

 stringstream linestream(line);

 RoutingEntry  entry;  //  entry of a routing table

 entry.setAlive();

 // source router
 getline(linestream, field, ',');
 char name = field[0];
//cout<<"name "<<name<<endl;

 // destination router
 getline(linestream, field, ',');
//cout<<"field : "<<field <<endl; 

 int dest = indexOf(field[0]);  // index for A is 0 , B is 1 and so on. 

 neighbour_info n;

 n.name = field[0];
//cout<<"n.name "<<n.name<<endl ; 

 entry.setNextRouterName(field[0]);

 // destination port number
 getline(linestream, field, ',');
//cout<<" line " << line <<endl ;

 int port = atoi( field.c_str() ) ;

 entry.setNextRouterPort(port) ;

 n.portno = port;

 memset( (char *)&n.addr, 0, sizeof(n.addr) );

 n.addr.sin_family = AF_INET;
 n.addr.sin_addr.s_addr = inet_addr("127.0.0.1");

 n.addr.sin_port = htons(port); // converts unsigned short integer hostshort from host byte order to network byte order

 // link cost
 getline(linestream, field, ',');
//cout<<"field "<<field<<endl<<endl ;  

entry.setCost( atoi(field.c_str() ) );


 if (my_idName == 'H')  // my_idName is passed as a command line argument. 
 {

 int i ;

 for (i = 0; i < m_neighbours.size(); i++ )  // initially the size of this vector is 0. 
{

//cout<<"m_neighbour : "<<i<<" : "<<m_neighbours[i].name<<endl ;
//cout<<"n.name "<<n.name <<endl;

 if ( m_neighbours[i].name == n.name )   // vector of neighbours_info 
 {
     break;  // avoiding duplicate entries. 
 }

 }

 if ( i == m_neighbours.size() )
 {
      m_neighbours.push_back(n) ;
 }

 }

 else if ( name == my_idName )  // name is the source. my_idName is passed as command line argument. 
 {
 
 aliveTime(n) ; // store the current time for this neighbour 
 
 m_neighbours.push_back(n) ;     // store neighbour
 
  Table[dest] = entry ;      // this will work when the initial router is not 'H'  , other entries have default values. 
 
 }

 m_portnos[n.name] = n.portno ;  // stored in a map and this field is for every router .  //  

 }   // end of while loop. 

 // special port number for sending data packet
 
 m_portnos['H'] = 11111; // special port number for sending data. 

 memcpy ( (void*)backupTable , (void*) Table , sizeof( Table) ) ;

 if ( nameOf( m_my_id ) != 'H' )
 {
     print ( Table , nameOf(m_my_id) , "Initial routing table", true ) ; // print the initial routing table ( dv_info ) of this Router 
 }

}


void Router ::reset ( char dead ) // reset this Router if no other Router muticasts to this router after 10 seconds. 
{

 for (int i = 0; i < m_neighbours.size(); i++)
 {
 
 if ( m_neighbours[i].name == dead )
 {
 
 if ( backupTable[indexOf(dead)].Cost() != -1 )
 {
     backupTable[indexOf(dead)].setDead();
 }

 }
 
 }

 memcpy( (void*)Table, (void*)backupTable , sizeof(Table) ) ;  // copy contents of backup routing table into original one. 

 print( Table, nameOf(m_my_id), "Reset routing table", true ) ;
// print one by one 
// Change the printing format of the Routing Table. 


}

// updateTable this router's distance vector based on received advertisement from source
// return false ( value of changedDV ) if this router's distance vector was not changed

void Router :: updateTable ( const void *advertisementBuf, char source )
{
 
 RoutingEntry  originalTable[max_routers] ; // routing table of this router 
 
 memcpy ( (void*)originalTable, (void*) Table, sizeof( Table) ) ;

 bool changedDV = false ;

 int intermediate = indexOf(source) ;  // router that sent its routing table will become the intermediate router 
 
 // 
 
 if ( backupTable [intermediate].Dead() )
 {
 
 // before router E is turned on it is dead for Router A. 

 backupTable[intermediate].setAlive() ; 
 
  Table[intermediate].setAlive() ;

 changedDV = true;
 
 }

 // load advertised distance vector

 RoutingEntry  advertisement[max_routers] ; // store the routing table of the router that sent its routing table .

 memcpy ( (void*)advertisement, advertisementBuf, sizeof(advertisement) ) ; // copy the routing table of the router that sent it into this array. 

 // recalculate my_id's distance vector

 for ( int dest = 0; dest < max_routers; dest++ ) // Check for every destination because a router not reachable before could be reached now. 
 {

 if ( dest == m_my_id )
 {
     continue;
 }
 
 bool changedEntry = false;  // m_Details[] has the entry for every other router. 
/// will be true if the cost is changed from source to dest. 


// m_Details[i] is the (i+1)th entry of this router's  Routing Table.

 char a =  Table[dest].nextRouterName() ;  // a = 'B' ; when turn of B comes. // router E sends its routing table. 

  Table[dest].setCost ( min  ( Table[dest].Cost(),  Table[intermediate].Cost(), advertisement[dest].Cost(),  Table[dest].nextRouterName(), source, changedEntry ) ) ;
 
// In some iteration setting cost for A to B through E 

// m_Details[dest].Cost() : A to B initial cost // From A's routing table

// m_Details[intermediate].Cost() : A to E intial cost  /// From A's routing table 

// addvertisement[dest].Cost() : E to B initial cost ( from E's routing table )  

// m_Details[dest].nextRouterName() : Router B 

// source : Router E 


 if ( changedEntry )  // if the cost is changed then the Routing Table should be changed now.  
 {
 
 changedDV = true ;  // change m_Details to dv_entries 

 //if ( m_Details[indexOf(source)].nextRouterName()!= source )  
 //{

 //m_Details[dest].setNextRouterPort ( m_Details[indexOf(source)].nextRouterPort() ) ;
 //m_Details[dest].setNextRouterName ( m_Details[indexOf(source)].nextRouterName() ) ;

// if ( m_Details[dest].nextRouterName() == a )
// {
// changedDV = false ;
// }

// }

// else
// {
  Table[dest].setNextRouterPort( portNoOf(source) ) ; // If cost from A to B changes through E 
  Table[dest].setNextRouterName( source ) ;           // then A's next router is E and port_dest will be port numer of router E.  

 }

 }

 

  Table[intermediate].setCost ( advertisement[m_my_id].Cost() ) ;

 if (changedDV)
 {
 print ( originalTable, nameOf(m_my_id), "Change detected!\nRouting table before change", true ) ;
 print ( advertisement, source, "DV that caused the change", false ) ;
 print (  Table, nameOf(m_my_id), "Routing table after change", false ) ;
 }

}

// return index of router
int Router ::indexOf(char router) const
{
 return router - 'A';
}

// return name of indexed router

char Router ::nameOf(int index) const
{
 return (char)index + 'A';
}

// return port number of router

int Router :: portNoOf ( char router )
{
 return m_portnos[router];
}

void Router ::initMyaddr( int portno )
{
 memset((char *)&m_myaddr, 0, sizeof(m_myaddr));
 m_myaddr.sin_family = AF_INET;
 m_myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 m_myaddr.sin_port = htons(portno);
}

void Router::aliveTime( neighbour_info &n )
{
 clock_gettime(CLOCK_MONOTONIC, &n.startTime);
}

bool Router::deadTimer( neighbour_info &n ) const
{

 timespec tend= { 0 , 0 } ;

 clock_gettime(CLOCK_MONOTONIC, &tend);

 if  ( (  ( double) tend.tv_sec + 1.0e-9*tend.tv_nsec ) - ( (double)n.startTime.tv_sec + 1.0e-9*n.startTime.tv_nsec ) > 10 )
 {
     return true;
 }

 else
 {
     return false;
 }

}

//-----------------
// HELPER FUNCTIONS
//-----------------

// return minimum cost and set changed flag

int Router:: min ( int original_cost, int self_IntermediateCost , int intermediate_DestinationCost , bool &updated ) const
{

// Bellman Ford Algorithm 

 int newCost = self_IntermediateCost + intermediate_DestinationCost ;


 if ( self_IntermediateCost == inf || intermediate_DestinationCost == inf )  /// replace -1 with infinity.   
 {
 return original_cost ;
 }
 
 else if ( original_cost == inf || newCost < original_cost ) // if original Cost is infinity then return newCost.
 {
 updated = true ;
 return newCost ;
 }

 else if ( original_cost == newCost )
 {
 return original_cost;
 }
 
 else
 {
 return original_cost;
 }

}

// print a DV
// format: source, destination, port number of nexthop router, cost to destination

void Router::print ( RoutingEntry  myrouter[], char name, string msg, bool curr_time ) const  // dv : myrouter 
{
 
	cout <<"\t"<< msg << ": " << name << endl;

	if (curr_time)
	{
		time_t rawtime;			//declare time var
		time(&rawtime);			//store current time
		cout <<"\t"<< ctime(&rawtime);//convert time_t to string and 
								//print in time formmat
	}
	
    cout << " |Destination| Cost  |  Outgoing UDP  |Destination Port |" << endl;

	for ( int dest = 0; dest < max_routers; dest++ )
	{	
        if ( myrouter[dest].Cost != inf )
        {

			cout << " |     " << nameOf(dest) << "     | ";

			cout <<" "<< myrouter[dest].Cost <<"    |";
			
            cout <<"      "<< portNoOf( name )<< " "<< name <<"   |";  // see portNoOf

			cout <<"      "<< portNoOf(nameOf(dest) );
			
            //cout<< " -> "<<nameOf(dest);

			cout<<"      |\n";	
		
        }
		
	}
	cout << endl;

}

// create a packet with header and payload

// payloadlength is the size of the data. 

void *createPacket ( int type, char source, char dest, int payloadLength, void *payload ) // payload has the Routing Table data of every router when other than router 'H' is working. 
{

 int allocatedPayloadLength = payloadLength ;  // size of the routing table . 

 if ( ( type != TYPE_DATA ) && ( type != TYPE_ADVERTISEMENT ) )
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

 vector<neighbour_info> neighbours = myrouter.neighbours();

 for (int i = 0; i < neighbours.size(); i++)
 {

 void *sendPacket = createPacket ( TYPE_ADVERTISEMENT, myrouter.getmyID(), neighbours[i].name, myrouter.getSize(), (void*)myrouter.getDetails() ) ;

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
