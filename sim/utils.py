import sst
import os
try:
    import ConfigParser
except ImportError:
    import configparser as ConfigParser



def connect(name, c0, port0, c1, port1, latency):
    link = sst.Link(name)
    link.connect( (c0, port0, latency), (c1, port1, latency) )
    return link



class Config:
    def __init__(self, cfgFile, **kwargs):
        cp = ConfigParser.ConfigParser()
        if not cp.read(cfgFile):
            raise Exception('Unable to read file "%s"'%cfgFile)

        self.verbose = "verbose" in kwargs and kwargs["verbose"]

        self.clock = cp.get('CPU', 'clock')
        self.total_cores = cp.getint('CPU', 'num_cores')
        self.max_reqs_cycle = cp.get('CPU', 'max_reqs_cycle')

        self.memory_clock = cp.get('Memory', 'clock')
        self.memory_network_bandwidth = cp.get('Memory', 'network_bw')
        self.memory_capacity = cp.get('Memory', 'capacity')
        self.coherence_protocol = "MESI"

        self.num_ring_stops = self.total_cores + 1

        self.ring_latency = cp.get('Network', 'latency')
        self.ring_bandwidth = cp.get('Network', 'bandwidth')
        self.ring_flit_size = cp.get('Network', 'flit_size')

        self.app = cp.get('CPU', 'application')
        self.coreConfigAppParams = dict(cp.items(self.app))
       
        if 'SPYLAXGenerator' in self.app: 
            self.coreConfig = self._mirandaCoreConfig
            self.coreGenConfig = self._spylaxGenConfig
         
        else:
            raise Exception("Unknown application '%s'"%app)

    def getCoreConfig(self, core_id):
        params = dict({
                'clock': self.clock,
                'verbose': int(self.verbose)
                })
        params.update(self.coreConfig(core_id))
        #params.update ({"generator" :"miranda.SPYLAXGenerator"}) #comment this out
        return params
    

    def _mirandaCoreConfig(self, core_id):
        params = dict()
        params['max_reqs_cycle'] =  self.max_reqs_cycle
        return params
    
    def getCoreGenConfig(self, core_id):
        return self.coreGenConfig(core_id)
    def _spylaxGenConfig(self, core_id) : 
        params = dict({"size" :     16000 ,  
                "unit_size" :  4 , 
                "issue_count" : 1 , 
                "cores" :   self.total_cores  , 
                "core_id" : core_id} ) 
 
        params.update(self.coreConfigAppParams) 
        if "size" in self.coreConfigAppParams: 
            params["size"] = int(params["size"])  


        return params 
    def _streamGenConfig(self, core_id):
        streamN = int(self.coreConfigAppParams['total_streamn'])
        params = dict({
            "size"             : streamN // self.total_cores,
            "start_a"       : ( (streamN * 32) // self.total_cores ) * core_id,
            "start_b"       : ( (streamN * 32) // self.total_cores ) * core_id + (streamN * 32),
            "start_c"       : ( (streamN * 32) // self.total_cores ) * core_id + (2 * streamN * 32),
            "operandwidth"  : 32,
            "verbose"       : int(self.verbose)
            })
        return params
    def getL1Params(self):
        return dict({
            "prefetcher": "cassini.StridePrefetcher",
            "prefetcher.reach": 4,
            "prefetcher.detect_range" : 1,
            "cache_frequency": self.clock,
            "cache_size": "32KB",
            "associativity": 8,
            "access_latency_cycles": 4,
            "L1": 1,
            # Default params
            # "cache_line_size": 64,
            # "coherence_protocol": self.coherence_protocol,
            # "replacement_policy": "lru",
            # Not neccessary for simple cases:
            #"maxRequestDelay" : "1000000",
            })

    def getL2Params(self):
        return dict({
            "prefetcher": "cassini.StridePrefetcher",
            "prefetcher.reach": 16,
            "prefetcher.detect_range" : 1,
            "cache_frequency": self.clock,
            "cache_size": "256KB",
            "associativity": 8,
            "access_latency_cycles": 6,
            "mshr_num_entries" : 16,
            "memNIC.network_bw": self.ring_bandwidth, # This parameter belongs to the MemNIC SubComponent
            # Default params
            #"cache_line_size": 64,
            #"coherence_protocol": self.coherence_protocol,
            #"replacement_policy": "lru",
            })

    def getMemCtrlParams(self):
        return dict({
            "clock" : self.memory_clock,
            "memNIC.network_bw": self.ring_bandwidth, # This parameter belongs to the MemNIC SubComponent
            "backing" : "none",
            })

    def getMemBackendType(self):
        return "memHierarchy.simpleMem"

    def getMemBackendParams(self):
        return dict({
            "access_time" : "30ns",
            "mem_size" : self.memory_capacity
            })

    def getDCParams(self, dc_id):
        return dict({
            "entry_cache_size": 256*1024*1024, #Entry cache size of mem/blocksize
            "clock": self.memory_clock,
            "memNIC.network_bw": self.ring_bandwidth, # This parameter belongs to the MemNIC SubComponent
            "addr_range_start" : 0,
            "addr_range_end" : (int(''.join(filter(str.isdigit, self.memory_capacity))) * 1024 * 1024),
            # Default params
            # "coherence_protocol": coherence_protocol,
            })

    def getRouterParams(self):
        return dict({
            "output_latency" : "25ps",
            "xbar_bw" : self.ring_bandwidth,
            "input_buf_size" : "2KB",
            "input_latency" : "25ps",
            "num_ports" : self.total_cores + 1,
            "flit_size" : self.ring_flit_size,
            "output_buf_size" : "2KB",
            "link_bw" : self.ring_bandwidth,
        })

