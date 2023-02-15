#pragma once 
#include "../miranda/mirandaGenerator.h"
/**
 * @brief Traffic generator for SPYLAX operation. Ex: Res[i] = C*A[i] + B[i], where Res, A and B are arrays with same number of
elements and C is a constant 
 *
 * 
 */
namespace SST {
namespace Miranda{ 
class SPYLAXGenerator : public RequestGenerator {  
    
    private: 
        uint64_t m_addr_a ;
        uint64_t m_addr_b ;
        uint64_t m_addr_res ;
        uint32_t m_unit_size;
        uint32_t m_size; 
        uint32_t m_index; 
        uint32_t m_issue_count ; 
        Output*  m_out;
    public:     
        SPYLAXGenerator(ComponentId_t id , Params& params) ; 
        ~SPYLAXGenerator(); 
	    void generate(MirandaRequestQueue<GeneratorRequest*>* q) override ;  
        bool isFinished() override; 
        void completed() override { }; 
        SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(SPYLAXGenerator,
         "spylaxgen", "SPYLAXGenerator" ,
         SST_ELI_ELEMENT_VERSION(1, 0, 0),  
         "Generates traffic for a spylax algorithm",
          SST::Miranda::RequestGenerator ) 
        SST_ELI_DOCUMENT_PARAMS( 
            {"size", "Total array size" , "10000"}, // total array size 
            {"unit_size"  , "Defines the byte size of an individual element", "4" }, // 4 bytes for number  
            {"issue_count" , "How many requests can the CPU issue per cycle", "1"}, 
            {"cores","How many cores will be used" , "1" } , 
            {"core_id" , "currentid", "0"}
        )  
} ; 
    }
}