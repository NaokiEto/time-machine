#ifndef CONNECTION_HH
#define CONNECTION_HH

#include <string>

#include "types.h"
#include "packet_headers.h"

#include "Hash.h"
//#include "IPAddr.h"
#include "re2/re2.h"

class QueryResult;
class Fifo;

typedef in_addr in4_addr;

class ConnectionID {
public:
	virtual ~ConnectionID() { }
	//  virtual hash_t hash() const = 0;
	virtual bool operator==(const ConnectionID& other) const = 0;
	//  virtual void* getVPtr() = 0;
	//  virtual const void* getConstVPtr() const = 0;
	virtual void getStr(char* s, int maxsize) const = 0;
	virtual std::string getStr() const = 0;

    //virtual HashKey* hash() const = 0;

 /**
  * Returns a hash key for a given ConnID. Passes ownership to caller.
  */
    //virtual HashKey* BuildConnHashKey(const ConnectionID& curr) const = 0;
};

class ConnectionID4: public ConnectionID {
public:
	ConnectionID4(proto_t proto,
				  uint32_t s_ip, uint32_t d_ip,
				  uint16_t s_port, uint16_t d_port) {
		init(proto, s_ip, d_ip, s_port, d_port);
	}
	ConnectionID4(proto_t proto,
                  unsigned char s_ip[], unsigned char d_ip[],
                  uint16_t s_port, uint16_t d_port) {
        init6(proto, s_ip, d_ip, s_port, d_port);
    }
	ConnectionID4(ConnectionID4 *c_id) {
        memcpy(v6.ip1, c_id->v6.ip1, 16);
        memcpy(v6.ip2, c_id->v6.ip2, 16);
		v6.port1 = c_id->v6.port1;
		v6.port2 = c_id->v6.port2;
		v6.proto = c_id->v6.proto;
        v6.version = c_id->v6.version;
	}
	ConnectionID4(const u_char* packet);
	ConnectionID4() {};

	/**
	 * Constructs an address instance from an IPv4 address.
	 *
	 * @param in6 The IPv6 address.
	 */
	explicit ConnectionID4(const in4_addr& in4_ip1, const in4_addr& in4_ip2)
		{

    	static uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
		memcpy(v6.ip1, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip1[12], &in4_ip1.s_addr, sizeof(in4_ip1.s_addr));

		memcpy(v6.ip2, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip2[12], &in4_ip2.s_addr, sizeof(in4_ip2.s_addr));
		}

	virtual ~ConnectionID4() {};
    /*
	uint32_t hash() const { 
		//TODO: initval
		return hash3words(v.ip1, v.ip2^v.proto, v.port1 | ((v.port2)<<16), 0);
	}
    */

    /*
    HashKey* hash() const
    {
        return BuildConnHashKey(v6.ip1, v6.ip2, v6.port1, v6.port2);
    }
    */
	bool operator==(const ConnectionID& other) const { 
		return (v6.ip1 == ((ConnectionID4*)&other)->v6.ip1)
			   && (v6.ip2 == ((ConnectionID4*)&other)->v6.ip2)
			   && (v6.port1 == ((ConnectionID4*)&other)->v6.port1)
			   && (v6.port2 == ((ConnectionID4*)&other)->v6.port2)
			   && (v6.proto == ((ConnectionID4*)&other)->v6.proto);
	}

	static ConnectionID4 *parse(const char *str);
/*
	proto_t get_proto() const {
		return v.proto;
	}
	uint32_t get_ip1() const {
		return v.ip1;
	}
	uint32_t get_ip2() const {
		return v.ip2;
	}
	uint16_t get_port1() const {
		return v.port1;
	}
	uint16_t get_port2() const {
		return v.port2;
	}
*/
	proto_t get_proto() const {
		return v6.proto;
	}
	const unsigned char* get_ip1() const {
		return v6.ip1;
	}
	const unsigned char* get_ip2() const {
		return v6.ip2;
	}
	uint16_t get_port1() const {
		return v6.port1;
	}
	uint16_t get_port2() const {
		return v6.port2;
	}

	//  bool get_is_canonified() const { return v.is_canonified; }
	/*
	uint32_t get_s_ip() const {
	  return v.is_canonified ? v.ip2 : v.ip1 ; }
	uint32_t get_d_ip() const {
	  return v.is_canonified ? v.ip1 : v.ip2 ; }
	uint16_t get_s_port() const {
	  return v.is_canonified ? v.port2 : v.port1 ; }
	uint16_t get_d_port() const {
	  return v.is_canonified ? v.port1 : v.port2 ; }
	
	typedef struct {
		//  time locality
		//    uint32_t ts;
		uint32_t ip1;
		uint32_t ip2;
		uint16_t port1;
		uint16_t port2;
		proto_t proto;
        // made up my own parameter to distinguish between IPv4 and IPv6
        int version;
		//    bool is_canonified;
	}
    // have the structure fields align on one-byte boundaries
	__attribute__((packed)) v_t;
    */

	typedef struct {
		//  time locality
		//    uint32_t ts;
		unsigned char ip1[16];
		unsigned char ip2[16];
		uint16_t port1;
		uint16_t port2;
		proto_t proto;
        int version;
		//    bool is_canonified;
	}
    // have the structure fields align on one-byte boundaries
	__attribute__((packed)) v6_t;
    /*
	v_t* getV() {
		return &v;
	}
	const v_t* getConstV() const {
		return &v;
	}
*/
	v6_t* getV() {
		return &v6;
	}
	const v6_t* getConstV() const {
		return &v6;
	}

	void getStr(char* s, int maxsize) const;
	std::string getStr() const;

    hash_t hash() const;

protected:
	void init(proto_t proto, uint32_t s_ip, uint32_t d_ip,
			  uint16_t s_port, uint16_t d_port);
    void init6(proto_t proto, unsigned char s_ip[], unsigned char d_ip[], 
               uint16_t s_port, uint16_t d_port);
	//v_t v;
    v6_t v6;
    hash_t hash_key;

private:
	static std::string pattern_connection4;
    static std::string pattern6_connection4;
	static RE2 re;
    static RE2 re6;

	//in6_addr in6; // IPv6 or v4-to-v6-mapped address
};

class ConnectionID3: public ConnectionID {
public:
	ConnectionID3(proto_t proto,
				  uint32_t ip1, uint32_t ip2,
				  uint16_t port2) {
		init(proto, ip1, ip2, port2);
	}

    ConnectionID3(proto_t proto,
                  unsigned char ip1[], unsigned char ip2[], 
                  uint16_t port2) {
            init6(proto, ip1, ip2, port2);
    }

	ConnectionID3(const u_char* packet, int wildcard_port);
	ConnectionID3() {};

	/**
	 * Constructs an address instance from an IPv4 address.
	 *
	 * @param in6 The IPv6 address.
	 */
	explicit ConnectionID3(const in4_addr& in4_ip1, const in4_addr& in4_ip2)
		{
    	static uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
		memcpy(v6.ip1, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip1[12], &in4_ip1.s_addr, sizeof(in4_ip1.s_addr));

		memcpy(v6.ip2, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip2[12], &in4_ip2.s_addr, sizeof(in4_ip2.s_addr));
		}

	virtual ~ConnectionID3() {
    };
    /*
	uint32_t hash() const {
		//TODO: initval
		return hash3words(v.ip1, v.ip2, v.port2 | ((v.proto)<<16), 0);
	}
    

    HashKey* hash() const
    {
        return BuildConnHashKey(v6.ip1, v6.ip2, v6.port2, 0);
    }
    
	bool operator==(const ConnectionID& other) const;
	proto_t get_proto() const {
		return v.proto;
	}
	uint32_t get_ip1() const {
		return v.ip1;
	}
	uint32_t get_ip2() const {
		return v.ip2;
	}
	uint16_t get_port() const {
		return v.port2;
	}
*/
	bool operator==(const ConnectionID& other) const;
	proto_t get_proto() const {
		return v6.proto;
	}
	const unsigned char* get_ip1() const {
		return v6.ip1;
	}
	const unsigned char* get_ip2() const {
		return v6.ip2;
	}
	uint16_t get_port() const {
		return v6.port2;
	}
	/*
	bool get_is_canonified() const { return v.is_canonified; }
	uint32_t get_s_ip() const {
	  return v.is_canonified ? v.ip2 : v.ip1 ; }
	uint32_t get_d_ip() const {
	  return v.is_canonified ? v.ip1 : v.ip2 ; }
	
	typedef struct {
		//  time locality
		//    uint32_t ts;
		uint32_t ip1;
		uint32_t ip2;
		uint16_t port2;
		proto_t proto;
        int version;
		//    bool is_canonified;
	}
	__attribute__((packed)) v_t;
    */

	typedef struct {
		//  time locality
		//    uint32_t ts;
		unsigned char ip1[16];
		unsigned char ip2[16];
		uint16_t port2;
		proto_t proto;
        int version;
		//    bool is_canonified;
	}
	__attribute__((packed)) v6_t;

/*
	v_t* getV() {
		return &v;
	}

	const v_t* getConstV() const {
		return &v;
	}
*/

	v6_t* getV() {
		return &v6;
	}

	const v6_t* getConstV() const {
		return &v6;
	}


	void getStr(char* s, int maxsize) const;
	std::string getStr() const;

/*
    HashKey* BuildConnHashKey(unsigned char s_ip[], unsigned char d_ip[],
                  uint16_t s_port, uint16_t d_port) const;
*/

    hash_t hash() const;

    hash_t hash_key;  
protected:
	void init(proto_t proto, uint32_t s_ip, uint32_t d_ip, uint16_t port);
    void init6(proto_t proto, unsigned char s_ip[], unsigned char d_ip[], 
               uint16_t port);
	//v_t v;
    v6_t v6;


//private:
	//in6_addr in6; // IPv6 or v4-to-v6-mapped address

	//static const uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
};


class ConnectionID2: public ConnectionID {
public:
	ConnectionID2( uint32_t s_ip, uint32_t d_ip) {
		init(s_ip, d_ip);
	}

    ConnectionID2(unsigned char s_ip[], unsigned char d_ip[]) {
        init6(s_ip, d_ip);
    }

	ConnectionID2(const u_char* packet);
	ConnectionID2() {};

	/**
	 * Constructs an address instance from an IPv4 address.
	 *
	 * @param in6 The IPv6 address.
	 */
	explicit ConnectionID2(const in4_addr& in4_ip1, const in4_addr& in4_ip2)
		{
    	static uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
		memcpy(v6.ip1, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip1[12], &in4_ip1.s_addr, sizeof(in4_ip1.s_addr));

		memcpy(v6.ip2, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&v6.ip2[12], &in4_ip2.s_addr, sizeof(in4_ip2.s_addr));
		}

	virtual ~ConnectionID2() {};
    /*
	uint32_t hash() const {
		//TODO: initval
		return hash2words(v.ip1, v.ip2, 0);
	}
    */

/*
    HashKey* hash() const
    {
        return BuildConnHashKey(v6.ip1, v6.ip2, 0, 0);
    }
*/
	bool operator==(const ConnectionID& other) const;
/*
	uint32_t get_ip1() const {
		return v.ip1;
	}
	uint32_t get_ip2() const {
		return v.ip2;
	}
*/

	const unsigned char* get_ip1() const {
		return v6.ip1;
	}
	const unsigned char* get_ip2() const {
		return v6.ip2;
	}

	/*
	bool get_is_canonified() const { return v.is_canonified; }
	uint32_t get_s_ip() const {
	  return v.is_canonified ? v.ip2 : v.ip1 ; }
	uint32_t get_d_ip() const {
	  return v.is_canonified ? v.ip1 : v.ip2 ; }
	
	typedef struct {
		//  time locality
		//    uint32_t ts;
		uint32_t ip1;
		uint32_t ip2;
        int version;
		//    bool is_canonified;
	}
	__attribute__((packed)) v_t;
    */

	typedef struct {
		//  time locality
		//    uint32_t ts;
		unsigned char ip1[16];
		unsigned char ip2[16];
        int version;
		//    bool is_canonified;
	}
	__attribute__((packed)) v6_t;
/*
	v_t* getV() {
		return &v;
	}
	const v_t* getConstV() const {
		return &v;
	}
*/
	v6_t* getV() {
		return &v6;
	}
	const v6_t* getConstV() const {
		return &v6;
	}

	void getStr(char* s, int maxsize) const;
	std::string getStr() const;

/*
    HashKey* BuildConnHashKey(unsigned char s_ip[], unsigned char d_ip[],
                  uint16_t s_port, uint16_t d_port) const;
*/

    hash_t hash() const;

    hash_t hash_key;

protected:
	void init(uint32_t s_ip, uint32_t d_ip);
    void init6( unsigned char s_ip[], unsigned char d_ip[]);
	//v_t v;
    v6_t v6;

//private:
	//in6_addr in6; // IPv6 or v4-to-v6-mapped address

	//static const uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
};


class Connection {
public:
	/*
	Connection(proto_t proto,
	    uint32_t s_ip, uint32_t d_ip,
	    uint16_t s_port, uint16_t d_port);
	Connection(ConnectionID&);
	*/
	/* id  will be owned by Connection class and freed by it */
	Connection(ConnectionID4 *id) {
		init(id);
	}
	Connection(Connection *c);
	virtual ~Connection() {
    // we need delete [] instead of delete because we allocated an array for the ipv6 addresses?
		delete c_id;
	}
	void addPkt(const struct pcap_pkthdr* header, const u_char* packet);
	tm_time_t getLastTs() {
		return last_ts;
	}
	uint64_t getTotPktbytes() {
		return tot_pktbytes;
	}
	//  ConnectionID* getKey() { return key; }
	Fifo* getFifo() {
		return fifo;
	}
	void setFifo(Fifo *f) {
		fifo=f;
	}
	void setSuspendCutoff(bool b) {
		suspend_cutoff=b;
	}
	bool getSuspendCutoff() {
		return suspend_cutoff;
	}
	void setSuspendTimeout(bool b) {
		suspend_timeout=b;
	}
	bool getSuspendTimeout() {
		return suspend_timeout;
	}
	std::string getStr() const;
	void setSubscription(QueryResult *q) {
		subscription=q;
	}
	QueryResult* getSubscription() {
		return subscription;
	}
	int deleteSubscription();

	friend class Connections;
protected:
	ConnectionID4 *c_id;
	//  ConnectionID* key;
	//  struct ConnectionID c_id;
	tm_time_t last_ts;

	/* cache to which class this connection belongs */
	Fifo* fifo;
	/* is there a subscription for this connection? */
	QueryResult* subscription;
	/* true if cutoff should not be done for this connection */
	bool suspend_cutoff;
	/* true if inactivity timeout should not be done for this connection */
	bool suspend_timeout;

	//	bool tcp_syn;

	uint64_t tot_pkts;
	uint64_t tot_pktbytes;

	//  hash_t hash() const;
	//  bool operator==(const Connection& other) const { return c_id==other.c_id; }
	void init(ConnectionID4 *);

	/* hash collision queue */
	Connection *col_next;
	Connection *col_prev;

	/* timeout queue */
	Connection *q_newer;
	Connection *q_older;

	
};

#endif
