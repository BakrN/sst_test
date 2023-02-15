#include "spylaxgen.h" 
#include <sst/core/output.h> 
#include <sst/core/params.h> 
#include <cmath> 
using namespace SST::Miranda; 

SPYLAXGenerator::SPYLAXGenerator(ComponentId_t id , Params& params) :  RequestGenerator(id, params) { 

    uint32_t core_count = params.find<uint32_t>("cores", 1); 
    uint32_t core_id =params.find<uint32_t>("core_id", 0); 
    assert (core_id <= core_count-1); 
    uint32_t total_size = params.find<uint32_t>("size", 10000); 
    m_unit_size = params.find<uint32_t>("unit_size", 4); 
    m_issue_count = params.find<uint32_t>("issue_count", 1); 

    uint64_t addr_a_low = 0 ; 
    uint64_t addr_b_low   = std::exp2(std::ceil(std::log2(total_size * m_unit_size  ))) ; 
    uint64_t addr_res_low = std::exp2(std::ceil(std::log2(addr_b_low + total_size* m_unit_size))) ; 
    // divide array
    uint64_t chunk_size = total_size / core_count; // Size of each partition 
    uint64_t remaining = total_size % core_count;
    uint64_t start = 0 , end = 0 ;
    
    for (int i = 0 ; i < core_count; i++){
        end += chunk_size;
            if (remaining > 0) {
                end++;
                remaining--;
            } 
            if ( i == core_id){ 
                break ; 
            }
            start = end ;  
    } 
    m_size = end - start; 
    m_addr_a   = start  * m_unit_size ;   
    m_addr_b   = addr_b_low + start * m_unit_size;  
    m_addr_res = addr_res_low + start * m_unit_size;   
    m_out = new Output("SpylaxGen[@p:@l]: ", 1, 0, Output::STDOUT);
    m_index = 0 ; 
    // PRINT INFO ABOUT MODULE 
    m_out->verbose(CALL_INFO, 1, 0, "Array size: %" PRIu32 "\n", total_size);
    m_out->verbose(CALL_INFO, 1, 0, "Partitioned array size: %" PRIu32 "\n", m_size);
    m_out->verbose(CALL_INFO, 1, 0, "Initialized generator for core with id: %" PRIu32 "\n", core_id);
    m_out->verbose(CALL_INFO, 1, 0, "A start address: %" PRIu64 "\n", m_addr_a); 
    m_out->verbose(CALL_INFO, 1, 0, "B start address: %" PRIu64 "\n", m_addr_b); 
    m_out->verbose(CALL_INFO, 1, 0, "Res start address: %" PRIu64 "\n", m_addr_res); 
	
}
void SPYLAXGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) { 
    for (int i = 0 ; i < std::max((uint32_t)1, m_issue_count-1); i++) {  
        if (m_index >= m_size){
            break ;
        }
        // created read_a request 
        MemoryOpRequest* read_a = new MemoryOpRequest(m_addr_a+m_index*m_unit_size,m_unit_size,  READ )  ; 
        // created read_b request 
        MemoryOpRequest* read_b = new MemoryOpRequest(m_addr_b+m_index*m_unit_size,m_unit_size,   READ )  ; 
        // created write_res request 
        MemoryOpRequest* write_res= new MemoryOpRequest(m_addr_res+m_index*m_unit_size,m_unit_size, WRITE) ; // cleanup handled by mirandacpu 
        // create dependencies 
        write_res->addDependency(read_a->getRequestID()); 
        write_res->addDependency(read_b->getRequestID()); 
        // adding requests to queue
        q->push_back(read_a) ;
        m_out->verbose(__LINE__, __FILE__, __FUNCTION__, 4, 0 , "Queued READ request for array A at address: %" PRIu64 "\n",m_addr_a+m_index*m_unit_size );
        q->push_back(read_b) ;
        m_out->verbose(__LINE__, __FILE__, __FUNCTION__, 4, 0 , "Queued READ request for array B at address: %" PRIu64 "\n",m_addr_b+m_index*m_unit_size );
        q->push_back(write_res);  
        m_out->verbose(__LINE__, __FILE__, __FUNCTION__, 4, 0 , "Queued WRITE request for array Res at address: %" PRIu64 "\n",m_addr_res+m_index*m_unit_size );
        
        m_index ++ ; 
    }
}
SPYLAXGenerator::~SPYLAXGenerator() { 
    delete m_out; 
}
bool SPYLAXGenerator::isFinished() { 
    return (m_index >= m_size); 
}