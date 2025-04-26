#include "NGPRC.h"
#include "grant.cpp"

// Template parameters: CHANNELS, WIDTH, WEIGHTLIMIT(removed)
template<int CHANNELS, int WIDTH, int DEC_SEL_WIDTH>
SC_MODULE(RRBTOP) {
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;

    sc_in<sc_uint<CHANNELS>> request; // one-hot request lines
    sc_in<sc_uint<CHANNELS * WIDTH>> weight; // packed weights bus

    sc_out<sc_uint<CHANNELS>> grant; // final grant signal (one-hot)

    // Internal signals
    sc_signal<sc_uint<CHANNELS>> s_selOneHot;
    sc_signal<sc_uint<WIDTH>> s_weight;
    sc_signal<sc_uint<CHANNELS>> s_nextGrant;

    // Submodules
    Mux<WIDTH, CHANNELS, DEC_SEL_WIDTH>* mux_inst;
    NGPRC* ngprc_inst;
    GrantModule<CHANNELS, WIDTH, DEC_SEL_WIDTH>* grant_inst;

SC_CTOR(RRBTOP) {
    // NGPRC instantiation
    ngprc_inst = new NGPRC("NGPRC");
    ngprc_inst->clk(clk);
    ngprc_inst->reset(reset);
    ngprc_inst->request(request);
    ngprc_inst->grant(s_selOneHot);
    ngprc_inst->nextGrant(s_nextGrant);
    
    // GRANT instantiation with internal MUX
    grant_inst = new GrantModule<CHANNELS, WIDTH, DEC_SEL_WIDTH>("GrantModule");
    grant_inst->clk(clk);
    grant_inst->reset(reset);
    grant_inst->request(request);
    grant_inst->nextGrant(s_nextGrant);
    grant_inst->weight(weight);  // Direct connection to packed weight bus
    grant_inst->grant(s_selOneHot);
    
    // Output assignment
    SC_METHOD(assign_grant);
    sensitive << s_selOneHot;
}

    void assign_grant() {
        grant.write(s_selOneHot.read());
    }
};
