



Router::Router ( const char *file, const char *my_id )
{
/// also do modifications in variable names here.

 fstream topology(file);

 string currentline; // current line of file

 string currentfield; // current token (to be put into entry's field)

 char my_idName = my_id[0]; // name of my_idt // A

//cout<<"my_idName "<<my_idName <<endl;

 myIndex = indexOf(my_id[0]); // index

 // initialize m_Details

 for ( int dest = 0; dest <  max_routers; dest++ )  // set the default values
 {
  Table[dest].setNextRouterName('0');
  Table[dest].setNextRouterPort(inf); // INT8_MAX
  Table[dest].setCost(inf);
  Table[dest].setAlive(); // router is alive.  // check whether here setAlive() will come or not
 }

 while ( getline(topology, currentline) )
 { // parse file line by line

 stringstream linestream(currentline);

 RoutingEntry  entry;  //  entry of a routing table

 entry.setAlive();

 // source router
 getline(linestream, currentfield, ',');

 char name = currentfield[0];  // A
/
/cout<<"name "<<name<<endl;

 // destination router
 getline(linestream, currentfield, ',');
//cout<<"field : "<<field <<endl;

 int dest = indexOf(currentfield[0]);  // index for A is 0 , B is 1 and so on.

 neighbour_info n;

 n.name = currentfield[0];
//cout<<"n.name "<<n.name<<endl ;

 entry.setNextRouterName(currentfield[0]);

 // destination port number
 getline(linestream, currentfield, ',');
//cout<<" line " << line <<endl ;

 int port = atoi( currentfield.c_str() ) ;  // converts string to integer 

 entry.setNextRouterPort(port) ;

 n.portno = port;

 memset( (char *)&n.addr, 0, sizeof(n.addr) );

 n.addr.sin_family = AF_INET;
 n.addr.sin_addr.s_addr = inet_addr("127.0.0.1");

 n.addr.sin_port = htons(port); // converts unsigned short integer hostshort from host byte order to network byte order

 // link cost
 getline(linestream, currentfield, ',');
//cout<<"field "<<field<<endl<<endl ;

entry.setCost( atoi( currentfield.c_str() ) );


 if (my_idName == 'H')  // my_idName is passed as a command line argument.
 {

 int i ;

 for (i = 0; i < neighbours.size(); i++ )  // initially the size of this vector is 0.
{

//cout<<"m_neighbour : "<<i<<" : "<<m_neighbours[i].name<<endl ;
//cout<<"n.name "<<n.name <<endl;

 if ( neighbours[i].name == n.name )   // vector of neighbours_info
 {
     break;  // avoiding duplicate entries.
 }

 }

 if ( i == neighbours.size() )
 {
      neighbours.push_back(n) ;
 }

 }

 else if ( name == my_idName )  // name is the source. my_idName is passed as command line argument.
 {

startTimer(n) ; // store the current time for this neighbour

neighbours.push_back(n) ;     // store neighbour in this Vector

Table[dest] = entry ;      // this will work when the initial router is not 'H'  , other entries have default values.

 }

 portnos[n.name] = n.portno ;  // stored in a map and this field is for every router .  //

 }   // end of while loop.

 // special port number for sending data packet

 portnos['H'] = 11111; // special port number for sending data.

 memcpy ( (void*)backupTable , (void*) Table , sizeof( Table) ) ;

 if ( nameOf( myIndex ) != 'H' )
 {
     print ( Table , nameOf(myIndex) , "Initial routing table", true ) ; // print the initial routing table ( dv_info ) of this Router
 }

}


void Router ::reset ( char dead ) // reset this Router if no other Router muticasts to this router after 10 seconds.
{

 for (int i = 0; i < neighbours.size(); i++)
 {

 if ( neighbours[i].name == dead )
 {

 if ( backupTable[indexOf(dead)].Cost() != -1 )
 {
     backupTable[indexOf(dead)].setDead();
 }

 }

 }

 memcpy( (void*)Table, (void*)backupTable , sizeof(Table) ) ;  // copy contents of backup routing table into original one.

 print( Table, nameOf(myIndex), "Reset routing table", true ) ;
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

 if ( dest == myIndex )
 {
     continue;
 }

 bool changedEntry = false;  // m_Details[] has the entry for every other router.
/// will be true if the cost is changed from source to dest.


// m_Details[i] is the (i+1)th entry of this router's  Routing Table.

 char a =  Table[dest].nextRouterName() ;  // a = 'B' ; when turn of B comes. // router E sends its routing table.

  Table[dest].setCost ( min  ( Table[dest].Cost(),  Table[intermediate].Cost(), advertisement[dest].Cost(), changedEntry ) ) ;

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
  Table[dest].setNextRouterPort( portNo(source) ) ; // If cost from A to B changes through E
  Table[dest].setNextRouterName( source ) ;           // then A's next router is E and port_dest will be port numer of router E.

 }

 }



  Table[intermediate].setCost ( advertisement[myIndex].Cost() ) ;

 if (changedDV)
 {
 print ( originalTable, nameOf(myIndex), "Change detected!\nRouting table before change", true ) ;
 print ( advertisement, source, "DV that caused the change", false ) ;
 print (  Table, nameOf(myIndex), "Routing table after change", false ) ;
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

int Router :: portNo ( char router )
{
 return portnos[router];
}

void Router ::initMyaddr( int portno )
{
 memset((char *)&myAddress, 0, sizeof(myAddress));
 myAddress.sin_family = AF_INET;
 myAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
 myAddress.sin_port = htons(portno);
}

void Router:: startTimer ( neighbour_info &n )
{
 clock_gettime(CLOCK_MONOTONIC, &n.startTime);
}

bool Router:: timeExpired ( neighbour_info &n ) const
{

 timespec tend= { 0 , 0 } ;   // broken into seconds and nanoseconds.  // this stores the current time and subtracts it from initial time.

 clock_gettime(CLOCK_MONOTONIC, &tend);

 if  ( (  ( double) tend.tv_sec + 1.0e-9*tend.tv_nsec ) - ( (double)n.startTime.tv_sec + 1.0e-9*n.startTime.tv_nsec ) > 10 )  // 5 second limit.
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

void Router::print ( RoutingEntry  myrouter[], char name, string msg, bool curr_time )   // dv : myrouter  ; removed const because of portNo() function
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
        if ( myrouter[dest].Cost() != inf ) // Router directly connected to A will have finite cost , so they get printed initially.
        {

			cout << " |     " << nameOf(dest) << "     | ";

			cout <<" "<< myrouter[dest].Cost() <<"    |";

            cout <<"      "<< portNo(name) << " "<< name <<"   |";  // see portNoOf

			cout <<"      "<< portNo(nameOf(dest));

            //cout<< " -> "<<nameOf(dest);

			cout<<"      |\n";

        }

	}
	cout << endl;

}
